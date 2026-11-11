#ifndef _DevEnv_Ctrl_VfsExplorerCtrl_h_
#define _DevEnv_Ctrl_VfsExplorerCtrl_h_

#include <CtrlLib/CtrlLib.h>
#include <DevEnv/Core/Core.h>

namespace Upp {
namespace DevEnv {

class VfsExplorerCtrl : public ParentCtrl {
public:
	typedef VfsExplorerCtrl CLASSNAME;
	VfsExplorerCtrl();
	virtual ~VfsExplorerCtrl() = default;

	void SetRoot(const String& path);
	void Refresh();
	String GetSelectedPath() const;

	Event<String> WhenFileSelected;
	Event<String> WhenRunTest;

private:
	void OnNodeCursor();
	void OnItemOpen(int id);
	void PopulateNode(int parent_id, const String& dir_path);

	TreeCtrl                tree;
	Button                  btn_refresh;
	Button                  btn_run_test;
	String                  root_path;
	VectorMap<int, String>  node_paths;
};

}
}

#endif
