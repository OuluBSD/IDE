#ifndef _DevEnv_Ctrl_WorkbenchCtrl_h_
#define _DevEnv_Ctrl_WorkbenchCtrl_h_

#include <CtrlLib/CtrlLib.h>
#include <DevEnv/Core/Core.h>
#include "VfsExplorerCtrl.h"
#include "WorkbenchTerminalCtrl.h"
#include "PortageVdbCtrl.h"
#include "AiAssistantCtrl.h"

namespace Upp {
namespace DevEnv {

class WorkbenchCtrl : public ParentCtrl {
public:
	typedef WorkbenchCtrl CLASSNAME;
	WorkbenchCtrl();
	virtual ~WorkbenchCtrl() = default;

	void SetWorkspaceRoot(const String& path);
	VfsExplorerCtrl&       GetVfs()      { return vfs_explorer; }
	WorkbenchTerminalCtrl& GetTerminal() { return terminal; }
	AiAssistantCtrl&       GetAi()       { return ai_assistant; }

	Event<String> WhenOpenFile;

private:
	void OnFileSelected(const String& path);
	void OnRunTestFromVfs(const String& path);

	Splitter              main_splitter;
	Splitter              work_splitter;

	// Left sidebar
	ParentCtrl            sidebar_pane;
	Label                 lbl_sidebar;
	VfsExplorerCtrl       vfs_explorer;

	// Center work area
	TabCtrl               center_tabs;
	ParentCtrl            editor_pane;
	Label                 lbl_current_file;
	DocEdit               file_editor;
	AiAssistantCtrl       ai_assistant;

	// Bottom drawer
	WorkbenchTerminalCtrl terminal;

	AgentBridge           agent_bridge;
	String                current_file_path;
};

}
}

#endif
