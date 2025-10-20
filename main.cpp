#include <iostream>
#include <string>
#include <cstdlib>
#include <fstream>

using namespace std;

string expandPath(const string& path) {
    const char* home = getenv("HOME");
    if (!home) return path;
    if (path.rfind("~", 0) == 0) {
        return string(home) + path.substr(1);
    }
    return path;
}

bool isAria2Installed()
{
    int status = system("aria2c --version > /dev/null 2>&1");
    return (status == 0);
}

void safePause() {}

void installAria2()
{
    cout << "Installing aria2 via apt (you may be prompted for your sudo password)...\n";
    int status = system("sudo apt update && sudo apt install -y aria2");
    if (status != 0) {
        cout << "Error: apt install failed (code " << status << "). Please install aria2 manually (sudo apt install aria2).\n";
        cout << "Press Enter to exit...";
        cin.get();
    } else {
        cout << "aria2 installed successfully.\n";
    }
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
    string mkDirCmd = "mkdir -p \"" + finalPath + "\"";
    system(mkDirCmd.c_str());

    if (!isAria2Installed())
    {
        cout << "Aria2 is not installed. Installing...\n";
        installAria2();
    }

    // build command
    string command;
    string baseCmd = "aria2c";
    command = "aria2c --dir=\"" + finalPath +
              "\" --bt-max-peers=50 --continue=true \"" +
              magnetLink + "\"";

    cout << "\nTesting aria2c before download...\n";
    string testCmd = "\"" + baseCmd + "\" --version";
    int testStatus = system(testCmd.c_str());
    if (testStatus != 0) {
        cout << "ERROR: aria2c test failed with code " << testStatus << "\n";
        cout << "This means aria2c is not working properly.\n";
        safePause();
        return 1;
    }
    cout << "aria2c test passed!\n";

    cout << "\nStarting download using aria2c...\n";
    cout << "Command: " << command << "\n";
    cout << "\nDebugging info:\n";
    cout << "- Aria2c path: " << baseCmd << "\n";
    cout << "- Download directory: " << finalPath << "\n";
    cout << "- Magnet link length: " << magnetLink.length() << " characters\n";
    cout << "\nIf download gets stuck, try:\n";
    cout << "1. Check firewall rules (ufw/iptables)\n";
    cout << "2. Try a different magnet link\n\n";
    
    int runStatus = system(command.c_str());
    if (runStatus != 0) {
        cout << "\naria2c exited with code " << runStatus << ".\n";
        cout << "This could mean:\n";
        cout << "- No seeders available for this torrent\n";
        cout << "- Network connectivity issues\n";
        cout << "- Firewall blocking the connection\n";
        cout << "- Try a different magnet link or check your internet connection\n";
    } else {
        cout << "\nDownload completed successfully!\n";
    }
    
    // Linux-only: no temp artifacts to clean

    return 0;
}
