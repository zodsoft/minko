
#include "QMinkoGLWidget.hpp"
#include "QMinkoEffectEditor.hpp"
//#include "QOpenGLWindow.hpp"

#include "openglwindow.h"

#include <QtGui/QGuiApplication>
#include <QtGui/QMatrix4x4>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QScreen>

#include <QtWidgets/QApplication>
#include <QtWidgets/QApplication>
#include <QtCore/qmath.h>

using namespace minko;

int 
main(int argc, char **argv)
{
	/*
	QApplication a(argc, argv);
	QMinkoEffectEditor w;
	w.show();
	
	return a.exec();
	*/
	
	
	QApplication app(argc, argv);
    app.setApplicationName("cube");
    app.setApplicationVersion("0.1");

#ifndef QT_NO_OPENGL
    QMinkoGLWidget widget;
    widget.show();
#else
	std::cerr << "OpenGL support required." << std::endl;
#endif // QT_NO_OPENGL
	return app.exec();
	
}