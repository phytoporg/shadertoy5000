#include "RenderWidget.h"
#include "Constants.h"

#include <fstream>
#include <stdexcept>

#include "shaders/vertex.glsl.hpp"

namespace 
{
    void CompileShaders(
        const std::string& fragmentPath,
        Program& quadProgram
        )
    {
        std::ifstream in(fragmentPath);
        in.seekg(0, in.end);
        const size_t FileSize = in.tellg();
        in.seekg(0, in.beg);

        std::vector<char> source(FileSize + 1, 0);
        in.read(source.data(), FileSize);
        source.back() = '\0';

        ShaderSource fragmentSource(source.data());

        try
        {
            quadProgram.link(
                VertexShader(qtwin::GLVersion, {shaders::vertex}),
                FragmentShader(qtwin::GLVersion, {fragmentSource})
            );
        } catch (GLError& e)
        {
            std::cerr << e.what() << std::endl;
        }
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
                [this]
                (const std::string& path, FileWatcher::FileWatcherEvent event)
                {
                    {
                        std::lock_guard<std::mutex> lock(m_shaderMutex);
                        m_mustLoadShaders = true;
                    }

                    updateGL();
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

        CompileShaders(m_fragmentShaderPath, m_progQuad);

        glGenVertexArrays(1, &m_vao); 
    }

    void RenderWidget::resizeGL(int width, int height)
    {
        glViewport(0, 0, width, height);
        m_curSize = QSize(width, height);

        updateGL();
    }

    void RenderWidget::paintGL()
    {
        {
            std::lock_guard<std::mutex> lock(m_shaderMutex);
            if (m_mustLoadShaders)
            {
                CompileShaders(m_fragmentShaderPath, m_progQuad);
                m_mustLoadShaders = false;
            }
        }

        if (m_curSize != m_texSize)
        {
            CreateTexture(m_curSize, m_tex);
            m_texSize = m_curSize;
        }

        m_progQuad.use();
        glBindImageTexture(0, m_tex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

        glUniform2f(
            m_progQuad.uniform("iResolution"),
            static_cast<float>(m_curSize.width()),
            static_cast<float>(m_curSize.height()));

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

