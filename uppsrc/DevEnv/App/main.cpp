#include "App.h"

namespace Upp {
namespace DevEnv {

AppWindow::AppWindow()
{
	Title("DevEnv - Modular Development Environment");
	Sizeable().Zoomable();
	SetRect(0, 0, 1024, 768);

	Add(ai_assistant.SizePos());
}

}
}

using namespace Upp;

GUI_APP_MAIN
{
	DevEnv::AppWindow win;
	win.Run();
}
