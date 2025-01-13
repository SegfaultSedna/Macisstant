#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickView>
#include <QVariant>
#include <QDebug>
#include <QAbstractListModel>
#include "AppController.h"
#include "FileOperator.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;
    AppController appController(&engine);
    FileOperator fileOperator;
    engine.rootContext()->setContextProperty("fileOperator", &fileOperator);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("MacisstantQML_cmake", "Main");

    QObject::connect(engine.rootObjects().first(), SIGNAL(kbMacrosWindowLoaded()), &appController, SLOT(onKbMacrosWindowLoaded()));

    return app.exec();
}
