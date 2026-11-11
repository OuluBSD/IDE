#include "VfsExplorerCtrl.h"

namespace Upp {
namespace DevEnv {

VfsExplorerCtrl::VfsExplorerCtrl()
{
	btn_refresh.SetLabel("Refresh");
	btn_refresh.WhenAction = THISBACK(Refresh);
	Add(btn_refresh.LeftPos(4, 80).TopPos(4, 24));

	btn_run_test.SetLabel("Test Pkg");
	btn_run_test.WhenAction = [this] {
		String p = GetSelectedPath();
		if (!p.IsEmpty() && WhenRunTest) {
			WhenRunTest(p);
		}
	};
	Add(btn_run_test.RightPos(4, 80).TopPos(4, 24));

	tree.WhenCursor = THISBACK(OnNodeCursor);
	tree.WhenOpen = THISBACK(OnItemOpen);
	Add(tree.HSizePos(4, 4).VSizePos(32, 4));

	root_path = GetCurrentDirectory();
	Refresh();
}

void VfsExplorerCtrl::SetRoot(const String& path)
{
	root_path = path;
	Refresh();
}

void VfsExplorerCtrl::Refresh()
{
	tree.Clear();
	node_paths.Clear();

	if (root_path.IsEmpty() || !DirectoryExists(root_path)) {
		return;
	}

	String title = GetFileName(root_path);
	if (title.IsEmpty()) title = root_path;

	tree.SetRoot(CtrlImg::Dir(), title);
	int root_id = 0;
	node_paths.Add(root_id, root_path);

	PopulateNode(root_id, root_path);
	tree.Open(root_id);
}

void VfsExplorerCtrl::PopulateNode(int parent_id, const String& dir_path)
{
	FindFile ff(AppendFileName(dir_path, "*"));
	Vector<String> dirs;
	Vector<String> files;

	while (ff) {
		String name = ff.GetName();
		if (name != "." && name != ".." && !name.StartsWith(".")) {
			if (ff.IsFolder()) {
				if (name != "bin" && name != "out" && name != ".git" && name != ".cache" && name != "tmp") {
					dirs.Add(name);
				}
			} else {
				files.Add(name);
			}
		}
		ff.Next();
	}

	Sort(dirs);
	Sort(files);

	for (const auto& d : dirs) {
		String child_path = AppendFileName(dir_path, d);
		int id = tree.Add(parent_id, CtrlImg::Dir(), d);
		node_paths.Add(id, child_path);
	}

	for (const auto& f : files) {
		String child_path = AppendFileName(dir_path, f);
		int id = tree.Add(parent_id, CtrlImg::File(), f);
		node_paths.Add(id, child_path);
	}
}

void VfsExplorerCtrl::OnItemOpen(int id)
{
	int idx = node_paths.Find(id);
	if (idx < 0) return;
	String p = node_paths[idx];

	if (DirectoryExists(p) && tree.GetChildCount(id) == 0) {
		PopulateNode(id, p);
	}
}

void VfsExplorerCtrl::OnNodeCursor()
{
	int id = tree.GetCursor();
	int idx = node_paths.Find(id);
	if (idx < 0) return;

	String p = node_paths[idx];
	if (FileExists(p) && WhenFileSelected) {
		WhenFileSelected(p);
	}
}

String VfsExplorerCtrl::GetSelectedPath() const
{
	int id = tree.GetCursor();
	int idx = node_paths.Find(id);
	if (idx < 0) return String();
	return node_paths[idx];
}

}
}
