//   Copyright 2019 Brady Brenot
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.

#include <QCoreApplication>

#include "frontend/gui/gui.h"

//@TODO: separate executable for pure CLI mode (bunch of #if in here)
//@TODO: separate handling for foreground and background Android processes (command line arg handling)

int main(int argc, char* argv[])
{
	/////////////////////
	// Qt settings
	QCoreApplication::setOrganizationName("Brady Brenot");
	QCoreApplication::setOrganizationDomain("bradybrenot.com");
	QCoreApplication::setApplicationName("Huestacean");
	/////////////////////
	
	return Gui::Main(argc, argv);
}