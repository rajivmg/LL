#include "LL_Utility.h"

#include <codecvt>

namespace utility
{
    filelist GetFileList(std::wstring const &Directory, std::wstring const &Pattern /*= L"*.*"*/)
    {
        filelist Result = std::make_shared<std::vector<std::wstring> >();

        std::function<void(std::wstring const &)> PushFilenameRecursive = [&](std::wstring const &SearchDir)
        {
            WIN32_FIND_DATA FileFD;
            std::wstring FileSearchPath = SearchDir + L"\\" + Pattern;

            HANDLE FHandle = ::FindFirstFile(FileSearchPath.c_str(), &FileFD);
            if(FHandle == INVALID_HANDLE_VALUE)
            {
                return;
            }

            for(;;)
            {
                if(wcscmp(FileFD.cFileName, L".") != 0 && wcscmp(FileFD.cFileName, L"..") != 0)
                {
                    // Check of FileFD is a File or Directory
                    if(FileFD.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                    {
                        std::wstring FileSearchDir = SearchDir + L"\\" + FileFD.cFileName;
                        PushFilenameRecursive(FileSearchDir);
                    }
                    else
                    {
                        Result->push_back(FileFD.cFileName);
                    }
                }

                if(!FindNextFile(FHandle, &FileFD))
                {
                    break;
                }
            }
            FindClose(FHandle);
        };

        PushFilenameRecursive(Directory);

        return Result;
    }

    std::string ToString(std::wstring WS)
    {
        const size_t WSLen = wcslen(WS.c_str()) + 1;
        const size_t NewStringBufferSize = WSLen * 2;

        char* NewString = new char[NewStringBufferSize];
        wcstombs_s(NULL, NewString, NewStringBufferSize, WS.c_str(), _TRUNCATE);
        
        std::string Result;
        Result.assign(NewString);
        
        delete[] NewString;
        return Result;
    }
}