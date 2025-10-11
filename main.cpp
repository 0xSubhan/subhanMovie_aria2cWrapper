#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>

using namespace std;

// functionality to cancel download
// functionality to make it cross platform 

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
// checking if area2c is installed or not!
bool isAria2Installed()
{
#ifdef _WIN32
    int status = system("where aria2c >nul 2>&1"); // Without redirecting output, it will print errors in terminal.
    if (status == 0)
        return true;
    else
        return false;    
    
#else
    int status = system("aria2c --version > /dev/null 2>&1"); // Without redirecting output, it will print errors in terminal.
    if (status == 0)
        return true;
    else
        return false;            
        
#endif
}
// this function will be called if area2c is not installed:
void installAria2()
{
#ifdef _WIN32
    cout << "Aria2 is not installed. Installing for current user...\n";

    // 1️⃣ Define user folder installation path
    string installDir = string(getenv("USERPROFILE")) + "\\Aria2";

    // 2️⃣ Download latest GitHub ZIP
    system(("powershell -Command \""
            "Invoke-WebRequest -Uri https://github.com/aria2/aria2/releases/latest/download/aria2-1.37.0-win-64bit-build1.zip "
            "-OutFile %USERPROFILE%\\aria2.zip\"").c_str());

    // 3️⃣ Extract ZIP to user folder
    system(("powershell -Command \""
            "Expand-Archive -Path %USERPROFILE%\\aria2.zip "
            "-DestinationPath " + installDir + " -Force\"").c_str());

    // 4️⃣ Add folder to PATH (user-level)
    system(("setx PATH \"%PATH%;" + installDir + "\"").c_str());

    cout << "✅ Aria2 installed successfully at " << installDir << " and added to PATH.\n";

#else
    system("sudo apt update && sudo apt install -y aria2");

#endif
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

    // checking if aria2c exist?
    if (!isAria2Installed())
    {
        cout << "Aria2 is not installed. Installing...\n";
        installAria2();
    }
    

#ifdef _WIN32
    // string command = "aria2c.exe --dir=\"" + finalPath +
    string aria2Path = expandPath("\\Aria2\\aria2c.exe");
    string command = "\"" + aria2Path + "\" --dir=\"" + finalPath + 
    // After installation, aria2c.exe is in PATH, but sometimes the PATH update requires a new terminal session.
    // Consider using full path to the executable immediately after extraction
    //This ensures your program runs aria2c right after installation, without waiting for PATH to refresh.  

#else
    string command = "aria2c --dir=\"" + finalPath +
#endif
        "\" --bt-max-peers=50 --continue=true \"" + magnetLink + "\"";

    cout << "\nStarting download using aria2c...\n";
    system(command.c_str());

    return 0;
}


