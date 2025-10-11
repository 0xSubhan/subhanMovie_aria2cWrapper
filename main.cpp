#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>

using namespace std;

// functionality to cancel download
// functionality to make it cross platform 
// functionality to download area2c if not downloaded for both ubuntu and windows

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

int main() {
    string magnetLink;
    string downloadPath;

    cout << "===== Simple Torrent Downloader (MVP) =====" << endl;

    // Step 1: Get magnet link from user
    cout << "Enter Magnet Link: ";
    getline(cin, magnetLink);

    if (magnetLink.empty()) {
        cout << "Error: Magnet link cannot be empty!" << endl;
        return 1;
    }

    // Step 2: Get download path
    cout << "Enter download path (Default: ~/Downloads): ";
    getline(cin, downloadPath);

    if (downloadPath.empty()) {
        downloadPath = "~/Downloads";  // default directory
    }

    // handling case where : ~ is not automatically understood by c/c++ so it expands the path
    string finalPath = expandPath(downloadPath);
    cout << "\nDownloading to: " << finalPath << endl;

#ifdef _WIN32
    string command = "aria2c.exe --dir=\"" + finalPath +
#else
    string command = "aria2c --dir=\"" + finalPath +
#endif
        "\" --bt-max-peers=50 --continue=true \"" + magnetLink + "\"";

    cout << "\nStarting download using aria2c...\n";
    system(command.c_str());

    return 0;
}


