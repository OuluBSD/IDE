#ifndef _DevEnv_App_App_h_
#define _DevEnv_App_App_h_

#include <DevEnv/Ctrl/Lib/Lib.h>
#include <DevEnv/Ctrl/Ctrl.h>
#include <DevEnv/Core/Core.h>

#ifdef flagAI
#include <SuperIntelligence/Ctrl/Ctrl.h>
#endif

namespace Upp {
namespace DevEnv {

class AppWindow : public TopWindow {
public:
	typedef AppWindow CLASSNAME;
	AppWindow();
	virtual ~AppWindow() = default;

	void MainMenu(Bar& bar);
	void FileMenu(Bar& bar);
	void PackagesMenu(Bar& bar);
	void TerminalMenu(Bar& bar);
	void AiMenu(Bar& bar);

private:
	MenuBar         menu;
	StatusBar       status;
	WorkbenchCtrl   workbench;
	AgentBridge     agent;
};

}
}

#endif

