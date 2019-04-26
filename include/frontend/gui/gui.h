#pragma once

#include <QObject>

class QQmlApplicationEngine;
class QQmlEngine;
class QJSEngine;

namespace Gui
{
	int Main(int argc, char* argv[]);

	class GuiHelper : public QObject
	{
		Q_OBJECT;

	public:
		GuiHelper(QQmlEngine* inEngine, QJSEngine* inScriptEngine);
		virtual ~GuiHelper();

		Q_INVOKABLE void PressedEnter();

		QQmlApplicationEngine* engine;
		QJSEngine* scriptEngine;
	};
};