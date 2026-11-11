#ifndef _DevEnv_Ctrl_WorkbenchTerminalCtrl_h_
#define _DevEnv_Ctrl_WorkbenchTerminalCtrl_h_

#include <CtrlLib/CtrlLib.h>
#include <DevEnv/Core/Core.h>

namespace Upp {
namespace DevEnv {

class WorkbenchTerminalCtrl : public ParentCtrl {
public:
	typedef WorkbenchTerminalCtrl CLASSNAME;
	WorkbenchTerminalCtrl();
	virtual ~WorkbenchTerminalCtrl();

	void ExecuteCommand(const String& cmd);
	void StopProcess();
	void AppendOutput(const String& text);
	void Clear();
	bool IsRunning();

	Event<String, int> WhenProcessFinished; // command, exit_code

private:
	void OnCommandEntered();
	void OnTimer();

	DocEdit           terminal_view;
	EditString        cmd_input;
	Button            btn_send;
	Button            btn_stop;
	Button            btn_clear;
	Label             lbl_status;

	One<LocalProcess> process;
	String            current_cmd;
	Vector<String>    history;
	int               history_idx = -1;
};

}
}

#endif
