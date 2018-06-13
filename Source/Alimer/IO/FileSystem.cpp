//
// Copyright (c) 2018 Amer Koleci and contributors.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include "../IO/FileSystem.h"
#include "../IO/Path.h"
#include "../Util/Util.h"
#include "../Core/Log.h"

#if ALIMER_PLATFORM_WINDOWS || ALIMER_PLATFORM_UWP
#ifndef WIN32_LEAN_AND_MEAN
#	define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#	define NOMINMAX
#endif

#include <windows.h>

inline std::wstring ToWide(const std::string& str)
{
    int len;
    int slength = (int)str.length() + 1;
    len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), slength, nullptr, 0);
    wchar_t* buf = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), slength, buf, len);
    std::wstring r(buf);
    delete[] buf;
    return r;
}

inline std::string ToMultibyte(const std::wstring& str)
{
    int len;
    int slength = (int)str.length() + 1;
    len = WideCharToMultiByte(CP_UTF8, 0, str.c_str(), slength, nullptr, 0, nullptr, nullptr);
    char* buf = new char[len];
    WideCharToMultiByte(CP_UTF8, 0, str.c_str(), slength, buf, len, nullptr, nullptr);
    std::string r(buf);
    delete[] buf;
    return r;
}

std::string ToUniversalPath(const std::string& path)
{
    return Util::Replace(path, "\\", "/");
}
#endif


using namespace std;

namespace Alimer
{
    string GetInternalPath(const string& path)
    {
        return Util::Replace(path, "\\", "/");
    }

    string GetNativePath(const string& path)
    {
#ifdef _WIN32
        return Util::Replace(path, "/", "\\");
#else
        return pathName;
#endif
    }

    string AddTrailingSlash(const string& path)
    {
        string ret = path;
        Util::Trim(ret);
        ret = Util::Replace(ret, "\\", "/");
        if (!ret.empty() && ret.back() != '/')
        {
            ret += '/';
        }

        return ret;
    }

    string RemoveTrailingSlash(const string& path)
    {
        string ret = path;
        Util::Trim(ret);
        ret = Util::Replace(ret, "\\", "/");
        if (!ret.empty() && ret.back() == '/')
        {
            ret.resize(ret.length() - 1);
        }

        return ret;
    }

    void SplitPath(const string& fullPath, string& pathName, string& fileName, string& extension, bool lowerCaseExtension)
    {
        string fullPathCopy = GetInternalPath(fullPath);

        size_t extPos = fullPathCopy.find_last_of('.');
        size_t pathPos = fullPathCopy.find_last_of('/');

        if (extPos != string::npos
            && (pathPos == string::npos || extPos > pathPos))
        {
            extension = fullPathCopy.substr(extPos);
            if (lowerCaseExtension)
            {
                Util::ToLower(extension);
            }

            fullPathCopy = fullPathCopy.substr(0, extPos);
        }
        else {
            extension.clear();
        }

        pathPos = fullPathCopy.find_last_of('/');
        if (pathPos != string::npos)
        {
            fileName = fullPathCopy.substr(pathPos + 1);
            pathName = fullPathCopy.substr(0, pathPos + 1);
        }
        else
        {
            fileName = fullPathCopy;
            pathName.clear();
        }
    }

    string GetPath(const string& fullPath)
    {
        string path, file, extension;
        SplitPath(fullPath, path, file, extension);
        return path;
    }

    string GetFileName(const string& fullPath)
    {
        string path, file, extension;
        SplitPath(fullPath, path, file, extension);
        return file;
    }

    string GetExtension(const string& fullPath, bool lowercaseExtension)
    {
        string path, file, extension;
        SplitPath(fullPath, path, file, extension, lowercaseExtension);
        return extension;
    }

    string GetFileNameAndExtension(const string& fileName, bool lowercaseExtension)
    {
        string path, file, extension;
        SplitPath(fileName, path, file, extension, lowercaseExtension);
        return file + extension;
    }

    string GetParentPath(const string& path)
    {
        size_t pos = RemoveTrailingSlash(path).find_last_of('/');
        if (pos != string::npos)
            return path.substr(0, pos + 1);

        return string();
    }

    bool IsAbsolutePath(const string& pathName)
    {
        if (pathName.empty())
            return false;

        string path = GetInternalPath(pathName);

        if (path[0] == '/')
            return true;

#ifdef _WIN32
        if (path.length() > 1 && Util::IsAlpha(path[0]) && path[1] == ':')
            return true;
#endif

        return false;
    }

#ifdef _WIN32
    string GetCurrentDir()
    {
        wchar_t path[MAX_PATH];
        path[0] = 0;
        GetCurrentDirectoryW(MAX_PATH, path);
        return ToMultibyte(path);
    }

    string GetExecutableFolder()
    {
        wchar_t exeName[MAX_PATH];
        exeName[0] = 0;
        GetModuleFileNameW(nullptr, exeName, MAX_PATH);
        return GetPath(ToUniversalPath(ToMultibyte(exeName)));
    }

    bool FileExists(const std::string& fileName)
    {
        string fixedName = GetNativePath(RemoveTrailingSlash(fileName));

        DWORD attributes = GetFileAttributesW(ToWide(fixedName).c_str());
        if (attributes == INVALID_FILE_ATTRIBUTES || attributes & FILE_ATTRIBUTE_DIRECTORY)
            return false;

        return true;
    }

    bool DirectoryExists(const string& path)
    {
        string fixedName = GetNativePath(RemoveTrailingSlash(path));

        DWORD attributes = GetFileAttributesW(ToWide(fixedName).c_str());
        if (attributes == INVALID_FILE_ATTRIBUTES
            || !(attributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            return false;
        }
        return true;
    }

    class Win32FileStream final : public Stream
    {
    public:
        Win32FileStream(const string &path, StreamMode mode);
        ~Win32FileStream() override;

        bool CanSeek() const override { return _handle != nullptr; }

        size_t Read(void* dest, size_t size) override;
        void Write(const void* data, size_t size) override;

    private:
        HANDLE _handle = nullptr;
    };

    static bool Win32EnsureDirectoryInner(const string &path)
    {
        if (Path::IsRootPath(path))
            return false;

        if (DirectoryExists(path))
            return true;

        auto basedir = Path::GetBaseDir(path);
        if (!Win32EnsureDirectoryInner(basedir))
            return false;

        if (!CreateDirectoryA(path.c_str(), nullptr))
        {
            return GetLastError() == ERROR_ALREADY_EXISTS;
        }

        return true;
    }

    static bool Win32EnsureDirectory(const std::string &path)
    {
       string basedir = Path::GetBaseDir(path);
        return Win32EnsureDirectoryInner(basedir);
    }

    Win32FileStream::Win32FileStream(const string &path, StreamMode mode)
    {
        _name = path;
        DWORD access = 0;
        DWORD disposition = 0;

        switch (mode)
        {
        case StreamMode::ReadOnly:
            access = GENERIC_READ;
            disposition = OPEN_EXISTING;
            break;

        case StreamMode::ReadWrite:
            if (!Win32EnsureDirectory(path))
            {
                ALIMER_LOGCRITICAL("Win32 Stream failed to create directory");
            }

            access = GENERIC_READ | GENERIC_WRITE;
            disposition = OPEN_ALWAYS;
            break;

        case StreamMode::WriteOnly:
            if (!Win32EnsureDirectory(path))
            {
                ALIMER_LOGCRITICAL("Win32 Stream failed to create directory");
            }

            access = GENERIC_READ | GENERIC_WRITE;
            disposition = CREATE_ALWAYS;
            break;
        }

        _handle = CreateFileA(path.c_str(), access, FILE_SHARE_READ, nullptr, disposition, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, INVALID_HANDLE_VALUE);
        if (_handle == INVALID_HANDLE_VALUE)
        {
            ALIMER_LOGERROR("Failed to open file: %s.\n", path.c_str());
            throw runtime_error("FileStream::FileStream()");
        }

        LARGE_INTEGER size;
        if (!GetFileSizeEx(_handle, &size))
        {
            throw runtime_error("[Win32] - GetFileSizeEx: failed");
        }

        _position = 0;
        _size = static_cast<size_t>(size.QuadPart);
    }

    Win32FileStream::~Win32FileStream()
    {
        if (_handle != INVALID_HANDLE_VALUE)
        {
            CloseHandle(_handle);
            _handle = INVALID_HANDLE_VALUE;
        }

        _position = 0;
        _size = 0;
    }

    size_t Win32FileStream::Read(void* dest, size_t size)
    {
        if (!CanRead())
        {
            ALIMER_LOGERROR("Cannot read for write only stream");
            return static_cast<size_t>(-1);
        }

        DWORD byteRead;
        if (!ReadFile(_handle, dest, static_cast<DWORD>(size), &byteRead, nullptr))
        {
            return static_cast<size_t>(-1);
        }

        _position += byteRead;
        return static_cast<size_t>(byteRead);
    }

    void Win32FileStream::Write(const void* data, size_t size)
    {
        if (!size)
            return;

        DWORD byteWritten;
        if (!WriteFile(_handle, data, static_cast<DWORD>(size), &byteWritten, nullptr))
        {
            return;
        }

        _position += byteWritten;
        if (_position > _size)
            _size = _position;
    }

    UniquePtr<Stream> OpenStream(const string &path, StreamMode mode)
    {
        if (mode == StreamMode::ReadOnly
            && !FileExists(path))
        {
            return nullptr;
        }

        try
        {
            UniquePtr<Stream> file(new Win32FileStream(path, mode));
            return file;
        }
        catch (const std::exception &e)
        {
            ALIMER_LOGERROR("OSFileSystem::Open(): %s", e.what());
            return {};
        }
    }

#else
    string GetCurrentDir()
    {
        char path[MAX_PATH];
        path[0] = 0;
        getcwd(path, MAX_PATH);
        return string(path);
    }

    string GetExecutableFolder()
    {
#if defined(__linux__)
        char exeName[MAX_PATH];
        memset(exeName, 0, MAX_PATH);
        pid_t pid = getpid();
        String link = "/proc/" + String(pid) + "/exe";
        readlink(link.CString(), exeName, MAX_PATH);
        return GetPath(string(exeName));
#elif defined(__APPLE__)
        char exeName[MAX_PATH];
        memset(exeName, 0, MAX_PATH);
        unsigned size = MAX_PATH;
        _NSGetExecutablePath(exeName, &size);
        return GetPath(string(exeName));
#else
        return GetCurrentDir();
#endif
    }

    bool FileExists(const std::string& fileName)
    {
        string fixedName = GetNativePath(RemoveTrailingSlash(fileName));

        struct stat st {};
        if (stat(fixedName.c_str(), &st) || st.st_mode & S_IFDIR)
            return false;

        return true;
    }

    bool DirectoryExists(const string& path)
    {
        // Always return true for the root directory
        if (path == "/")
            return true;

        string fixedName = GetNativePath(RemoveTrailingSlash(path));

        struct stat st {};
        if (stat(fixedName.CString(), &st) || !(st.st_mode & S_IFDIR))
            return false;
        return true;
    }

    
#endif
}
