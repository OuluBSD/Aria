#include "WebDriver.h"
#include "Undetected.h"

NAMESPACE_UPP

namespace detail {

String GetFirefoxInstallationPath() {
#ifdef PLATFORM_POSIX
	static const char* paths[] = {
		"/usr/lib/firefox",
		"/usr/lib64/firefox",
		"/usr/lib/firefox-esr",
		"/usr/lib/firefox-developer-edition",
		"/usr/lib/firefox-nightly",
		"/opt/firefox",
		"/usr/local/lib/firefox",
		"/usr/local/lib64/firefox",
		"/opt/firefox-bin",
	};
	for (const char* p : paths) {
		RLOG("WebDriver: Checking path: " << p);
		if (DirectoryExists(p) && FileExists(AppendFileName(p, "libxul.so"))) {
			RLOG("WebDriver: Found Firefox at: " << p);
			return p;
		}
	}
	
	// Try to find via binary path
	String out = TrimBoth(Sys("which firefox"));
	RLOG("WebDriver: 'which firefox' returned: " << out);
	if (!out.IsEmpty()) {
		String real_path = out;
		// Follow symlinks to find real installation
		for (int i = 0; i < 10; i++) {
			String link = GetSymLinkPath(real_path);
			if (link.IsEmpty()) break;
			RLOG("WebDriver: Following symlink: " << real_path << " -> " << link);
			real_path = NormalizePath(link, GetFileFolder(real_path));
		}
		String dir = GetFileFolder(real_path);
		RLOG("WebDriver: Checking dir of binary: " << dir);
		if (FileExists(AppendFileName(dir, "libxul.so"))) return dir;
		
		// Some distros have 'firefox' as a script in /usr/bin, but libs in /usr/lib/firefox
		dir = "/usr/lib/firefox";
		if (FileExists(AppendFileName(dir, "libxul.so"))) return dir;
	}
#endif
	RLOG("WebDriver: Firefox installation path NOT FOUND");
	return "";
}

String GetUndetectedFirefoxPath() {
	String home_bin = GetHomeDirFile("bin");
	String path_env = GetEnv("PATH");
	if (DirectoryExists(home_bin) && path_env.Find(home_bin) >= 0) {
		return AppendFileName(home_bin, "undetected_firefox");
	}
	return GetHomeDirFile(AppendFileName(".cache", "undetected_firefox"));
}

static String ReadIniValue(const String& ini_path, const char* section, const char* key) {
	if (!FileExists(ini_path)) return "";
	String data = LoadFile(ini_path);
	Vector<String> lines = Split(data, '\n');
	bool in_section = false;
	for (const String& raw : lines) {
		String line = TrimBoth(raw);
		if (line.StartsWith("[") && line.EndsWith("]")) {
			in_section = line.Mid(1, line.GetLength() - 2) == section;
			continue;
		}
		if (!in_section) continue;
		int eq = line.Find("=");
		if (eq < 0) continue;
		if (TrimBoth(line.Left(eq)) == key) return TrimBoth(line.Mid(eq + 1));
	}
	return "";
}

//Version of the Firefox build living in 'dir', empty when the directory holds no recognizable build.
static String FirefoxVersion(const String& dir) {
	return ReadIniValue(AppendFileName(dir, "application.ini"), "App", "Version");
}

static String FirefoxBuildId(const String& dir) {
	return ReadIniValue(AppendFileName(dir, "application.ini"), "App", "BuildID");
}

static bool IsAllDigits(const String& s) {
	if (s.IsEmpty()) return false;
	for (int i = 0; i < s.GetLength(); i++)
		if (!IsDigit(s[i])) return false;
	return true;
}

//Compares dotted versions or all-digit build identifiers; -1, 0 or 1.
static int CompareBuildIdentifiers(const String& a, const String& b) {
	if (IsAllDigits(a) && IsAllDigits(b)) {
		int64 va = Atoi64(a), vb = Atoi64(b);
		return va < vb ? -1 : va > vb ? 1 : 0;
	}
	Vector<String> pa = Split(a, '.'), pb = Split(b, '.');
	int count = pa.GetCount() > pb.GetCount() ? pa.GetCount() : pb.GetCount();
	for (int i = 0; i < count; i++) {
		String x = i < pa.GetCount() ? pa[i] : "0";
		String y = i < pb.GetCount() ? pb[i] : "0";
		if (IsAllDigits(x) && IsAllDigits(y)) {
			int64 vx = Atoi64(x), vy = Atoi64(y);
			if (vx != vy) return vx < vy ? -1 : 1;
		}
		else if (x != y)
			return x < y ? -1 : 1;
	}
	return 0;
}

//Physical path of a directory, so paths compared here agree with the /proc/<pid>/exe targets.
static String ResolveSymLinkedDir(const String& dir) {
	String resolved = dir;
	for (int i = 0; i < 16; i++) {
		String link = GetSymLinkPath(resolved);
		if (link.IsEmpty()) break;
		resolved = NormalizePath(link, GetFileFolder(resolved));
	}
	return resolved;
}

static bool IsFirefoxRunningFrom(const String& dir) {
#ifdef PLATFORM_POSIX
	String target = ResolveSymLinkedDir(dir);
	for (FindFile ff("/proc/*"); ff; ++ff) {
		String pid = ff.GetName();
		if (!IsAllDigits(pid)) continue;
		String exe = GetSymLinkPath(AppendFileName("/proc", AppendFileName(pid, "exe")));
		if (exe.IsEmpty()) continue;
		if (PathIsEqual(ResolveSymLinkedDir(GetFileFolder(exe)), target)) return true;
	}
#endif
	return false;
}

//Copies 'src' into 'dst' so that the result contains exactly the files of 'src'.
//Builds into a sibling directory and swaps it in, keeping the previous tree until
//the swap succeeds, so a failed move cannot leave the copy destroyed.
static bool SyncFirefoxTree(const String& src, const String& dst) {
	String tmp = dst + ".sync";
	String prev = dst + ".prev";
	DeleteFolderDeep(tmp);
	DeleteFolderDeep(prev);
	if (!RealizeDirectory(tmp)) {
		RLOG("WebDriver: Could not create staging directory " << tmp);
		return false;
	}
	//'src/.' rather than 'src/*' so dotfiles are copied too.
	String cp_cmd = "cp -r '" + src + "/.' '" + tmp + "/'";
	RLOG("WebDriver: Executing " << cp_cmd);
	if (system(~cp_cmd) != 0) {
		RLOG("WebDriver: Copy failed, aborting undetected Firefox sync");
		DeleteFolderDeep(tmp);
		return false;
	}
	bool had_dst = DirectoryExists(dst);
	if (had_dst) {
		String aside_cmd = "mv '" + dst + "' '" + prev + "'";
		if (system(~aside_cmd) != 0) {
			RLOG("WebDriver: Could not move the old tree aside, aborting undetected Firefox sync");
			DeleteFolderDeep(tmp);
			return false;
		}
	}
	String mv_cmd = "mv '" + tmp + "' '" + dst + "'";
	RLOG("WebDriver: Executing " << mv_cmd);
	if (system(~mv_cmd) != 0) {
		RLOG("WebDriver: Swap failed, aborting undetected Firefox sync");
		DeleteFolderDeep(tmp);
		if (had_dst && system(~("mv '" + prev + "' '" + dst + "'")) != 0)
			RLOG("WebDriver: Error: could not restore the previous tree at " << dst);
		else
			DeleteFolderDeep(prev);
		return false;
	}
	DeleteFolderDeep(prev);
	return true;
}

static void ApplyStealthPatches(const String& dst, const String& dst_exe, const String& xul_name) {
#ifdef PLATFORM_POSIX
	//The stock launcher has an absolute RPATH pointing at the system installation, so without this
	//the copied binary would load the unpatched /usr libxul instead of the patched one next to it.
	String patchelf_cmd = "patchelf --set-rpath '$ORIGIN' '" + dst_exe + "'";
	RLOG("WebDriver: Executing " << patchelf_cmd);
	if (system(~patchelf_cmd) != 0)
		RLOG("WebDriver: Warning: patchelf failed, the copied launcher may load the system libxul");
#endif
	String dst_xul = AppendFileName(dst, xul_name);
	if (!FileExists(dst_xul))
		dst_xul = AppendFileName(AppendFileName(dst, "browser"), xul_name);
	if (!FileExists(dst_xul)) {
		RLOG("WebDriver: Warning: " << xul_name << " missing from " << dst);
		return;
	}
	String data = LoadFile(dst_xul);
	const char* target = "webdriver";
	int len = 9;
	int pos = data.Find(target);
	if (pos < 0) {
		RLOG("WebDriver: " << dst_xul << " does not contain '" << target << "', nothing to patch");
		return;
	}
	String replacement;
	for (int i = 0; i < len; i++)
		replacement.Cat('a' + Random(26));
	RLOG("WebDriver: Patching " << target << " with " << replacement << " in " << dst_xul);
	while (pos >= 0) {
		for (int i = 0; i < len; i++)
			data.Set(pos + i, replacement[i]);
		pos = data.Find(target, pos + len);
	}
	SaveFile(dst_xul, data);
}

String PatchFirefoxBinary() {
	String src = GetFirefoxInstallationPath();
	String dst = GetUndetectedFirefoxPath();
	
	RLOG("WebDriver: PatchFirefoxBinary src=" << src << " dst=" << dst);
	
	if (src.IsEmpty()) {
		RLOG("WebDriver: Warning: Could not find Firefox installation path. Binary patching skipped.");
		return "";
	}
	
	String xul_name = "libxul.so";
#ifdef PLATFORM_WIN32
	xul_name = "xul.dll";
#endif

	String src_xul = AppendFileName(src, xul_name);
	
	if (!FileExists(src_xul)) {
		src_xul = AppendFileName(AppendFileName(src, "browser"), xul_name);
		if (!FileExists(src_xul)) {
			RLOG("WebDriver: Warning: Could not find " + xul_name + " in " + src);
			return "";
		}
	}

	String exe_name = "firefox";
#ifdef PLATFORM_WIN32
	exe_name = "firefox.exe";
#endif
	String dst_exe = AppendFileName(dst, exe_name);
	
	if (PathIsEqual(ResolveSymLinkedDir(src), ResolveSymLinkedDir(dst))) {
		RLOG("WebDriver: Warning: Undetected Firefox path resolves to the system installation, refusing to overwrite " << src);
		return "";
	}
	
	String src_version = FirefoxVersion(src);
	String dst_version = FirefoxVersion(dst);
	RLOG("WebDriver: System Firefox " << src_version << " build " << FirefoxBuildId(src)
		<< ", undetected Firefox " << (dst_version.IsEmpty() ? String("(absent)") : dst_version + " build " + FirefoxBuildId(dst)));
	
	//Firefox refuses to attach a profile whose compatibility.ini records a newer LastVersion, so the
	//undetected copy must never fall behind the installation the user profile was last written by.
	bool sync_needed = !FileExists(dst_exe);
	if (!sync_needed) {
		int version_cmp = dst_version.IsEmpty() ? -1 : CompareBuildIdentifiers(src_version, dst_version);
		if (version_cmp > 0)
			sync_needed = true;
		else if (version_cmp == 0 && CompareBuildIdentifiers(FirefoxBuildId(src), FirefoxBuildId(dst)) > 0)
			sync_needed = true;
		else if (version_cmp < 0)
			RLOG("WebDriver: Warning: Undetected Firefox " << dst_version
				<< " is newer than system Firefox " << src_version << ", keeping the newer copy to avoid a profile downgrade");
	}
	
	if (sync_needed) {
		if (IsFirefoxRunningFrom(dst)) {
			RLOG("WebDriver: Warning: Undetected Firefox " << dst_version << " is older than system Firefox " << src_version
				<< " and is still running. Close it (bin/Aria close) and retry to refresh it.");
		} else {
			RLOG("WebDriver: Syncing undetected Firefox in " << dst << " from " << src);
			if (SyncFirefoxTree(src, dst)) {
				// If only firefox-bin exists, copy it as firefox
				if (!FileExists(AppendFileName(dst, "firefox")) && FileExists(AppendFileName(dst, "firefox-bin"))) {
					RLOG("WebDriver: Copying firefox-bin to firefox");
					SaveFile(AppendFileName(dst, "firefox"), LoadFile(AppendFileName(dst, "firefox-bin")));
				}
#ifdef PLATFORM_POSIX
				chmod(AppendFileName(dst, "firefox"), 0755);
				if (FileExists(AppendFileName(dst, "firefox-bin")))
					chmod(AppendFileName(dst, "firefox-bin"), 0755);
#endif
				ApplyStealthPatches(dst, dst_exe, xul_name);
				RLOG("WebDriver: Undetected Firefox refreshed to " << FirefoxVersion(dst) << " build " << FirefoxBuildId(dst));
			} else {
				RLOG("WebDriver: Error: Failed to refresh undetected Firefox, falling back to existing copy");
			}
		}
	} else {
		RLOG("WebDriver: Undetected Firefox is up to date at " << dst_exe);
	}
	
	RLOG("WebDriver: Returning patched binary path: " << dst_exe);
	return dst_exe;
}

String GetFirefoxDefaultProfilePath() {
	Vector<String> ini_paths;
#ifdef PLATFORM_POSIX
	String xdg_config = GetEnv("XDG_CONFIG_HOME");
	if (xdg_config.IsEmpty())
		xdg_config = GetHomeDirFile(".config");
	ini_paths.Add(AppendFileName(xdg_config, "mozilla/firefox/profiles.ini"));
	ini_paths.Add(GetHomeDirFile(".mozilla/firefox/profiles.ini"));
#elif defined(PLATFORM_WIN32)
	ini_paths.Add(AppendFileName(GetAppDataFolder(), "Mozilla/Firefox/profiles.ini"));
#endif

	String ini_path;
	for (const String& path : ini_paths) {
		if (FileExists(path)) {
			ini_path = path;
			break;
		}
	}
	if (ini_path.IsEmpty()) return "";

	String data = LoadFile(ini_path);
	Vector<String> sections = Split(data, '[');
	
	String default_profile_path;
	String release_profile_path;
	String install_default_path;
	
	for (const String& section_content : sections) {
		Vector<String> lines = Split(section_content, '\n');
		if (lines.IsEmpty()) continue;
		
		String header = lines[0];
		bool is_profile = header.StartsWith("Profile");
		bool is_install = header.StartsWith("Install");
		
		if (!is_profile && !is_install) continue;

		String p_path;
		String p_name;
		bool is_relative = true;
		bool is_p_default = false;

		for (int i = 1; i < lines.GetCount(); i++) {
			String line = TrimBoth(lines[i]);
			if (line.StartsWith("Path=")) p_path = line.Mid(5);
			if (line.StartsWith("Name=")) p_name = line.Mid(5);
			if (line.StartsWith("Default=1")) is_p_default = true;
			if (line.StartsWith("IsRelative=0")) is_relative = false;
			if (is_install && line.StartsWith("Default=")) p_path = line.Mid(8);
		}
		
		if (p_path.IsEmpty()) continue;
		
		String full_path = is_relative ? AppendFileName(GetFileFolder(ini_path), p_path) : p_path;
		
		if (is_install) {
			install_default_path = full_path;
		} else if (is_profile) {
			// Prioritize "default-release" explicitly
			if (p_name == "default-release") release_profile_path = full_path;
			// Fallback to Default=1
			if (is_p_default) default_profile_path = full_path;
		}
	}
	
	//profiles.ini v2 records the per-install default in [Install*] sections; that is
	//the profile Firefox itself opens. A profile merely *named* "default-release" can
	//be an old, nearly empty leftover, so it must not outrank the Install default.
	if (!install_default_path.IsEmpty()) {
		RLOG("WebDriver: Selected install-specific default profile: " << install_default_path);
		return install_default_path;
	}

	if (!release_profile_path.IsEmpty()) {
		RLOG("WebDriver: Selected default-release profile: " << release_profile_path);
		return release_profile_path;
	}

	if (!default_profile_path.IsEmpty()) {
		RLOG("WebDriver: Selected default profile: " << default_profile_path);
		return default_profile_path;
	}
	
	return "";
}

} // namespace detail

END_UPP_NAMESPACE
