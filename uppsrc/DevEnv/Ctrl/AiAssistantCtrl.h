#ifndef _DevEnv_Ctrl_AiAssistantCtrl_h_
#define _DevEnv_Ctrl_AiAssistantCtrl_h_

#include <CtrlLib/CtrlLib.h>
#include <DevEnv/Draw/Draw.h>

#ifdef flagAI
#include <SweAgent/SweAgent.h>
#endif

namespace Upp {
namespace DevEnv {

class AiAssistantCtrl : public Ctrl {
public:
	typedef AiAssistantCtrl CLASSNAME;

	AiAssistantCtrl();
	virtual ~AiAssistantCtrl();

	void SetWorkspace(const String& path) { workspace_dir = path; }
	const String& GetWorkspace() const    { return workspace_dir; }

	void AppendLog(const String& tag, const String& text, Color color = SColorText());
	void ClearLog();

	void RunTask(const String& task);
	void StopTask();
	bool IsRunning() const                { return running; }

	Event<const String&> WhenTaskFinished;
	Event<const String&> WhenTaskError;

	virtual void Layout() override;
	virtual void Paint(Draw& w) override;

private:
	// Top Header Bar
	Label        lbl_title;
	Label        lbl_model;
	DropList     dl_model;
	Label        lbl_status;
	Button       btn_clear;

	// Center Log Transcript
	RichTextView log_view;
	String       qtf_log;

	// Bottom Action Bar
	EditString   input_prompt;
	Button       btn_run;
	Button       btn_stop;

	String       workspace_dir;
	bool         running = false;
	bool         stop_requested = false;
	Mutex        state_lock;

	void OnRunClicked();
	void OnStopClicked();
	void OnClearClicked();
	void OnPromptEnter();

	void WorkerLoop(String task, String model, String workspace);

	// Dispatches onto main UI thread via PostCallback
	void DispatchedReasoning(String text);
	void DispatchedThought(String text);
	void DispatchedAction(String tool_name, String args_desc);
	void DispatchedObservation(String tool_name, String obs);
	void DispatchedFinished(String answer);
	void DispatchedError(String error_msg);
	void DispatchedSetStatus(String status_text);
	void DispatchedRunning(bool is_running);
};

}
}

#endif
