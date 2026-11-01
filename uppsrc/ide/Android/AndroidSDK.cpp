#include "Android.h"

#include <plugin/pcre/Pcre.h>

namespace Upp {

String AndroidSDK::GetDownloadUrl()
{
	return "https://developer.android.com/studio/index.html";
}

AndroidSDK::AndroidSDK()
{
}

AndroidSDK::AndroidSDK(const String& path, bool deduceValues)
{
	this->path = path;
	if(deduceValues)
		DeducePathReleatedValues();
}

AndroidSDK::~AndroidSDK()
{
}

void AndroidSDK::DeducePathReleatedValues()
{
	DeducePlatform();
	DeduceBuildToolsRelease();
}

void AndroidSDK::DeducePlatform()
{
	platform = FindDefaultPlatform();
}

void AndroidSDK::DeduceBuildToolsRelease()
{
	build_tools_release = FindDefaultBuildToolsRelease();
}

bool AndroidSDK::Validate() const
{
	if(IsNull(path) || !DirectoryExists(path)) return false;

	// Modern Android SDK validation: check for essential directories
	// build-tools contains build tools (aapt, zipalign, d8...)
	// platforms contains API platforms (android.jar)
	if(!DirectoryExists(BuildToolsDir()) || !DirectoryExists(PlatformsDir()))
		return false;

	// Must contain at least one valid platform with android.jar
	if(FindPlatforms().IsEmpty())
		return false;

	// Must contain at least one valid build-tools release for the host OS
	if(FindBuildToolsReleases().IsEmpty())
		return false;

	return true;
}

bool AndroidSDK::ValidatePlatform() const
{
	if(platform.IsEmpty())
		return false;
	return ValidatePlatform(platform);
}

bool AndroidSDK::ValidatePlatform(const String& platformVersion) const
{
	if(platformVersion.IsEmpty())
		return false;
	String dir = AppendFileName(PlatformsDir(), platformVersion);
	return DirectoryExists(dir) && FileExists(AppendFileName(dir, "android.jar"));
}

bool AndroidSDK::ValidateBuildTools() const
{
	if(build_tools_release.IsEmpty())
		return false;
	return ValidateBuildToolsRelease(build_tools_release);
}

bool AndroidSDK::ValidateBuildToolsRelease(const String& release) const
{
	if(release.IsEmpty())
		return false;
	String dir = AppendFileName(BuildToolsDir(), release);
	if(!DirectoryExists(dir))
		return false;

	// Must contain aapt or aapt2 for host OS
	bool hasAapt = FileExists(AppendFileName(dir, "aapt" + GetExeExt())) ||
	               FileExists(AppendFileName(dir, "aapt2" + GetExeExt()));
	if(!hasAapt)
		return false;

	// Must contain zipalign for host OS
	if(!FileExists(AppendFileName(dir, "zipalign" + GetExeExt())))
		return false;

	return true;
}

Vector<String> AndroidSDK::FindPlatforms() const
{
	Vector<String> platforms;
	
	for(FindFile ff(AppendFileName(PlatformsDir(), "*")); ff; ff.Next()) {
		if(!ff.IsHidden() && ff.IsFolder() && ValidatePlatform(ff.GetName()))
			platforms.Add(ff.GetName());
	}
	
	return platforms;
}

Vector<String> AndroidSDK::FindBuildToolsReleases() const
{
	Vector<String> buildTools;
	
	for(FindFile ff(AppendFileName(BuildToolsDir(), "*")); ff; ff.Next()) {
		if(!ff.IsHidden() && ff.IsFolder() && ValidateBuildToolsRelease(ff.GetName()))
			buildTools.Add(ff.GetName());
	}
	
	return buildTools;
}

Vector<AndroidDevice> AndroidSDK::FindDevices() const
{
	Vector<AndroidDevice> devices;
	
	if(!HasAdb())
		return devices;
	
	String out;
	if(Sys(NormalizeExePath(AdbPath()) + " devices -l", out) == 0) {
		Vector<String> lines;
		Vector<String> outputLines = Split(out, "\n");
		for(int i = 0; i < outputLines.GetCount(); i++) {
			if(!outputLines[i].StartsWith("*") && !outputLines[i].StartsWith(" "))
				lines.Add(outputLines[i]);
		}
		
		for(int i = 1; i < lines.GetCount(); i++) {
			AndroidDevice device;
			
			Vector<String> elements = Split(lines[i], " ");
			for(int j = 0; j < elements.GetCount(); j++) {
				if(j == 0)
					device.SetSerial(elements[j]);
				else if(j == 1)
					device.SetState(elements[j]);
				else {
					Vector<String> element = Split(elements[j], ":");
					if(element.GetCount() == 2) {
						String tag  = element[0];
						String data = element[1];
						if(tag == "usb")
							device.SetUsb(data);
						else
						if(tag == "model")
							device.SetModel(data);
					}
				}
			}
			devices.Add(device);
		}
	}
	
	return devices;
}

Vector<AndroidVirtualDevice> AndroidSDK::FindVirtualDevices() const
{
	Vector<AndroidVirtualDevice> avdes;

	// Check if the deprecated android tool exists (older SDKs)
	String androidToolPath = AndroidPath();
	if (FileExists(androidToolPath)) {
		String out;
		if(Sys(NormalizeExePath(androidToolPath) + " list avd", out) == 0) {
			Vector<String> lines = Split(out, "\n");

			AndroidVirtualDevice avd;
			for(int i = 0; i < lines.GetCount(); i++) {
				Vector<String> line = Split(lines[i], ":");
				if(line.GetCount() == 2) {
					String tag  = line[0];
					String data = line[1];
					if(data.StartsWith(" "))
						data.Remove(0);
					if(tag.Find("Name") > -1) {
						if(!avd.GetName().IsEmpty() && avd.GetName() != data)
							avdes.Add(avd);
						avd.SetName(data);
					}
					else
					if(tag.Find("Device") > -1)
						avd.SetDeviceType(data);
					else
					if(tag.Find("Path") > -1)
						avd.SetPath(data);
					else
					if(tag.Find("Target") > -1)
						avd.SetTarget(data);
					else
					if(tag.Find("Tag/ABI") > -1)
						avd.SetAbi(data);

					// TODO: implement all possible tags
				}
			}

			if(!avd.GetName().IsEmpty())
				avdes.Add(avd);
		}
	}
	// For modern SDKs, we could potentially use avdmanager, but that would require more complex implementation
	// For now, return empty list if the old android tool doesn't exist

	return avdes;
}

String AndroidSDK::FindDefaultPlatform() const
{
	Vector<String> platforms = FindPlatforms();
	if(platforms.GetCount()) {
		Android::NormalizeVersions(platforms);
		Sort(platforms, StdGreater<String>());
		Android::RemoveVersionsNormalization(platforms);

		for(int i = 0; i < platforms.GetCount(); i++) {
			// Tolerate platform folders with suffixes (e.g. "android-34-ext12")
			// or minor versions (e.g. "android-37.0"), not just plain "android-N".
			if(RegExp("^android-[0-9]").Match(platforms[i])) {
				return platforms[i];
			}
		}
		// If no platform matches the expected pattern, return empty string
		return "";
	}
	return "";
}

String AndroidSDK::FindDefaultBuildToolsRelease() const
{
	Vector<String> releases = FindBuildToolsReleases();
	if(releases.GetCount()) {
		Sort(releases, StdGreater<String>());
		for(int i = 0; i < releases.GetCount(); i++) {
			if(RegExp("^[1-9][0-9.]*$").Match(releases[i])) {
				return releases[i];
			}
		}
		// If no build tools match the expected pattern, return empty string
		return "";
	}
	return "";
}

// ---------------------------------------------------------------

String AndroidSDK::BinDir() const
{
	return path + DIR_SEPS + "bin";
}

String AndroidSDK::BuildToolsDir() const
{
	return path + DIR_SEPS + "build-tools";
}

String AndroidSDK::PlatformsDir() const
{
	return path + DIR_SEPS + "platforms";
}

String AndroidSDK::ConcreteBuildToolsDir() const
{
	return BuildToolsDir() + DIR_SEPS + build_tools_release;
}

String AndroidSDK::ConcretePlatformDir() const
{
	return PlatformsDir() + DIR_SEPS + platform;
}

String AndroidSDK::PlatformToolsDir() const
{
	return path + DIR_SEPS + "platform-tools";
}

String AndroidSDK::ToolsDir() const
{
	return path + DIR_SEPS + "tools";
}

String AndroidSDK::NdkDir() const
{
	return path + DIR_SEPS + "ndk";
}

Vector<String> AndroidSDK::FindNdks() const
{
	Vector<String> ndks;
	String ndkRoot = NdkDir();
	if(!DirectoryExists(ndkRoot))
		return ndks;
	
	for(FindFile ff(AppendFileName(ndkRoot, "*")); ff; ff.Next()) {
		if(!ff.IsHidden() && ff.IsFolder()) {
			AndroidNDK ndk(ff.GetPath());
			if(ndk.Validate())
				ndks.Add(ff.GetPath());
		}
	}
	return ndks;
}

String AndroidSDK::FindDefaultNdk() const
{
	Vector<String> ndks = FindNdks();
	if(ndks.IsEmpty())
		return String();
	Sort(ndks, StdGreater<String>());
	return ndks[0];
}

String AndroidSDK::AaptPath() const
{
	String aapt = ConcreteBuildToolsDir() + DIR_SEPS + "aapt" + GetExeExt();
	if(FileExists(aapt))
		return aapt;
	String aapt2 = ConcreteBuildToolsDir() + DIR_SEPS + "aapt2" + GetExeExt();
	if(FileExists(aapt2))
		return aapt2;
	return aapt;
}

String AndroidSDK::AdbPath() const
{
	String p = AppendFileName(PlatformToolsDir(), "adb" + GetExeExt());
	if(FileExists(p))
		return p;

	String onPath = GetFileOnPath("adb" + GetExeExt(), GetEnv("PATH"));
	if(!onPath.IsEmpty())
		return onPath;

	return p;
}

bool AndroidSDK::HasAdb() const
{
	return FileExists(AdbPath());
}

// ---------------------------------------------------------------

bool AndroidSDK::HasD8() const
{
	return FileExists(D8Path());
}

}
