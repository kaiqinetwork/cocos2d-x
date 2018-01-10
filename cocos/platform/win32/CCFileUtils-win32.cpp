/****************************************************************************
Copyright (c) 2010-2012 cocos2d-x.org
Copyright (c) 2013-2017 Chukong Technologies Inc.

http://www.cocos2d-x.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/

#include "platform/CCPlatformConfig.h"
#if CC_TARGET_PLATFORM == CC_PLATFORM_WIN32

#include "platform/win32/CCFileUtils-win32.h"
#include "platform/win32/CCUtils-win32.h"
#include "platform/CCCommon.h"
#include <Shlobj.h>
#include <cstdlib>
#include <regex>
#include <sstream>

using namespace std;

NS_CC_BEGIN

#define CC_MAX_PATH  512

#ifdef UNICODE
typedef	std::wstring tstring;
#else
typedef	std::string tstring;
#endif // UNICODE

// The root path of resources, the character encoding is UTF-8.
// UTF-8 is the only encoding supported by cocos2d-x API.
static std::string s_resourcePath = "";

// D:\aaa\bbb\ccc\ddd\abc.txt --> D:/aaa/bbb/ccc/ddd/abc.txt
static inline std::string convertPathFormatToUnixStyle(const std::string& path)
{
    std::string ret = path;
    int len = ret.length();
    for (int i = 0; i < len; ++i)
    {
        if (ret[i] == '\\')
        {
            ret[i] = '/';
        }
    }
    return ret;
}

static std::string MultiByteStringToUtf8(const std::string& strMultiByte)
{
	std::string ret;
	if (!strMultiByte.empty())
	{
		int nNum = MultiByteToWideChar(CP_ACP, 0, strMultiByte.c_str(), -1, nullptr, 0);
		if (nNum)
		{
			WCHAR* wideCharString = new WCHAR[nNum + 1];
			wideCharString[0] = 0;
			nNum = MultiByteToWideChar(CP_ACP, 0, strMultiByte.c_str(), -1, wideCharString, nNum);

			std::wstring strWideChar = wideCharString;
			delete[] wideCharString;

			ret = StringWideCharToUtf8(strWideChar);
		}
		else
		{
			CCLOG("Wrong convert to Utf8 code:0x%x", GetLastError());
		}
	}

	return ret;
}

static void _checkPath()
{
    if (s_resourcePath.empty())
    {
        WCHAR utf16Path[CC_MAX_PATH] = { 0 };
        GetModuleFileNameW(NULL, utf16Path, CC_MAX_PATH - 1);
        WCHAR *pUtf16ExePath = &(utf16Path[0]);

		// We need only directory part without exe
		WCHAR *pUtf16DirEnd = wcsrchr(pUtf16ExePath, L'\\');

        char utf8ExeDir[CC_MAX_PATH] = { 0 };
        int nNum = WideCharToMultiByte(CP_UTF8, 0, pUtf16ExePath, pUtf16DirEnd-pUtf16ExePath+1, utf8ExeDir, sizeof(utf8ExeDir), nullptr, nullptr);
        s_resourcePath = convertPathFormatToUnixStyle(utf8ExeDir);
    }
}

FileUtils* FileUtils::getInstance()
{
    if (s_sharedFileUtils == nullptr)
    {
        s_sharedFileUtils = new FileUtilsWin32();
        if(!s_sharedFileUtils->init())
        {
          delete s_sharedFileUtils;
          s_sharedFileUtils = nullptr;
          CCLOG("ERROR: Could not init CCFileUtilsWin32");
        }
    }
    return s_sharedFileUtils;
}

FileUtilsWin32::FileUtilsWin32()
{
}

bool FileUtilsWin32::init()
{
    _checkPath();
    _defaultResRootPath = s_resourcePath;
    return FileUtils::init();
}

bool FileUtilsWin32::isDirectoryExistInternal(const std::string& dirPath) const
{
#ifdef UNICODE
	unsigned long fAttrib = GetFileAttributes(StringUtf8ToWideChar(dirPath).c_str());
#else
	unsigned long fAttrib = GetFileAttributes(UTF8StringToMultiByte(dirPath).c_str());
#endif // UNICODE

    if (fAttrib != INVALID_FILE_ATTRIBUTES &&
        (fAttrib & FILE_ATTRIBUTE_DIRECTORY))
    {
        return true;
    }
    return false;
}

std::string FileUtilsWin32::getSuitableFOpen(const std::string& filenameUtf8) const
{
    return UTF8StringToMultiByte(filenameUtf8);
}

long FileUtilsWin32::getFileSize(const std::string &filepath)
{
    WIN32_FILE_ATTRIBUTE_DATA fad;
#ifdef UNICODE
    if (!GetFileAttributesEx(StringUtf8ToWideChar(filepath).c_str(), GetFileExInfoStandard, &fad))
#else
	if (!GetFileAttributesEx(UTF8StringToMultiByte(filepath).c_str(), GetFileExInfoStandard, &fad))
#endif
    {
        return 0; // error condition, could call GetLastError to find out more
    }
    LARGE_INTEGER size;
    size.HighPart = fad.nFileSizeHigh;
    size.LowPart = fad.nFileSizeLow;
    return (long)size.QuadPart;
}

bool FileUtilsWin32::isFileExistInternal(const std::string& strFilePath) const
{
    if (strFilePath.empty())
    {
        return false;
    }

    std::string strPath = strFilePath;
    if (!isAbsolutePath(strPath))
    { // Not absolute path, add the default root path at the beginning.
        strPath.insert(0, _defaultResRootPath);
    }

    DWORD attr = GetFileAttributesW(StringUtf8ToWideChar(strPath).c_str());
    if(attr == INVALID_FILE_ATTRIBUTES || (attr & FILE_ATTRIBUTE_DIRECTORY))
        return false;   //  not a file
    return true;
}

bool FileUtilsWin32::isAbsolutePath(const std::string& strPath) const
{
    if (   (strPath.length() > 2
        && ( (strPath[0] >= 'a' && strPath[0] <= 'z') || (strPath[0] >= 'A' && strPath[0] <= 'Z') )
        && strPath[1] == ':') || (strPath[0] == '/' && strPath[1] == '/'))
    {
        return true;
    }
    return false;
}


FileUtils::Status FileUtilsWin32::getContents(const std::string& filename, ResizableBuffer* buffer)
{
	if (filename.empty())
		return FileUtils::Status::NotExists;

	// read the file from hardware
	std::string fullPath = FileUtils::getInstance()->fullPathForFilename(filename);

	HANDLE fileHandle = ::CreateFileW(StringUtf8ToWideChar(fullPath).c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, nullptr);
	if (fileHandle == INVALID_HANDLE_VALUE)
		return FileUtils::Status::OpenFailed;

	DWORD hi;
	auto size = ::GetFileSize(fileHandle, &hi);
	if (hi > 0)
	{
		::CloseHandle(fileHandle);
		return FileUtils::Status::TooLarge;
	}

	// don't read file content if it is empty
	if (size == 0)
	{
		::CloseHandle(fileHandle);
		return FileUtils::Status::OK;
	}

	buffer->resize(size);
	DWORD sizeRead = 0;
	BOOL successed = ::ReadFile(fileHandle, buffer->buffer(), size, &sizeRead, nullptr);
	::CloseHandle(fileHandle);

	if (!successed) {
		CCLOG("Get data from file(%s) failed, error code is %s", filename.data(), std::to_string(::GetLastError()).data());
		buffer->resize(sizeRead);
		return FileUtils::Status::ReadFailed;
	}
	return FileUtils::Status::OK;
}

std::string FileUtilsWin32::getPathForFilename(const std::string& filename, const std::string& resolutionDirectory, const std::string& searchPath) const
{
    std::string unixFileName = convertPathFormatToUnixStyle(filename);
    std::string unixResolutionDirectory = convertPathFormatToUnixStyle(resolutionDirectory);
    std::string unixSearchPath = convertPathFormatToUnixStyle(searchPath);

    return FileUtils::getPathForFilename(unixFileName, unixResolutionDirectory, unixSearchPath);
}

std::string FileUtilsWin32::getFullPathForDirectoryAndFilename(const std::string& strDirectory, const std::string& strFilename) const
{
    std::string unixDirectory = convertPathFormatToUnixStyle(strDirectory);
    std::string unixFilename = convertPathFormatToUnixStyle(strFilename);

    return FileUtils::getFullPathForDirectoryAndFilename(unixDirectory, unixFilename);
}

string FileUtilsWin32::getWritablePath() const
{
    if (_writablePath.length())
    {
        return _writablePath;
    }

    // Get full path of executable, e.g. c:\Program Files (x86)\My Game Folder\MyGame.exe
    TCHAR full_path[CC_MAX_PATH + 1] = { 0 };
    ::GetModuleFileName(nullptr, full_path, CC_MAX_PATH + 1);

    // Debug app uses executable directory; Non-debug app uses local app data directory
//#ifndef _DEBUG
    // Get filename of executable only, e.g. MyGame.exe
#ifdef UNICODE
    WCHAR *base_name = wcsrchr(full_path, L'\\');
	wstring retPath;
#else
	string retPath;
	char *base_name = strchr(full_path, '\\');
#endif // UNICODE
    if(base_name)
    {
        TCHAR app_data_path[CC_MAX_PATH + 1];

        // Get local app data directory, e.g. C:\Documents and Settings\username\Local Settings\Application Data
        if (SUCCEEDED(SHGetFolderPath(nullptr, CSIDL_LOCAL_APPDATA, nullptr, SHGFP_TYPE_CURRENT, app_data_path)))
        {
#ifdef UNICODE
            wstring ret(app_data_path);
#else
			string ret(app_data_path);
#endif // UNICODE

            // Adding executable filename, e.g. C:\Documents and Settings\username\Local Settings\Application Data\MyGame.exe
            ret += base_name;

            // Remove ".exe" extension, e.g. C:\Documents and Settings\username\Local Settings\Application Data\MyGame
            ret = ret.substr(0, ret.rfind(TEXT(".")));

			ret += TEXT("\\");

            // Create directory
            if (SUCCEEDED(SHCreateDirectoryEx(nullptr, ret.c_str(), nullptr)))
            {
                retPath = ret;
            }
        }
    }
    if (retPath.empty())
//#endif // not defined _DEBUG
    {
        // If fetching of local app data directory fails, use the executable one
        retPath = full_path;

        // remove xxx.exe
		retPath = retPath.substr(0, retPath.rfind(TEXT("\\")) + 1);
    }

#ifdef UNICODE
	return convertPathFormatToUnixStyle(StringWideCharToUtf8(retPath));
#else
	return convertPathFormatToUnixStyle(MultiByteStringToUtf8(retPath));
#endif // UNICODE
}

bool FileUtilsWin32::renameFile(const std::string &oldfullpath, const std::string& newfullpath)
{
    CCASSERT(!oldfullpath.empty(), "Invalid path");
    CCASSERT(!newfullpath.empty(), "Invalid path");

#ifdef UNICODE
	std::wstring _strNew = StringUtf8ToWideChar(newfullpath);
	std::wstring _strOld = StringUtf8ToWideChar(oldfullpath);
#else
	std::string _strNew = UTF8StringToMultiByte(newfullpath);
	std::string _strOld = UTF8StringToMultiByte(oldfullpath);
#endif // UNICODE

    if (FileUtils::getInstance()->isFileExist(newfullpath))
    {
        if (!DeleteFile(_strNew.c_str()))
        {
            CCLOGERROR("Fail to delete file %s !Error code is 0x%x", newfullpath.c_str(), GetLastError());
        }
    }

    if (MoveFile(_strOld.c_str(), _strNew.c_str()))
    {
        return true;
    }
    else
    {
        CCLOGERROR("Fail to rename file %s to %s !Error code is 0x%x", oldfullpath.c_str(), newfullpath.c_str(), GetLastError());
        return false;
    }
}

bool FileUtilsWin32::renameFile(const std::string &path, const std::string &oldname, const std::string &name)
{
    CCASSERT(!path.empty(), "Invalid path");
    std::string oldPath = path + oldname;
    std::string newPath = path + name;

    std::regex pat("\\/");
    std::string _old = std::regex_replace(oldPath, pat, "\\");
    std::string _new = std::regex_replace(newPath, pat, "\\");

    return renameFile(_old, _new);
}

bool FileUtilsWin32::createDirectory(const std::string& dirPath)
{
    CCASSERT(!dirPath.empty(), "Invalid path");

    if (isDirectoryExist(dirPath))
        return true;

#ifdef UNICODE
    std::wstring path = StringUtf8ToWideChar(dirPath);
#else
	std::string path = UTF8StringToMultiByte(dirPath);
#endif // UNICODE

    // Split the path
    size_t start = 0;
    size_t found = path.find_first_of(TEXT("/\\"), start);
    tstring subpath;
    std::vector<tstring> dirs;

    if (found != tstring::npos)
    {
        while (true)
        {
            subpath = path.substr(start, found - start + 1);
            if (!subpath.empty())
                dirs.push_back(subpath);
            start = found + 1;
            found = path.find_first_of(TEXT("/\\"), start);
            if (found == tstring::npos)
            {
                if (start < path.length())
                {
                    dirs.push_back(path.substr(start));
                }
                break;
            }
        }
    }

    if ((GetFileAttributes(path.c_str())) == INVALID_FILE_ATTRIBUTES)
    {
        subpath = TEXT("");
        for (unsigned int i = 0, size = dirs.size(); i < size; ++i)
        {
            subpath += dirs[i];
#ifdef UNICODE
			std::string utf8Path = StringWideCharToUtf8(subpath);
#else
			std::string utf8Path = MultiByteStringToUtf8(subpath);
#endif // UNICODE
            if (!isDirectoryExist(utf8Path))
            {
                BOOL ret = CreateDirectory(subpath.c_str(), NULL);
                if (!ret && ERROR_ALREADY_EXISTS != GetLastError())
                {
                    CCLOGERROR("Fail create directory %s !Error code is 0x%x", utf8Path.c_str(), GetLastError());
                    return false;
                }
            }
        }
    }
    return true;
}

bool FileUtilsWin32::removeFile(const std::string &filepath)
{
    std::regex pat("\\/");
    std::string win32path = std::regex_replace(filepath, pat, "\\");

#ifdef UNICODE
	std::wstring strPath = StringUtf8ToWideChar(win32path);
#else
	std::string strPath = UTF8StringToMultiByte(win32path);
#endif // UNICODE

    if (DeleteFile(strPath.c_str()))
    {
        return true;
    }
    else
    {
        CCLOGERROR("Fail remove file %s !Error code is 0x%x", filepath.c_str(), GetLastError());
        return false;
    }
}

bool FileUtilsWin32::removeDirectory(const std::string& dirPath)
{
#ifdef UNICODE
    std::wstring path = StringUtf8ToWideChar(dirPath);
#else
	std::string path = UTF8StringToMultiByte(dirPath);
#endif // UNICODE

	tstring files = path + TEXT("*.*");
    WIN32_FIND_DATA fd;
    HANDLE  search = FindFirstFileEx(files.c_str(), FindExInfoStandard, &fd, FindExSearchNameMatch, NULL, 0);
    bool ret = true;
    if (search != INVALID_HANDLE_VALUE)
    {
        BOOL find = true;
        while (find)
        {
            // Need check string . and .. for delete folders and files begin name.
            tstring fileName = fd.cFileName;
            if (fileName != TEXT(".") && fileName != TEXT(".."))
            {
				tstring temp = path + fd.cFileName;
                if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    temp += '/';
#ifdef UNICODE
					std::string strUtf8Path = StringWideCharToUtf8(temp);
#else
					std::string strUtf8Path = MultiByteStringToUtf8(temp);
#endif // UNICODE
					ret = ret && this->removeDirectory(strUtf8Path);
                }
                else
                {
                    SetFileAttributes(temp.c_str(), FILE_ATTRIBUTE_NORMAL);
                    ret = ret && DeleteFile(temp.c_str());
                }
            }
            find = FindNextFile(search, &fd);
        }
        FindClose(search);
    }
    if (ret && RemoveDirectory(path.c_str()))
    {
        return true;
    }
    return false;
}

NS_CC_END

#endif // CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
