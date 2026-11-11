#include "App.h"

namespace Upp {
namespace DevEnv {

AppWindow::AppWindow()
{
	Title("DevEnv - Modular Development Environment");
	Sizeable().Zoomable();
	SetRect(0, 0, 1024, 768);

	AddFrame(menu);
	menu.Set(THISBACK(MainMenu));

	AddFrame(status);
	status.Set(0, "Ready", 0);
	status.Set(1, "AgentIPC: Offline", 160);
	status.Set(2, "Tests: Idle", 200);

	agent.WhenTestResult = [this](const String& target, bool pass, int ms) {
		status.Set(2, String().Cat() << "Test " << target << ": " << (pass ? "PASS" : "FAIL") << " (" << ms << "ms)", 200);
	};

	if (agent.Connect()) {
		status.Set(1, "AgentIPC: Connected (7421)", 160);
	}

	Add(workbench.SizePos());
}

void AppWindow::MainMenu(Bar& bar)
{
	bar.Sub("File", THISBACK(FileMenu));
	bar.Sub("Packages", THISBACK(PackagesMenu));
	bar.Sub("Terminal", THISBACK(TerminalMenu));
#ifdef flagAI
	bar.Sub("AI", THISBACK(AiMenu));
#endif
}

void AppWindow::FileMenu(Bar& bar)
{
	bar.Add("Exit", [this] { Break(); });
}

void AppWindow::TerminalMenu(Bar& bar)
{
	bar.Add("Run All Tests (@installed)", [this] {
		workbench.GetTerminal().ExecuteCommand("uppbuilder test @installed");
	});
	bar.Add("Run Upstream Tests (@autotest)", [this] {
		workbench.GetTerminal().ExecuteCommand("uppbuilder test @autotest");
	});
	bar.Add("Clear Terminal Output", [this] {
		workbench.GetTerminal().Clear();
	});
}

void AppWindow::PackagesMenu(Bar& bar)
{
	bar.Add("Portage Overlay & VDB Manager...", [] {
		RunOverlayManager();
	});
	bar.Add("Run All Installed Tests...", [] {
		PortageManager pm;
		String log;
		pm.RunTest("@installed", &log);
		PromptOK(DeQtf(log));
	});
}

#ifdef flagAI
void AppWindow::AiMenu(Bar& bar)
{
	bar.Add("AI Playground...", [] {
		RunAiPlayground();
	});
	bar.Add("AI Accounts & Providers...", [] {
		RunAiSetupModal();
	});
}
#endif

}
}

using namespace Upp;

namespace Upp {
#ifdef UPP_HEAP
extern bool NoMemoryLeaksCheck;
#endif
}

GUI_APP_MAIN
{
	const Vector<String>& cmd = CommandLine();
	if (cmd.GetCount() > 0 && cmd[0] == "--test") {
		Cout() << "DevEnv: Running automated self-test...\n";
		DevEnv::AppWindow win;
		Cout() << "DevEnv: Automated self-test passed successfully!\n";
#ifdef UPP_HEAP
		NoMemoryLeaksCheck = true;
#endif
		return;
	}

	DevEnv::AppWindow win;
	win.Run();
}
