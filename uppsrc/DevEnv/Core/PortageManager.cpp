#include "PortageManager.h"

namespace Upp {
namespace DevEnv {

String PortageManager::GetVdbDir()
{
	String custom = GetEnv("UPPBUILDER_VDB_DIR");
	if (!custom.IsEmpty()) return custom;

#ifdef PLATFORM_WIN32
	String appdata = GetEnv("APPDATA");
	if (!appdata.IsEmpty()) return AppendFileName(appdata, "uppbuilder/vdb");
	return "C:\\uppbuilder\\vdb";
#else
	String xdg = GetEnv("XDG_CONFIG_HOME");
	if (!xdg.IsEmpty()) return AppendFileName(xdg, "uppbuilder/vdb");
	String home = GetEnv("HOME");
	if (!home.IsEmpty()) return AppendFileName(home, ".config/uppbuilder/vdb");
	return "/var/db/uppbuilder/vdb";
#endif
}

String PortageManager::GetOverlayDir()
{
	String custom = GetEnv("UPPOVERLAY_DIR");
	if (!custom.IsEmpty() && DirectoryExists(custom)) return custom;

	Vector<String> candidates;
	String home = GetHomeDirectory();
	if (!home.IsEmpty()) {
		candidates.Add(AppendFileName(home, "OuluBSD/UppOverlay"));
		candidates.Add(AppendFileName(home, "Dev/UppOverlay"));
		candidates.Add(AppendFileName(home, ".config/uppbuilder/overlay"));
	}
	candidates.Add("/usr/local/share/uppbuilder/overlay");

	for (const auto& c : candidates) {
		if (DirectoryExists(c) && DirectoryExists(AppendFileName(c, "profiles"))) {
			return c;
		}
	}
	return candidates.GetCount() > 0 ? candidates[0] : "";
}

Vector<PortagePackage> PortageManager::ListPackages()
{
	Vector<PortagePackage> pkgs;
	String overlay = GetOverlayDir();
	String vdb = GetVdbDir();

	Index<String> seen_atoms;

	// 1. Scan overlay directory for available packages
	if (DirectoryExists(overlay)) {
		FindFile ff_cat(AppendFileName(overlay, "*"));
		while (ff_cat) {
			if (ff_cat.IsFolder() && !ff_cat.GetName().StartsWith(".") &&
			    ff_cat.GetName() != "metadata" && ff_cat.GetName() != "profiles")
			{
				String cat = ff_cat.GetName();
				FindFile ff_pkg(AppendFileName(ff_cat.GetPath(), "*"));
				while (ff_pkg) {
					if (ff_pkg.IsFolder() && !ff_pkg.GetName().StartsWith(".")) {
						String pkg_name = ff_pkg.GetName();
						String atom = cat + "/" + pkg_name;

						FindFile ff_eb(AppendFileName(ff_pkg.GetPath(), "*.escbuild"));
						if (!ff_eb) {
							ff_eb.Search(AppendFileName(ff_pkg.GetPath(), "*.ebuild"));
						}

						PortagePackage item;
						item.atom = atom;
						item.category = cat;
						item.name = pkg_name;
						item.version = "0.1.0";

						if (ff_eb) {
							String content = LoadFile(ff_eb.GetPath());
							// Extract DESCRIPTION and VERSION
							int dpos = content.Find("DESCRIPTION");
							if (dpos >= 0) {
								int q1 = content.Find('"', dpos);
								if (q1 >= 0) {
									int q2 = content.Find('"', q1 + 1);
									if (q2 >= 0) item.description = content.Mid(q1 + 1, q2 - q1 - 1);
								}
							}
							int vpos = content.Find("VERSION");
							if (vpos >= 0) {
								int q1 = content.Find('"', vpos);
								if (q1 >= 0) {
									int q2 = content.Find('"', q1 + 1);
									if (q2 >= 0) item.version = content.Mid(q1 + 1, q2 - q1 - 1);
								}
							}
						}

						// Check if installed in VDB
						PortagePackage vdb_info;
						if (IsInstalled(atom, &vdb_info)) {
							item.installed = true;
							item.profile = vdb_info.profile;
							item.use = vdb_info.use;
							item.repo_path = vdb_info.repo_path;
							if (!vdb_info.version.IsEmpty()) item.version = vdb_info.version;
						}

						pkgs.Add(item);
						seen_atoms.Add(atom);
					}
					ff_pkg.Next();
				}
			}
			ff_cat.Next();
		}
	}

	// 2. Scan VDB for installed packages that might not be in overlay
	if (DirectoryExists(vdb)) {
		FindFile ff_cat(AppendFileName(vdb, "*"));
		while (ff_cat) {
			if (ff_cat.IsFolder() && !ff_cat.GetName().StartsWith(".")) {
				String cat = ff_cat.GetName();
				FindFile ff_pf(AppendFileName(ff_cat.GetPath(), "*"));
				while (ff_pf) {
					if (ff_pf.IsFolder() && !ff_pf.GetName().StartsWith(".")) {
						String atom_file = AppendFileName(ff_pf.GetPath(), "ATOM");
						String atom = TrimBoth(LoadFile(atom_file));
						if (atom.IsEmpty()) {
							String pf = ff_pf.GetName();
							int h = pf.ReverseFind('-');
							String name = (h >= 0) ? pf.Mid(0, h) : pf;
							atom = cat + "/" + name;
						}

						if (seen_atoms.Find(atom) < 0) {
							PortagePackage item;
							item.atom = atom;
							int s = atom.Find('/');
							item.category = (s >= 0) ? atom.Mid(0, s) : cat;
							item.name = (s >= 0) ? atom.Mid(s + 1) : atom;
							item.installed = true;
							item.version = TrimBoth(LoadFile(AppendFileName(ff_pf.GetPath(), "VERSION")));
							item.profile = TrimBoth(LoadFile(AppendFileName(ff_pf.GetPath(), "PROFILE")));
							item.use = TrimBoth(LoadFile(AppendFileName(ff_pf.GetPath(), "USE")));
							item.repo_path = TrimBoth(LoadFile(AppendFileName(ff_pf.GetPath(), "REPO")));
							pkgs.Add(item);
							seen_atoms.Add(atom);
						}
					}
					ff_pf.Next();
				}
			}
			ff_cat.Next();
		}
	}

	return pkgs;
}

bool PortageManager::IsInstalled(const String& atom, PortagePackage* out_info)
{
	String vdb = GetVdbDir();
	if (!DirectoryExists(vdb)) return false;

	String cat = "sys";
	String pkg = atom;
	int slash = atom.Find('/');
	if (slash >= 0) {
		cat = atom.Mid(0, slash);
		pkg = atom.Mid(slash + 1);
	}

	String cat_dir = AppendFileName(vdb, cat);
	if (!DirectoryExists(cat_dir)) return false;

	FindFile ff(AppendFileName(cat_dir, "*"));
	while (ff) {
		if (ff.IsFolder()) {
			String dirname = ff.GetName();
			if (dirname == pkg || dirname.StartsWith(pkg + "-")) {
				if (out_info) {
					out_info->atom = atom;
					out_info->category = cat;
					out_info->name = pkg;
					out_info->installed = true;
					out_info->version = TrimBoth(LoadFile(AppendFileName(ff.GetPath(), "VERSION")));
					out_info->profile = TrimBoth(LoadFile(AppendFileName(ff.GetPath(), "PROFILE")));
					out_info->use = TrimBoth(LoadFile(AppendFileName(ff.GetPath(), "USE")));
					out_info->repo_path = TrimBoth(LoadFile(AppendFileName(ff.GetPath(), "REPO")));
				}
				return true;
			}
		}
		ff.Next();
	}
	return false;
}

String PortageManager::ExecuteUppBuilder(const String& args)
{
	String cmd = "uppbuilder " + args + " 2>&1";
	String out = Sys(cmd);
	return out;
}

bool PortageManager::Emerge(const String& atom, bool test, String* out_log)
{
	String args = "emerge ";
	if (test) args << "--test ";
	args << atom;
	String log = ExecuteUppBuilder(args);
	if (out_log) *out_log = log;
	return log.Find("Successfully emerged") >= 0;
}

bool PortageManager::Unmerge(const String& atom, bool force, String* out_log)
{
	String args = "unmerge ";
	if (force) args << "--force ";
	args << atom;
	String log = ExecuteUppBuilder(args);
	if (out_log) *out_log = log;
	return log.Find("Successfully unmerged") >= 0;
}

bool PortageManager::RunTest(const String& atom, String* out_log)
{
	String args = "test " + atom;
	String log = ExecuteUppBuilder(args);
	if (out_log) *out_log = log;
	return log.Find("[PASS]") >= 0 || log.Find("0 failed") >= 0;
}

}
}
