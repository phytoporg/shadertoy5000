#include "MainWindow.h"

//
// Heavily borrowed from jakubcerveny/gl-compute
//
namespace qtwin
{
    MainWindow::MainWindow(QWidget* pGL)
    {
        setCentralWidget(pGL);
    }
}

