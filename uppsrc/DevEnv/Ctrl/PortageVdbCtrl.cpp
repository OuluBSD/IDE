#include "PortageVdbCtrl.h"

namespace Upp {
namespace DevEnv {

PortageVdbCtrl::PortageVdbCtrl()
{
	Title("Portage Overlay & VDB Package Manager");
	Sizeable().Zoomable();
	SetRect(0, 0, 960, 640);

	// Setup Left Pane
	lbl_filter.SetText("Filter:");
	left_pane.Add(lbl_filter.LeftPos(4, 45).TopPos(4, 24));
	left_pane.Add(filter.HSizePos(55, 4).TopPos(4, 24));
	filter.WhenAction = THISBACK(Reload);

	package_list.AddColumn("Status", 80);
	package_list.AddColumn("Atom", 180);
	package_list.AddColumn("Version", 70);
	package_list.AddColumn("Description", 250);
	package_list.WhenCursor = THISBACK(OnSelectPackage);
	left_pane.Add(package_list.HSizePos(4, 4).VSizePos(34, 4));

	// Setup Details Pane
	lbl_details.SetText("Package Details & Operations:");
	details_pane.Add(lbl_details.LeftPos(4, 250).TopPos(4, 20));

	txt_details.SetReadOnly();
	details_pane.Add(txt_details.HSizePos(4, 4).VSizePos(28, 44));

	btn_emerge.SetLabel("Emerge");
	btn_emerge.WhenAction = THISBACK(DoEmerge);
	details_pane.Add(btn_emerge.LeftPos(4, 80).BottomPos(4, 32));

	btn_test.SetLabel("Run Test");
	btn_test.WhenAction = THISBACK(DoTest);
	details_pane.Add(btn_test.LeftPos(90, 80).BottomPos(4, 32));

	btn_unmerge.SetLabel("Unmerge");
	btn_unmerge.WhenAction = THISBACK(DoUnmerge);
	details_pane.Add(btn_unmerge.LeftPos(176, 80).BottomPos(4, 32));

	btn_all_tests.SetLabel("Run All Tests");
	btn_all_tests.WhenAction = THISBACK(DoRunAllTests);
	details_pane.Add(btn_all_tests.LeftPos(262, 110).BottomPos(4, 32));

	btn_refresh.SetLabel("Refresh");
	btn_refresh.WhenAction = THISBACK(Reload);
	details_pane.Add(btn_refresh.RightPos(4, 80).BottomPos(4, 32));

	lbl_agent_status.SetText("AgentIPC: Offline");
	lbl_agent_status.SetInk(Gray());
	details_pane.Add(lbl_agent_status.LeftPos(4, 180).BottomPos(42, 20));

	lbl_test_status.SetText("Tests: Idle");
	lbl_test_status.SetInk(Gray());
	details_pane.Add(lbl_test_status.LeftPos(190, 220).BottomPos(42, 20));

	agent_bridge.WhenTestResult = [this](const String& target, bool pass, int ms) {
		lbl_test_status.SetText(String().Cat() << "Test [" << target << "]: " << (pass ? "PASS" : "FAIL") << " (" << ms << " ms)");
		lbl_test_status.SetInk(pass ? LtGreen() : LtRed());
	};

	if (agent_bridge.Connect()) {
		lbl_agent_status.SetText("AgentIPC: Connected (7421)");
		lbl_agent_status.SetInk(LtGreen());
	}

	// Setup Console Pane
	lbl_console.SetText("Process Execution Log:");
	console_pane.Add(lbl_console.LeftPos(4, 200).TopPos(4, 20));
	txt_console.SetReadOnly();
	console_pane.Add(txt_console.HSizePos(4, 4).VSizePos(26, 4));

	// Connect Splitters
	right_splitter.Vert(details_pane, console_pane);
	right_splitter.SetPos(5500);

	main_splitter.Horz(left_pane, right_splitter);
	main_splitter.SetPos(4500);

	Add(main_splitter.SizePos());

	Reload();
}

void PortageVdbCtrl::Reload()
{
	packages = pm.ListPackages();
	package_list.Clear();

	String q = ToLower(filter.GetData().ToString());

	for (int i = 0; i < packages.GetCount(); i++) {
		const auto& p = packages[i];
		if (!q.IsEmpty()) {
			String haystack = ToLower(p.atom + " " + p.description);
			if (haystack.Find(q) < 0) continue;
		}

		String status = p.installed ? "Installed" : "Available";
		package_list.Add(status, p.atom, p.version, p.description);
	}

	if (package_list.GetCount() > 0) {
		package_list.SetCursor(0);
	} else {
		txt_details.SetData("");
	}
}

void PortageVdbCtrl::OnSelectPackage()
{
	int cursor = package_list.GetCursor();
	if (cursor < 0) {
		txt_details.SetData("");
		return;
	}

	String atom = package_list.Get(cursor, 1);
	PortagePackage info;
	for (const auto& p : packages) {
		if (p.atom == atom) {
			info = p;
			break;
		}
	}

	String text;
	text << "Atom:        " << info.atom << "\n"
	     << "Category:    " << info.category << "\n"
	     << "Package:     " << info.name << "\n"
	     << "Version:     " << info.version << "\n"
	     << "Status:      " << (info.installed ? "INSTALLED in VDB" : "Available in Overlay") << "\n";

	if (info.installed) {
		text << "Profile:     " << info.profile << "\n"
		     << "USE Flags:   " << info.use << "\n"
		     << "Repository:  " << info.repo_path << "\n";
	}

	if (!info.description.IsEmpty()) {
		text << "Description: " << info.description << "\n";
	}

	txt_details.SetData(text);
	btn_unmerge.Enable(info.installed);
}

void PortageVdbCtrl::DoEmerge()
{
	int cursor = package_list.GetCursor();
	if (cursor < 0) return;
	String atom = package_list.Get(cursor, 1);

	txt_console.SetData(">>> Emerging package: " + atom + " ...\n");
	Sync();

	String log;
	bool ok = pm.Emerge(atom, false, &log);
	txt_console.SetData(log);

	Reload();
}

void PortageVdbCtrl::DoTest()
{
	int cursor = package_list.GetCursor();
	if (cursor < 0) return;
	String atom = package_list.Get(cursor, 1);

	txt_console.SetData(">>> Running tests for: " + atom + " ...\n");
	Sync();

	String log;
	bool ok = pm.RunTest(atom, &log);
	txt_console.SetData(log);

	AgentMessage msg;
	msg.type = "test_result";
	msg.target = atom;
	msg.status = ok ? "pass" : "fail";
	msg.message = ok ? "Test suite passed" : "Test suite failed";
	agent_bridge.SendEvent(msg);
	if (agent_bridge.WhenTestResult) {
		agent_bridge.WhenTestResult(atom, ok, 0);
	}
}

void PortageVdbCtrl::DoUnmerge()
{
	int cursor = package_list.GetCursor();
	if (cursor < 0) return;
	String atom = package_list.Get(cursor, 1);

	if (!PromptYesNo("Are you sure you want to unmerge " + atom + "?")) {
		return;
	}

	txt_console.SetData(">>> Unmerging package: " + atom + " ...\n");
	Sync();

	String log;
	bool ok = pm.Unmerge(atom, false, &log);
	txt_console.SetData(log);

	Reload();
}

void PortageVdbCtrl::DoRunAllTests()
{
	txt_console.SetData(">>> Running full-stack test suite across @installed packages...\n");
	Sync();

	String log;
	bool ok = pm.RunTest("@installed", &log);
	txt_console.SetData(log);

	AgentMessage msg;
	msg.type = "test_result";
	msg.target = "@installed";
	msg.status = ok ? "pass" : "fail";
	msg.message = ok ? "Test suite passed" : "Test suite failed";
	agent_bridge.SendEvent(msg);
	if (agent_bridge.WhenTestResult) {
		agent_bridge.WhenTestResult("@installed", ok, 0);
	}
}

void RunOverlayManager()
{
	PortageVdbCtrl dlg;
	dlg.Execute();
}

}
}
