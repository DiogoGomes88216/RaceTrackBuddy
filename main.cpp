#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <stdlib.h>
#include "Backend/cmainprocess.h"
cMainProcess *myClass;

void tSignalHandler(int sig, siginfo_t * info, void * context)
{
    cout<<"SIGTERM RECEIVED"<<endl;
    if(sig == SIGTERM)
    {
        myClass->~cMainProcess();
        exit(0);
    }
}
int main(int argc, char *argv[])
{
    qputenv("QT_IM_MODULE", QByteArray("qtvirtualkeyboard"));

    #if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    #endif
    QGuiApplication app(argc, argv);

    myClass = new cMainProcess;
    QQmlApplicationEngine engine;
    QQmlContext *rootContext = engine.rootContext();

    rootContext->setContextProperty("myclass", myClass);

    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    rootContext->setContextProperty("rootObject", QVariant::fromValue(engine.rootObjects().first()));

    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags = (SA_SIGINFO | SA_RESTART);
    act.sa_sigaction = tSignalHandler;
    sigaction(SIGTERM, &act, NULL);

    return app.exec();
}
