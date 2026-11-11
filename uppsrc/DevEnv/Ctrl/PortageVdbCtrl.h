#ifndef _DevEnv_Ctrl_PortageVdbCtrl_h_
#define _DevEnv_Ctrl_PortageVdbCtrl_h_

#include <CtrlLib/CtrlLib.h>
#include <DevEnv/Core/Core.h>

namespace Upp {
namespace DevEnv {

class PortageVdbCtrl : public TopWindow {
public:
	typedef PortageVdbCtrl CLASSNAME;
	PortageVdbCtrl();
	virtual ~PortageVdbCtrl() = default;

	void Reload();

private:
	void OnSelectPackage();
	void DoEmerge();
	void DoTest();
	void DoUnmerge();
	void DoRunAllTests();

	PortageManager        pm;
	Vector<PortagePackage> packages;

	Splitter              main_splitter;
	Splitter              right_splitter;

	// Left panel
	ParentCtrl            left_pane;
	Label                 lbl_filter;
	EditString            filter;
	ArrayCtrl             package_list;

	// Right top panel (Details & actions)
	ParentCtrl            details_pane;
	Label                 lbl_details;
	DocEdit               txt_details;
	Button                btn_emerge;
	Button                btn_test;
	Button                btn_unmerge;
	Button                btn_all_tests;
	Button                btn_refresh;
	Label                 lbl_agent_status;
	Label                 lbl_test_status;
	AgentBridge           agent_bridge;

	// Right bottom panel (Console log)
	ParentCtrl            console_pane;
	Label                 lbl_console;
	DocEdit               txt_console;
};

void RunOverlayManager();

}
}

#endif
