#include "RenderWidget.h"
#include "Constants.h"

#include <stdexcept>

#include "shaders/quad.glsl.hpp"
#include "shaders/raytracer.glsl.hpp"

namespace 
{
    void CompileShaders(Program& quadProgram, Program& computeProgram)
    {
        quadProgram.link(
            VertexShader(qtwin::GLVersion, {shaders::quad}),
            FragmentShader(qtwin::GLVersion, {shaders::quad})
        );
        computeProgram.link(
            ComputeShader(
                qtwin::GLVersion, {shaders::raytracer})
        );
    }

    void CreateTexture(QSize size, GLuint& tex)
    {
        if (tex) { glDeleteTextures(1, &tex); };

        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RGBA32F,
            size.width(), size.height(),
            0,
            GL_RGBA,
            GL_FLOAT,
            nullptr
        );
    }
}

namespace qtwin
{
    RenderWidget::RenderWidget(
        const std::string& fragmentShaderPath,
        const QGLFormat& format
        )
    : QGLWidget(format, static_cast<QWidget*>(nullptr)),
      m_fragmentShaderPath(fragmentShaderPath)
    {
        grabKeyboard();

        using namespace fsutil;
        m_spFileWatcher.reset(
            new FileWatcher(
                m_fragmentShaderPath,
                [](const std::string& path, FileWatcher::FileWatcherEvent event)
                {
                    const std::string EventString = 
                        (event == FileWatcher::FileWatcherModified ?
                             "modified" : "deleted");
                    std::cout << path << " " << EventString << std::endl;
                })
        );
    }

    RenderWidget::~RenderWidget() 
    {
        releaseKeyboard();
    }

    void RenderWidget::initializeGL()
    {
        GLint major;
        GLint minor;
        glGetIntegerv(GL_MAJOR_VERSION, &major);
        glGetIntegerv(GL_MINOR_VERSION, &minor);

        if (major * 100 + minor * 10 < GLVersion)
        {
            throw std::runtime_error(
                "OpenGL version " + std::to_string(GLVersion) +
                " is required.");
        }

        CompileShaders(m_progQuad, m_progCompute);

        glGenVertexArrays(1, &m_vao); 
    }

    void RenderWidget::resizeGL(int width, int height)
    {
        glViewport(0, 0, width, height);
        m_curSize = QSize(width, height);
    }

    void RenderWidget::paintGL()
    {
        if (m_curSize != m_texSize)
        {
            CreateTexture(m_curSize, m_tex);
            m_texSize = m_curSize;
        }

        m_progCompute.use();

        glBindImageTexture(0, m_tex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

        int lsize[3];
        m_progCompute.localSize(lsize);

        int ngroups[3];
        ngroups[0] = (m_texSize.width() + lsize[0]-1) / lsize[0];
        ngroups[1] = (m_texSize.height() + lsize[1]-1) / lsize[1];
        ngroups[2] = 1;

        glDispatchCompute(ngroups[0], ngroups[1], ngroups[2]);

        // Prevent sampling before all writes to texture are done
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        m_progQuad.use();

        glUniform1i(m_progQuad.uniform("sampler"), 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_tex);

        glBindVertexArray(m_vao);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }

    void RenderWidget::keyPressEvent(QKeyEvent* pEvent)
    {
        switch(pEvent->key())
        {
            case Qt::Key_Q:
                parentWidget()->close();
            default: break;
        }

        updateGL();
    }
}

