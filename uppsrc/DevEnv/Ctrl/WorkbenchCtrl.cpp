#include "WorkbenchCtrl.h"

namespace Upp {
namespace DevEnv {

WorkbenchCtrl::WorkbenchCtrl()
{
	// 1. Sidebar Pane setup
	lbl_sidebar.SetText("Filesystem & Virtual Packages:");
	sidebar_pane.Add(lbl_sidebar.HSizePos(4, 4).TopPos(4, 20));
	sidebar_pane.Add(vfs_explorer.HSizePos(4, 4).VSizePos(26, 4));

	vfs_explorer.WhenFileSelected = THISBACK(OnFileSelected);
	vfs_explorer.WhenRunTest = THISBACK(OnRunTestFromVfs);

	// 2. Editor Pane setup
	lbl_current_file.SetText("No file selected");
	editor_pane.Add(lbl_current_file.HSizePos(4, 4).TopPos(4, 20));
	file_editor.SetFont(Monospace());
	editor_pane.Add(file_editor.HSizePos(4, 4).VSizePos(26, 4));

	// 3. Center Tabs (Editor & AI Assistant)
	center_tabs.Add(editor_pane.SizePos(), "Source Editor");
	center_tabs.Add(ai_assistant.SizePos(), "AI Assistant");

	// 4. Work Splitter (Center Editor vs Bottom Terminal)
	work_splitter.Vert(center_tabs, terminal);
	work_splitter.SetPos(6500);

	// 5. Main Splitter (Left Sidebar vs Right Work Area)
	main_splitter.Horz(sidebar_pane, work_splitter);
	main_splitter.SetPos(2800);

	Add(main_splitter.SizePos());

	// Connect AgentBridge
	agent_bridge.Connect();
}

void WorkbenchCtrl::SetWorkspaceRoot(const String& path)
{
	vfs_explorer.SetRoot(path);
}

void WorkbenchCtrl::OnFileSelected(const String& path)
{
	current_file_path = path;
	lbl_current_file.SetText("Editing: " + path);

	String content = LoadFile(path);
	file_editor.SetData(content);
	center_tabs.Set(0); // Switch to Source Editor tab

	if (WhenOpenFile) {
		WhenOpenFile(path);
	}
}

void WorkbenchCtrl::OnRunTestFromVfs(const String& path)
{
	String atom;
	String norm = NormalizePath(path);

	// Infer atom
	for (const char* prefix : {"autotest", "upptst", "reference", "tutorial", "examples"}) {
		String p_str = String("/applied-") + prefix + "/";
		int p = norm.Find(p_str);
		if (p < 0) p = norm.Find(String("/") + prefix + "/");
		if (p >= 0) {
			int start = norm.Find('/', p + 1);
			if (start >= 0) {
				start++;
				int end = norm.Find('/', start);
				if (end >= 0) {
					atom = String(prefix) + "/" + norm.Mid(start, end - start);
					break;
				}
			}
		}
	}

	if (atom.IsEmpty()) {
		for (const char* cat : {"DevCpp", "DevNetwork", "DevGraphics", "DevUtil", "DevMedia", "DevGui"}) {
			String cat_str = String("/") + cat + "/";
			int p = norm.Find(cat_str);
			if (p >= 0) {
				int start = p + (int)strlen(cat) + 2;
				int end = norm.Find('/', start);
				if (end >= 0) {
					atom = String(cat) + "/" + norm.Mid(start, end - start);
					break;
				}
			}
		}
	}

	if (atom.IsEmpty()) {
		atom = "@installed";
	}

	terminal.ExecuteCommand("uppbuilder test " + atom);
}

}
}
