// Dumb hello world in C++
// https://code.visualstudio.com/docs/languages/cpp
// https://code.visualstudio.com/docs/cpp/config-msvc

// File Watcher stuff
// https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-readdirectorychangesw
// https://cppcodetips.wordpress.com/2019/07/26/file-system-watcher-in-c-for-windows/

#include <iostream>
#include <windows.h>
#include <Winbase.h>

// Should be a custom .h file somewhere.
extern "C" {
    void call_me_from_c(uint32_t num);
}

int main() {
    // Program starting
    std::wcout << "Hello world" << std::endl;

    // Doesn't work yet
    // call_me_from_c(5);

    // Open a directory
    HANDLE hDir = CreateFile(
        TEXT("C:\\Users\\Robert\\Documents\\GitHub\\cyberdrive-poc"), // hard-coded
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS,
        NULL);

    if(hDir == INVALID_HANDLE_VALUE) {
        std::wcout << "Canot open directory. Exiting." << std::endl;
        exit(1);
    }

    std::wcout << "Waiting for directory changes. ^Break to exit. I am not sophisticated" << std::endl;

    FILE_NOTIFY_INFORMATION buffer[1024];
    DWORD bytesReturned;
    OVERLAPPED overlapped;
    while(ReadDirectoryChangesW(
        hDir,
        &buffer,
        sizeof(buffer),
        TRUE,
        FILE_NOTIFY_CHANGE_SECURITY    | FILE_NOTIFY_CHANGE_CREATION |
        FILE_NOTIFY_CHANGE_LAST_ACCESS | FILE_NOTIFY_CHANGE_LAST_WRITE |
        FILE_NOTIFY_CHANGE_SIZE        | FILE_NOTIFY_CHANGE_ATTRIBUTES |
        FILE_NOTIFY_CHANGE_DIR_NAME    | FILE_NOTIFY_CHANGE_FILE_NAME,
        &bytesReturned,
        NULL,
        NULL))
        {
            std::wcout << "A change was detected" << std::endl;

            // Unpack file notify info from buffer
            int offset = 0;
            FILE_NOTIFY_INFORMATION* pNotify;
            pNotify = (FILE_NOTIFY_INFORMATION*)((char*)buffer + offset);
            
            wchar_t filename[MAX_PATH];
            wcscpy(filename, L"");
            wcsncpy(filename, pNotify->FileName, pNotify->FileNameLength);

           std:: wcout << "Action= " << pNotify->Action << " " << filename << std::endl;

            // Could loop round for more info here, incrementing offset
            // but not doing so for reasons of time
        }

    // In practice this won't be executed here, because we're not catching the exit cleanly
    // but make an effort. Untested.
    if(hDir) {
        CloseHandle(hDir);
        hDir = NULL;
    }
}

