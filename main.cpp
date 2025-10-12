#include <iostream>
#include <string>
#include <cstdlib>

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

void installAria2()
{
#ifdef _WIN32
    cout << "Aria2 is not installed. Installing for current user...\n";

    std::string installDir = std::string(getenv("USERPROFILE")) + "\\Aria2";

    // 1️⃣ Download Aria2 ZIP
    std::string downloadCmd =
        "powershell -NoProfile -ExecutionPolicy Bypass -Command "
        "\"Invoke-WebRequest -Uri https://github.com/aria2/aria2/releases/latest/download/aria2-1.37.0-win-64bit-build1.zip "
        "-OutFile $env:USERPROFILE\\aria2.zip\"";

    // 2️⃣ Extract ZIP to custom folder
    std::string extractCmd =
        "powershell -NoProfile -ExecutionPolicy Bypass -Command "
        "\"Expand-Archive -Path $env:USERPROFILE\\aria2.zip -DestinationPath '" + installDir + "' -Force\"";

    // 3️⃣ Add to USER PATH permanently (avoids needing Admin rights)
    std::string setPathCmd =
        "powershell -NoProfile -ExecutionPolicy Bypass -Command "
        "\"$p = [Environment]::GetEnvironmentVariable('PATH','User'); "
        "if (-not ($p -like '*" + installDir + "*')) { "
        "[Environment]::SetEnvironmentVariable('PATH', $p + ';"+ installDir +"', 'User'); "
        "Write-Host 'User PATH updated.' } else { Write-Host 'PATH already contains Aria2.' }\"";

    system(downloadCmd.c_str());
    system(extractCmd.c_str());
    system(setPathCmd.c_str());

    // 4️⃣ Delete ZIP
    std::string cleanupCmd = "del \"%USERPROFILE%\\aria2.zip\" >nul 2>&1";
    system(cleanupCmd.c_str());

    cout << "✅ Aria2 installed successfully at: " << installDir << "\n";
    cout << "ℹ️ Restart terminal to update PATH or use full path immediately.\n";

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
    // Use full path to aria2c.exe in the user install folder first (guarantees first-run)
    string aria2Path = string(getenv("USERPROFILE")) + "\\Aria2\\aria2c.exe";
    // if aria2 still not present at that path, fallback to calling aria2c (PATH)
    // Build accurate command string
    if (std::system((string("if exist \"") + aria2Path + "\" (echo 1) >nul 2>&1").c_str()) == 0) {
        command = "\"" + aria2Path + "\" --dir=\"" + finalPath + "\" --bt-max-peers=50 --continue=true \"" + magnetLink + "\"";
    } else {
        command = "aria2c --dir=\"" + finalPath + "\" --bt-max-peers=50 --continue=true \"" + magnetLink + "\"";
    }
#else
    command = "aria2c --dir=\"" + finalPath + "\" --bt-max-peers=50 --continue=true \"" + magnetLink + "\"";
#endif

    cout << "\nStarting download using aria2c...\n";
    cout << "Command: " << command << "\n";
    int runStatus = system(command.c_str());
    if (runStatus != 0) {
        cout << "aria2c exited with code " << runStatus << ".\n";
    }

    system("pause");
    return 0;
}
