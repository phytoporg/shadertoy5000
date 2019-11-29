//
// QT window wrapper class.
//

#pragma once

#include <GL/gl.h>
#include <GL/glu.h>

#include <QtGui>
#include <QGLWidget>
#include <QSize>

//
// Heavily borrowed from jakubcerveny/gl-compute
//
namespace qtwin
{
    class MainWindow : public QMainWindow
    {
    Q_OBJECT

    public:
        MainWindow(QWidget* pGl);
    };
}

