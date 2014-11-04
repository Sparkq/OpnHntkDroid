#include <QGuiApplication>
#include <QApplication>
#include <QQmlApplicationEngine>

#include <QtQuick/QQuickView>

#include "src/viewer.h"
#include <QLibraryInfo>
#include <QLocale>
#include <QTranslator>



//! [1]
int main(int argc, char **argv)
{






        QApplication app(argc, argv);
        qmlRegisterType<Viewer>("OpenGLUnderQML", 1, 0, "Viewer");

        QQmlApplicationEngine engine;
        engine.load(QUrl(QStringLiteral("qrc:/main.qml")));






        return app.exec();
}


