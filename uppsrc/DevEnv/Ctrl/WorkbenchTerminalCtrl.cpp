#include "WorkbenchTerminalCtrl.h"

namespace Upp {
namespace DevEnv {

WorkbenchTerminalCtrl::WorkbenchTerminalCtrl()
{
	terminal_view.SetFont(Monospace());
	terminal_view.SetReadOnly();
	Add(terminal_view.HSizePos(4, 4).VSizePos(4, 34));

	cmd_input.WhenEnter = THISBACK(OnCommandEntered);
	Add(cmd_input.HSizePos(4, 210).BottomPos(4, 26));

	btn_send.SetLabel("Run");
	btn_send.WhenAction = THISBACK(OnCommandEntered);
	Add(btn_send.RightPos(140, 65).BottomPos(4, 26));

	btn_stop.SetLabel("Stop");
	btn_stop.WhenAction = THISBACK(StopProcess);
	btn_stop.Enable(false);
	Add(btn_stop.RightPos(72, 65).BottomPos(4, 26));

	btn_clear.SetLabel("Clear");
	btn_clear.WhenAction = THISBACK(Clear);
	Add(btn_clear.RightPos(4, 65).BottomPos(4, 26));

	AppendOutput("DevEnv Terminal ready. Enter a command (e.g. 'uppbuilder test autotest/Bits') below.\n\n");
}

WorkbenchTerminalCtrl::~WorkbenchTerminalCtrl()
{
	StopProcess();
}

void WorkbenchTerminalCtrl::Clear()
{
	terminal_view.Clear();
}

void WorkbenchTerminalCtrl::AppendOutput(const String& text)
{
	terminal_view.Append(text);
	terminal_view.SetCursor(terminal_view.GetLength());
}

bool WorkbenchTerminalCtrl::IsRunning()
{
	return process && process->IsRunning();
}

void WorkbenchTerminalCtrl::OnCommandEntered()
{
	String cmd = TrimBoth(cmd_input.GetData().ToString());
	if (cmd.IsEmpty()) return;

	cmd_input.SetData("");
	history.Add(cmd);
	history_idx = history.GetCount();

	ExecuteCommand(cmd);
}

void WorkbenchTerminalCtrl::ExecuteCommand(const String& cmd)
{
	if (IsRunning()) {
		AppendOutput("\n[Notice: Stopping previous process...]\n");
		StopProcess();
	}

	current_cmd = cmd;
	AppendOutput("\n$ " + cmd + "\n");

	process.Create();
	if (!process->Start(cmd)) {
		AppendOutput("Error: Failed to launch command: " + cmd + "\n");
		process.Clear();
		return;
	}

	btn_stop.Enable(true);
	btn_send.Enable(false);

	SetTimeCallback(-50, THISBACK(OnTimer));
}

void WorkbenchTerminalCtrl::StopProcess()
{
	if (process && process->IsRunning()) {
		process->Kill();
		AppendOutput("\n[Process terminated by user]\n");
	}
	KillTimeCallback();
	btn_stop.Enable(false);
	btn_send.Enable(true);
	process.Clear();
}

void WorkbenchTerminalCtrl::OnTimer()
{
	if (!process) {
		KillTimeCallback();
		return;
	}

	String chunk;
	if (process->Read(chunk)) {
		if (!chunk.IsEmpty()) {
			AppendOutput(chunk);
		}
	}

	if (!process->IsRunning()) {
		// Read any trailing bytes
		String tail;
		while (process->Read(tail) && !tail.IsEmpty()) {
			AppendOutput(tail);
		}

		int code = process->GetExitCode();
		AppendOutput(String().Cat() << "\n[Process exited with code " << code << "]\n");

		KillTimeCallback();
		btn_stop.Enable(false);
		btn_send.Enable(true);

		if (WhenProcessFinished) {
			WhenProcessFinished(current_cmd, code);
		}

		process.Clear();
	}
}

}
}
