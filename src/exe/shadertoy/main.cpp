#include <qtwin/MainWindow.h>
#include <qtwin/RenderWidget.h>

#include <memory>

#include <iostream>

namespace
{
    void PrintUsage(const std::string& programName)
    {
        std::cerr << "Usage: " 
                  << programName 
                  << " <path_to_shader>"
                  << std::endl;
    }
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        PrintUsage(argv[0]);
        return -1;
    }

    QApplication app(argc, argv);

    using namespace qtwin;
    const std::string PathToShader(argv[1]);
    std::unique_ptr<RenderWidget> spGL(new RenderWidget(PathToShader));
    
    MainWindow window(spGL.get());
    spGL->setParent(&window);

    QSize size(1280, 720);
    window.resize(size);
    window.show();

    return app.exec();
}

