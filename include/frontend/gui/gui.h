#pragma once

#include <QObject>

class QQmlApplicationEngine;
class QQmlEngine;
class QJSEngine;

namespace Gui
{
	int Main(int argc, char* argv[]);

	class Frontend : public QObject
	{
		Q_OBJECT;

	public:
		Frontend(QQmlEngine* inEngine, QJSEngine* inScriptEngine);
		virtual ~Frontend();

		Q_INVOKABLE void PressedEnter();

		QQmlApplicationEngine* engine;
		QJSEngine* scriptEngine;
	};
};