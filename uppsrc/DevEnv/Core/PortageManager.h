#ifndef _DevEnv_Core_PortageManager_h_
#define _DevEnv_Core_PortageManager_h_

#include <Core/Core.h>

namespace Upp {
namespace DevEnv {

struct PortagePackage : Moveable<PortagePackage> {
	String atom;        // e.g. "DevCpp/BasicHtmlRenderer"
	String category;    // e.g. "DevCpp"
	String name;        // e.g. "BasicHtmlRenderer"
	String version;     // e.g. "0.1.0"
	String profile;     // e.g. "NativeClang"
	String use;         // e.g. "+stdexc +stdrtti"
	String description; // e.g. "Lightweight pure C++ HTML layout engine"
	String repo_path;
	bool   installed = false;
};

class PortageManager {
public:
	PortageManager() = default;

	static String GetVdbDir();
	static String GetOverlayDir();

	Vector<PortagePackage> ListPackages();
	bool IsInstalled(const String& atom, PortagePackage* out_info = nullptr);

	String ExecuteUppBuilder(const String& args);
	bool Emerge(const String& atom, bool test = false, String* out_log = nullptr);
	bool Unmerge(const String& atom, bool force = false, String* out_log = nullptr);
	bool RunTest(const String& atom, String* out_log = nullptr);
};

}
}

#endif
