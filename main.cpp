#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "huerunner.h"

static QObject *huerunner_singleton_provider(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)

    HueRunner *runner = new HueRunner();
    return runner;
}

int main(int argc, char *argv[])
{
#if defined(Q_OS_WIN)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QCoreApplication::setOrganizationName("BradyBrenot");
    QCoreApplication::setOrganizationDomain("bradybrenot.com");
    QCoreApplication::setApplicationName("Huestacean");

    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    qmlRegisterSingletonType<HueRunner>("Huestacean.HueRunner", 1, 0, "HueRunner", huerunner_singleton_provider);

    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
