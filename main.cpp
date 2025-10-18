#include <iostream>
#include <string>
#include <cstdlib>
#include <fstream>
#include <sstream>

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
    // Check if aria2c is in PATH
    int status = system("where aria2c >nul 2>&1");
    if (status == 0) return true;
    
    // Also check if it's in the user's Aria2 folder
    const char* userProfile = getenv("USERPROFILE");
    if (userProfile) {
        string aria2Path = string(userProfile) + "\\Aria2\\aria2-1.37.0-win-64bit-build1\\aria2c.exe";
        string checkCmd = "if exist \"" + aria2Path + "\" (exit 0) else (exit 1)";
        status = system(checkCmd.c_str());
        return (status == 0);
    }
    return false;
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
        
        // Remove BOM if present (UTF-8 BOM is EF BB BF)
        if (aria2FullPath.length() >= 3 && 
            (unsigned char)aria2FullPath[0] == 0xEF && 
            (unsigned char)aria2FullPath[1] == 0xBB && 
            (unsigned char)aria2FullPath[2] == 0xBF) {
            aria2FullPath = aria2FullPath.substr(3);
        }
        
        // Remove any leading/trailing whitespace
        aria2FullPath.erase(0, aria2FullPath.find_first_not_of(" \t\r\n"));
        aria2FullPath.erase(aria2FullPath.find_last_not_of(" \t\r\n") + 1);
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
    string baseCmd;
#ifdef _WIN32
    // Prefer found aria2 in user folder if present
    string userProfile = getenv("USERPROFILE");
    string defaultAria2 = userProfile + "\\Aria2\\aria2c.exe";
    string aria2SubDir = userProfile + "\\Aria2\\aria2-1.37.0-win-64bit-build1\\aria2c.exe";

    // Check multiple possible locations for aria2c.exe
    bool localExists = (std::system((string("if exist \"") + defaultAria2 + "\" (exit 0) else (exit 1)").c_str()) == 0);
    bool subDirExists = (std::system((string("if exist \"") + aria2SubDir + "\" (exit 0) else (exit 1)").c_str()) == 0);
    
    if (localExists) {
        baseCmd = defaultAria2;
    } else if (subDirExists) {
        baseCmd = aria2SubDir;
    } else {
        baseCmd = "aria2c";
    }

    // Create a batch file to handle the command properly
    string batchFile = userProfile + "\\aria2_temp.bat";
    ofstream batchOut(batchFile.c_str());
    if (batchOut.good()) {
        batchOut << "@echo off\n";
        batchOut << "echo Starting aria2c with Windows-optimized settings...\n";
        batchOut << "\"" << baseCmd << "\" --dir=\"" << finalPath 
                 << "\" --bt-max-peers=50 --continue=true"
                 << " --enable-dht=true --enable-dht6=true --enable-peer-exchange=true"
                 << " --bt-save-metadata=true --bt-metadata-only=false"
                 << " --bt-tracker-timeout=30 --bt-tracker-interval=30"
                 << " --bt-request-timeout=60 --bt-stop-timeout=300"
                 << " --dht-entry-point=router.bittorrent.com:6881"
                 << " --dht-entry-point=router.utorrent.com:6881"
                 << " --dht-entry-point=dht.transmissionbt.com:6881"
                 << " --bt-enable-lpd=true --bt-lpd-interface=0.0.0.0"
                 << " --bt-external-ip=auto --bt-external-ip-version=4"
                 << " --bt-prioritize-piece=head=32M,tail=32M"
                 << " --bt-detach-seed-only=true --bt-remove-unselected-file=true"
                 << " --max-tries=10 --retry-wait=5 --timeout=60"
                 << " --connect-timeout=30 --max-connection-per-server=4"
                 << " --split=4 --min-split-size=1M --piece-length=1M"
                 << " --bt-hash-check-seed=true --bt-seed-unverified=true"
                 << " \"" << magnetLink << "\"\n";
        batchOut.close();
        
        cout << "Batch file created at: " << batchFile << "\n";
        cout << "Using simplified aria2c configuration...\n";
        
        command = "cmd /c \"" + batchFile + "\"";
    } else {
        // Fallback to direct command (may have issues with special chars)
        command = "cmd /c \"\"" + baseCmd +
                  "\" --dir=\"" + finalPath +
                  "\" --bt-max-peers=50 --continue=true \"" +
                  magnetLink + "\"\"\"";
    }
#else
    baseCmd = "aria2c";
    command = "aria2c --dir=\"" + finalPath +
              "\" --bt-max-peers=50 --continue=true \"" +
              magnetLink + "\"";
#endif

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
    
    // Test Windows network connectivity
    cout << "\nTesting Windows network connectivity...\n";
    int netTest = system("ping -n 1 8.8.8.8 >nul 2>&1");
    if (netTest == 0) {
        cout << "Network connectivity: OK\n";
    } else {
        cout << "Network connectivity: ISSUE DETECTED\n";
        cout << "This may cause download problems. Check your internet connection.\n";
    }
    
    cout << "\nStarting download using aria2c...\n";
    cout << "Command: " << command << "\n";
    cout << "\nDebugging info:\n";
    cout << "- Aria2c path: " << baseCmd << "\n";
    cout << "- Download directory: " << finalPath << "\n";
    cout << "- Magnet link length: " << magnetLink.length() << " characters\n";
    cout << "\nIf download gets stuck, try:\n";
    cout << "1. Run as Administrator (Right-click -> Run as administrator)\n";
    cout << "2. Check Windows Firewall - allow aria2c through firewall\n";
    cout << "3. Disable Windows Defender real-time protection temporarily\n";
    cout << "4. Check if your ISP blocks BitTorrent traffic\n";
    cout << "5. Try a VPN if your ISP blocks BitTorrent\n";
    cout << "6. Check Windows Network Discovery settings\n\n";
    cout << "Windows-specific networking optimizations applied...\n\n";
    
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
    
    // Clean up batch file if it was created
#ifdef _WIN32
    if (command.find("aria2_temp.bat") != string::npos) {
        string cleanupBatch = "del \"" + batchFile + "\" >nul 2>&1";
        system(cleanupBatch.c_str());
    }
#endif

#ifdef _WIN32
    cout << "Press Enter to exit...";
    cin.get();
#endif

    return 0;
}
