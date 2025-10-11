#include <iostream>
#include <string>
#include <cstdlib>  // for system()

using namespace std;

// functionality to cancel download
// functionality to make it cross platform 
// functionality to download area2c if not downloaded for both ubuntu and windows

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
        downloadPath = "/home/subhan/Downloads";  // default directory
    }

    // Step 3: Build aria2c command
    string command = "aria2c --dir=\"" + downloadPath +
                 "\" --bt-max-peers=50 --continue=true \"" +
                 magnetLink + "\""; // download path is also inside double quotes!

    // Step 4: Execute download command
    cout << "\nStarting download using aria2c...\n";
    system(command.c_str());

    return 0;
}


