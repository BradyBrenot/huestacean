#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QFontDatabase>
#include <QQmlContext>
#include "huestacean.h"

extern QQmlApplicationEngine* engine;
QQmlApplicationEngine* engine = nullptr;

static QObject *huestacean_singleton_provider(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine);
    Q_UNUSED(scriptEngine);

    Huestacean *huestacean = new Huestacean();
    return huestacean;
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

    qmlRegisterSingletonType<Huestacean>("Huestacean", 1, 0, "Huestacean", huestacean_singleton_provider);

    const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    QQmlApplicationEngine theEngine;
    ::engine = &theEngine;
    theEngine.rootContext()->setContextProperty("fixedFont", fixedFont);
    theEngine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
    if (theEngine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
