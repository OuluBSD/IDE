#include "AiAssistantCtrl.h"

namespace Upp {
namespace DevEnv {

AiAssistantCtrl::AiAssistantCtrl()
{
	lbl_title.SetText("SWE-Agent Assistant");
	lbl_title.SetFont(StdFont().Bold());
	Add(lbl_title);

	lbl_model.SetText("Model:");
	Add(lbl_model);

	dl_model.Add("openrouter/auto", "Auto Router");
	dl_model.Add("anthropic/claude-3.7-sonnet", "Claude 3.7 Sonnet");
	dl_model.Add("anthropic/claude-3.5-haiku", "Claude 3.5 Haiku");
	dl_model.Add("deepseek/deepseek-chat", "DeepSeek V3");
	dl_model.Add("deepseek/deepseek-r1", "DeepSeek R1");
	dl_model.Add("google/gemini-2.0-flash-001", "Gemini 2.0 Flash");
	dl_model.Add("openai/gpt-4o", "GPT-4o");
	dl_model.Add("openai/gpt-4o-mini", "GPT-4o Mini");
	dl_model.Add("qwen/qwen-2.5-coder-32b-instruct", "Qwen 2.5 Coder 32B");

	String env_model = GetEnv("OPENROUTER_MODEL");
	if(!env_model.IsEmpty()) {
		if(dl_model.FindKey(env_model) < 0)
			dl_model.Add(env_model, env_model);
		dl_model.SetData(env_model);
	}
	else {
		dl_model.SetData("openrouter/auto");
	}
	Add(dl_model);

	lbl_status.SetText("Status: Idle");
	Add(lbl_status);

	btn_clear.SetLabel("Clear Log");
	btn_clear << [this] { OnClearClicked(); };
	Add(btn_clear);

	log_view.NoWantFocus();
	Add(log_view);

	input_prompt.NullText("Describe coding or investigation task for SWE-Agent...");
	input_prompt.WhenEnter = [this] { OnPromptEnter(); };
	Add(input_prompt);

	btn_run.SetLabel("Run");
	btn_run << [this] { OnRunClicked(); };
	Add(btn_run);

	btn_stop.SetLabel("Stop");
	btn_stop.Disable();
	btn_stop << [this] { OnStopClicked(); };
	Add(btn_stop);

	ClearLog();
}

AiAssistantCtrl::~AiAssistantCtrl()
{
	StopTask();
}

void AiAssistantCtrl::ClearLog()
{
	qtf_log = "[*@b SWE-Agent In-Process Assistant Ready.]&";
#ifndef flagAI
	qtf_log << "[*@r Notice: AI subsystem is disabled. Rebuild with flag 'AI' to activate.]&";
#endif
	log_view.SetQTF(qtf_log);
	log_view.ScrollEnd();
}

void AiAssistantCtrl::AppendLog(const String& tag, const String& text, Color color)
{
	String tag_color;
	if(color == LtBlue())        tag_color = "@b ";
	else if(color == LtGreen())  tag_color = "@g ";
	else if(color == LtRed())    tag_color = "@r ";
	else if(color == LtMagenta()) tag_color = "@m ";
	else if(color == LtCyan())   tag_color = "@c ";
	else                         tag_color = "";

	if(!qtf_log.IsEmpty())
		qtf_log << "&";
	qtf_log << "[* " << tag_color << "[" << DeQtf(tag) << ":]] " << DeQtf(text);
	log_view.SetQTF(qtf_log);
	log_view.ScrollEnd();
}

void AiAssistantCtrl::Layout()
{
	Size sz = GetSize();
	if(sz.cx <= 0 || sz.cy <= 0)
		return;

	// Top Header Bar (height 26)
	int y = 4;
	lbl_title.SetRect(8, y, 160, 24);
	lbl_model.SetRect(175, y, 45, 24);
	dl_model.SetRect(222, y, 180, 24);
	lbl_status.SetRect(410, y, sz.cx - 410 - 95, 24);
	btn_clear.SetRect(sz.cx - 90, y, 82, 24);

	// Bottom Action Bar (height 26)
	int by = sz.cy - 30;
	int run_w = 65;
	int stop_w = 65;
	int input_w = sz.cx - 8 - run_w - stop_w - 16;
	if(input_w < 50) input_w = 50;

	input_prompt.SetRect(8, by, input_w, 24);
	btn_run.SetRect(8 + input_w + 6, by, run_w, 24);
	btn_stop.SetRect(8 + input_w + 6 + run_w + 4, by, stop_w, 24);

	// Center Log Transcript
	int log_y = y + 28;
	int log_h = by - log_y - 4;
	if(log_h < 10) log_h = 10;
	log_view.SetRect(8, log_y, sz.cx - 16, log_h);
}

void AiAssistantCtrl::Paint(Draw& w)
{
	Size sz = GetSize();
	w.DrawRect(sz, SColorFace());
}

void AiAssistantCtrl::OnPromptEnter()
{
	OnRunClicked();
}

void AiAssistantCtrl::OnRunClicked()
{
	String task = TrimBoth(input_prompt.GetText().ToString());
	if(task.IsEmpty())
		return;
	RunTask(task);
}

void AiAssistantCtrl::OnStopClicked()
{
	StopTask();
}

void AiAssistantCtrl::OnClearClicked()
{
	ClearLog();
}

void AiAssistantCtrl::RunTask(const String& task)
{
	Mutex::Lock __(state_lock);
	if(running)
		return;

	running = true;
	stop_requested = false;
	btn_run.Disable();
	btn_stop.Enable();
	lbl_status.SetText("Status: Starting SWE-Agent...");

	String model = dl_model.GetData().ToString();
	String ws = workspace_dir.IsEmpty() ? GetCurrentDirectory() : workspace_dir;

	AppendLog("User", task, LtBlue());
	input_prompt.Clear();

	Thread::Start([this, task, model, ws] {
		WorkerLoop(task, model, ws);
	});
}

void AiAssistantCtrl::StopTask()
{
	Mutex::Lock __(state_lock);
	if(!running)
		return;
	stop_requested = true;
	lbl_status.SetText("Status: Stopping...");
}

void AiAssistantCtrl::WorkerLoop(String task, String model, String workspace)
{
#ifdef flagAI
	Agent agent(15);
	agent.config.workspace_dir = workspace;
	agent.config.model_name = model;

	agent.AddTool(new ShellTool);
	agent.AddTool(new FileReadTool);
	agent.AddTool(new FileWriteTool);
	agent.AddTool(new FileEditTool);
	agent.AddTool(new ListDirTool);

	agent.WhenReasoning = [this](const String& r) {
		PostCallback([this, r] { DispatchedReasoning(r); });
	};
	agent.WhenThought = [this](const String& t) {
		PostCallback([this, t] { DispatchedThought(t); });
	};
	agent.WhenAction = [this](const String& name, const ValueMap& args) {
		String desc = AsJSON(Value(args));
		PostCallback([this, name, desc] { DispatchedAction(name, desc); });
	};
	agent.WhenObservation = [this](const String& name, const String& obs) {
		PostCallback([this, name, obs] { DispatchedObservation(name, obs); });
	};
	agent.WhenFinished = [this](const String& ans) {
		PostCallback([this, ans] { DispatchedFinished(ans); });
	};
	agent.WhenError = [this](const String& err) {
		PostCallback([this, err] { DispatchedError(err); });
	};

	PostCallback([this] { DispatchedSetStatus("Thinking..."); });
	agent.ExecuteTask(task);
	PostCallback([this] { DispatchedRunning(false); });
#else
	PostCallback([this] {
		DispatchedError("SWE-Agent is not enabled in this build configuration. Build with flag 'AI' to activate.");
		DispatchedRunning(false);
	});
#endif
}

void AiAssistantCtrl::DispatchedReasoning(String text)
{
	AppendLog("Reasoning", text, LtCyan());
	DispatchedSetStatus("Reasoning...");
}

void AiAssistantCtrl::DispatchedThought(String text)
{
	AppendLog("Thought", text, SColorText());
	DispatchedSetStatus("Thinking...");
}

void AiAssistantCtrl::DispatchedAction(String tool_name, String args_desc)
{
	AppendLog("Action (" + tool_name + ")", args_desc, LtGreen());
	DispatchedSetStatus("Executing: " + tool_name);
}

void AiAssistantCtrl::DispatchedObservation(String tool_name, String obs)
{
	AppendLog("Observation (" + tool_name + ")", obs, LtMagenta());
	DispatchedSetStatus("Processing observation...");
}

void AiAssistantCtrl::DispatchedFinished(String answer)
{
	AppendLog("Finished", answer, LtGreen());
	DispatchedSetStatus("Task Completed");
	if(WhenTaskFinished)
		WhenTaskFinished(answer);
}

void AiAssistantCtrl::DispatchedError(String error_msg)
{
	AppendLog("Error", error_msg, LtRed());
	DispatchedSetStatus("Error");
	if(WhenTaskError)
		WhenTaskError(error_msg);
}

void AiAssistantCtrl::DispatchedSetStatus(String status_text)
{
	lbl_status.SetText("Status: " + status_text);
}

void AiAssistantCtrl::DispatchedRunning(bool is_running)
{
	Mutex::Lock __(state_lock);
	running = is_running;
	if(!running) {
		btn_run.Enable();
		btn_stop.Disable();
	}
}

}
}
