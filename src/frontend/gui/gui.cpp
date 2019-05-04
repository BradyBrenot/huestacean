#include "frontend/gui/gui.h"
#include "frontend/frontend.h"
#include "frontend/frontendtypes.h"

#include <QRemoteObjectHost>

#include <QCoreApplication>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QFontDatabase>
#include <QQmlContext>
#include <QKeyEvent>

#include <QDebug>
#include <QIcon>
#include <QPixmap>

#ifdef _WIN32
#include <Windows.h>
#endif


#include <QLoggingCategory>

using namespace Gui;

int Gui::Main(int argc, char* argv[])
{
	/////////////////////
	// Qt settings
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	/////////////////////

#ifdef _WIN32
	FILE* pout = nullptr;
	FILE* perr = nullptr;
	FILE* pin = nullptr;

	if (AttachConsole(ATTACH_PARENT_PROCESS)) {

		freopen_s(&pout, "CON", "w", stdout);
		freopen_s(&perr, "CON", "w", stderr);
		freopen_s(&pin, "CON", "r", stdin);
	}
#endif

	QGuiApplication app(argc, argv);
	app.setWindowIcon(QIcon(QPixmap(":/images/icon.png")));

	qmlRegisterSingletonType<GuiHelper>("Huestacean.GuiHelper", 1, 0, "GuiHelper", [](QQmlEngine * engine, QJSEngine * scriptEngine) -> QObject * {
		Q_UNUSED(engine)
		Q_UNUSED(scriptEngine)

		return new GuiHelper(engine, scriptEngine);
	});

	qmlRegisterSingletonType<GuiHelper>("Huestacean.Types", 1, 0, "TypeFactory", [](QQmlEngine * engine, QJSEngine * scriptEngine) -> QObject * {
		Q_UNUSED(engine)
		Q_UNUSED(scriptEngine)

		return new TypeFactory(engine);
	});

	
	QQmlApplicationEngine engine;

	const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
	engine.rootContext()->setContextProperty("fixedFont", fixedFont);

	//app.setFont(QFont(QFontDatabase::applicationFontFamilies(QFontDatabase::addApplicationFont(":/fonts/roboto/Roboto-Medium.ttf")).at(0)));

	QFontDatabase::addApplicationFont(":/fonts/roboto/Roboto-Black.ttf");
    QFontDatabase::addApplicationFont(":/fonts/roboto/Roboto-BlackItalic.ttf");
    QFontDatabase::addApplicationFont(":/fonts/roboto/Roboto-Bold.ttf");
    QFontDatabase::addApplicationFont(":/fonts/roboto/Roboto-BoldItalic.ttf");
    QFontDatabase::addApplicationFont(":/fonts/roboto/Roboto-Italic.ttf");
    QFontDatabase::addApplicationFont(":/fonts/roboto/Roboto-Light.ttf");
    QFontDatabase::addApplicationFont(":/fonts/roboto/Roboto-LightItalic.ttf");
    QFontDatabase::addApplicationFont(":/fonts/roboto/Roboto-Medium.ttf");
    QFontDatabase::addApplicationFont(":/fonts/roboto/Roboto-MediumItalic.ttf");
    QFontDatabase::addApplicationFont(":/fonts/roboto/Roboto-Regular.ttf");
    QFontDatabase::addApplicationFont(":/fonts/roboto/Roboto-Thin.ttf");
    QFontDatabase::addApplicationFont(":/fonts/roboto/Roboto-ThinItalic.ttf");
    QFontDatabase::addApplicationFont(":/fonts/roboto/RobotoCondensed-Bold.ttf");
    QFontDatabase::addApplicationFont(":/fonts/roboto/RobotoCondensed-BoldItalic.ttf");
    QFontDatabase::addApplicationFont(":/fonts/roboto/RobotoCondensed-Italic.ttf");
    QFontDatabase::addApplicationFont(":/fonts/roboto/RobotoCondensed-Light.ttf");
    QFontDatabase::addApplicationFont(":/fonts/roboto/RobotoCondensed-LightItalic.ttf");
    QFontDatabase::addApplicationFont(":/fonts/roboto/RobotoCondensed-Medium.ttf");
    QFontDatabase::addApplicationFont(":/fonts/roboto/RobotoCondensed-MediumItalic.ttf");
    QFontDatabase::addApplicationFont(":/fonts/roboto/RobotoCondensed-Regular.ttf");
	QFontDatabase::addApplicationFont(":/fonts/materialicons/MaterialIcons-Regular.ttf");

	auto backend = std::make_shared<Backend>();

	QLoggingCategory::setFilterRules("qt.remoteobjects=true\n"
		"qt.remoteobjects.*=true");

	QRemoteObjectHost srcNode(QUrl(QStringLiteral("local:switch")));
	Frontend srcFrontend(backend);
	srcNode.enableRemoting(&srcFrontend);

	/////////////////////////////////
	qmlRegisterType<FrontendQmlReplica>("Huestacean.Frontend", 1, 0, "FrontendQmlReplica");

	/////////////////////////////////
	
	engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
	if (engine.rootObjects().isEmpty())
		return -1;

	engine.rootContext()->setContextProperty("mainWindow", engine.rootObjects().first());

#ifdef Q_OS_ANDROID
	constexpr auto isMobile = true;
#else
	constexpr auto isMobile = false;
#endif
	engine.rootContext()->setContextProperty("isMobile", isMobile);


	auto ret = app.exec();

#ifdef _WIN32
	if (pout) {
		fclose(pout);
	}
	if (perr) {
		fclose(perr);
	}
	if (pin) {
		fclose(pin);
	}
#endif

	return ret;
}

/////////////////////////////////////////////////////////////////

GuiHelper::GuiHelper(QQmlEngine* inEngine, QJSEngine* inScriptEngine)
	: QObject(inEngine), engine(qobject_cast<QQmlApplicationEngine*>(inEngine)), scriptEngine(inScriptEngine)
{

}

GuiHelper::~GuiHelper()
{

}

void GuiHelper::PressedEnter()
{
	QObject* rootObject = engine->rootObjects().first();
	QObject* qmlObject = rootObject->findChild<QObject*>("window");

	QKeyEvent* event = new QKeyEvent(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier);
	QCoreApplication::postEvent(rootObject, event);

	event = new QKeyEvent(QEvent::KeyRelease, Qt::Key_Space, Qt::NoModifier);
	QCoreApplication::postEvent(rootObject, event);
}