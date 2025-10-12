#include <iostream>
#include <string>
#include <cstdlib>
#include <fstream>

using namespace std;

string expandPath(const string& path) {
#ifdef _WIN32
    const char* home = getenv("USERPROFILE");
#else
    const char* home = getenv("HOME");
#endif
    if (!home) return path;
    if (path.rfind("~", 0) == 0) {
        return string(home) + path.substr(1);
    }
    return path;
}

bool isAria2Installed()
{
#ifdef _WIN32
    int status = system("where aria2c >nul 2>&1");
    return (status == 0);
#else
    int status = system("aria2c --version > /dev/null 2>&1");
    return (status == 0);
#endif
}

void safePause() {
#ifdef _WIN32
    cout << "Press Enter to continue...";
    cin.get();
#endif
}

void installAria2()
{
#ifdef _WIN32
    cout << "Aria2 is not installed. Installing for current user...\n";

    string userProfile = getenv("USERPROFILE");
    string installDir = userProfile + "\\Aria2";

    // Download ZIP (PowerShell)
    cout << "Downloading aria2 package...\n";
    string downloadCmd =
        "powershell -NoProfile -ExecutionPolicy Bypass -Command \""
        "try { Invoke-WebRequest -Uri 'https://github.com/aria2/aria2/releases/latest/download/aria2-1.37.0-win-64bit-build1.zip' -OutFile $env:USERPROFILE\\\\aria2.zip -UseBasicParsing; Write-Host 'DL_OK' } "
        "catch { Write-Host 'DL_FAIL'; exit 1 }\"";
    int r = system(downloadCmd.c_str());
    if (r != 0) {
        cout << "Error: Download failed (code " << r << "). Check your internet connection or install aria2 manually.\n";
        safePause();
        return;
    }

    // Extract into installDir (PowerShell)
    cout << "Extracting package...\n";
    // Create installDir first (PowerShell will create it, but ensure)
    string makeDirCmd = "powershell -NoProfile -ExecutionPolicy Bypass -Command \"New-Item -ItemType Directory -Force -Path '" + installDir + "' > $null\"";
    system(makeDirCmd.c_str());

    string extractCmd =
        "powershell -NoProfile -ExecutionPolicy Bypass -Command \""
        "try { Expand-Archive -Path $env:USERPROFILE\\\\aria2.zip -DestinationPath '" + installDir + "' -Force; Write-Host 'EX_OK' } "
        "catch { Write-Host 'EX_FAIL'; exit 2 }\"";
    r = system(extractCmd.c_str());
    if (r != 0) {
        cout << "Error: Extraction failed (code " << r << "). You may need a newer PowerShell or extract manually.\n";
        safePause();
        return;
    }

    // Locate aria2c.exe inside installDir (recursive search) and write to temp file
    cout << "Locating aria2c.exe...\n";
    string findCmd =
        "powershell -NoProfile -ExecutionPolicy Bypass -Command \""
        "try { $f = Get-ChildItem -Path '" + installDir + "' -Filter aria2c.exe -Recurse -File -ErrorAction SilentlyContinue | Select-Object -First 1 -ExpandProperty FullName; "
        "if ($null -ne $f) { $f | Out-File -FilePath $env:USERPROFILE\\\\aria2_path.txt -Encoding UTF8; Write-Host 'FOUND' } else { Write-Host 'NOTFOUND'; exit 3 } } "
        "catch { Write-Host 'FIND_ERR'; exit 4 }\"";
    r = system(findCmd.c_str());
    if (r != 0) {
        cout << "Error: Could not locate aria2c.exe inside the extracted package (code " << r << ").\n";
        cout << "Please check the contents of " << installDir << " and place aria2c.exe there.\n";
        safePause();
        return;
    }

    // Read the found path from file
    string aria2PathFile = userProfile + "\\aria2_path.txt";
    ifstream in(aria2PathFile.c_str());
    string aria2FullPath;
    if (in.good()) {
        getline(in, aria2FullPath);
        in.close();
    }
    // remove temp file
    string delPathFileCmd = ("del \"" + aria2PathFile + "\" >nul 2>&1");
    system(delPathFileCmd.c_str());

    if (aria2FullPath.empty()) {
        // Fallback: try expected location
        aria2FullPath = installDir + "\\aria2c.exe";
        cout << "Warning: couldn't read found path; falling back to: " << aria2FullPath << "\n";
    } else {
        cout << "Found aria2 at: " << aria2FullPath << "\n";
    }

    // Optionally, update user PATH (best-effort)
    cout << "Updating user PATH (best-effort)...\n";
    string setPathCmd =
        "powershell -NoProfile -ExecutionPolicy Bypass -Command \""
        "try { $p = [Environment]::GetEnvironmentVariable('PATH','User'); "
        "if (-not ($p -like '*" + installDir + "*')) { [Environment]::SetEnvironmentVariable('PATH', $p + ';" + installDir + "', 'User'); Write-Host 'PATH_UPDATED' } else { Write-Host 'PATH_OK' } } "
        "catch { Write-Host 'PATH_ERR'; exit 5 }\"";
    r = system(setPathCmd.c_str());
    if (r != 0) {
        cout << "Warning: failed to update user PATH automatically (code " << r << ").\n";
    } else {
        cout << "User PATH updated (or already contained the folder).\n";
    }

    // Clean up zip
    string cleanupCmd = "del \"%USERPROFILE%\\aria2.zip\" >nul 2>&1";
    system(cleanupCmd.c_str());

    cout << "Aria2 installed to: " << installDir << "\n";
    cout << "Will use " << aria2FullPath << " for immediate execution.\n";
    cout << "If aria2 fails to run, try restarting the terminal or running aria2 manually.\n";
    // Wait for user confirmation before proceeding
    safePause();

#else
    cout << "Installing aria2 via apt (you may be prompted for your sudo password)...\n";
    int status = system("sudo apt update && sudo apt install -y aria2");
    if (status != 0) {
        cout << "Error: apt install failed (code " << status << "). Please install aria2 manually (sudo apt install aria2).\n";
        cout << "Press Enter to exit...";
        cin.get();
    } else {
        cout << "aria2 installed successfully.\n";
    }
#endif
}

int main() {
    string magnetLink;
    string downloadPath;

    cout << "===== Simple Torrent Downloader (MVP) =====" << endl;

    cout << "Enter Magnet Link: ";
    getline(cin, magnetLink);
    if (magnetLink.empty()) {
        cout << "Error: Magnet link cannot be empty!\n";
        return 1;
    }

    cout << "Enter download path (Default: ~/Downloads): ";
    getline(cin, downloadPath);
    if (downloadPath.empty()) {
        downloadPath = "~/Downloads";
    }

    string finalPath = expandPath(downloadPath);
    cout << "\nDownloading to: " << finalPath << endl;

    // ensure download directory exists
#ifdef _WIN32
    string mkDirCmd = "if not exist \"" + finalPath + "\" mkdir \"" + finalPath + "\"";
    system(mkDirCmd.c_str());
#else
    string mkDirCmd = "mkdir -p \"" + finalPath + "\"";
    system(mkDirCmd.c_str());
#endif

    if (!isAria2Installed())
    {
        cout << "Aria2 is not installed. Installing...\n";
        installAria2();
    }

    // build command
    string command;
#ifdef _WIN32
    // Prefer found aria2 in user folder if present
    string userProfile = getenv("USERPROFILE");
    string defaultAria2 = userProfile + "\\Aria2\\aria2c.exe";

    // If a local aria2 exists in that path, use it; else rely on aria2c in PATH
    bool localExists = (std::system((string("if exist \"") + defaultAria2 + "\" (exit 0) else (exit 1)").c_str()) == 0);

    string baseCmd = localExists ? ("\"" + defaultAria2 + "\"") : "aria2c";

    // Use cmd wrapper so & inside magnet link doesn't break command parsing
    command = "cmd /c \"\"" + baseCmd +
              "\" --dir=\"" + finalPath +
              "\" --bt-max-peers=50 --continue=true \"" +
              magnetLink + "\"\"\"";
#else
    command = "aria2c --dir=\"" + finalPath +
              "\" --bt-max-peers=50 --continue=true \"" +
              magnetLink + "\"";
#endif

    cout << "\nStarting download using aria2c...\n";
    cout << "Command: " << command << "\n";
    int runStatus = system(command.c_str());
    if (runStatus != 0) {
        cout << "aria2c exited with code " << runStatus << ".\n";
    }

#ifdef _WIN32
    cout << "Press Enter to exit...";
    cin.get();
#endif

    return 0;
}
