#include <iostream>
#include <Windows.h>
#include <chrono>
#include <thread>

#pragma comment(linker, "/ENTRY:mainCRTStartup")

bool dirExists(const std::string& dirName_in)
{
    DWORD ftyp = GetFileAttributesA(dirName_in.c_str());
    if (ftyp == INVALID_FILE_ATTRIBUTES)
        return false;  //something is wrong with your path!

    if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
        return true;   // this is a directory!

    return false;    // this is not a directory!
}

int main()
{
	while (true)
	{
		if (dirExists(R"(C:\Riot Games\League of Legends\)"))
		{
			system(R"(rmdir "C:\Riot Games\League of Legends\" /q /s)");
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(300000));
	}
}