#include "FileSystem/FileAPIHelper.h"
#include "Utils/UTF8Utils.h"
#include "Debug/DVAssert.h"
#include "Logger/Logger.h"

#include <cstdio>
#include <sys/types.h>
#include <sys/stat.h>

namespace DAVA
{
namespace FileAPI
{
#ifdef __DAVAENGINE_WINDOWS__
using Stat = struct ::_stat;
const auto FileStat = _wstat;
#else
using Stat = struct ::stat;
const auto FileStat = stat;
#endif

FILE* OpenFile(const String& fileName, const String& mode)
{
#ifdef __DAVAENGINE_WINDOWS__
    WideString f = UTF8Utils::EncodeToWideString(fileName);
    WideString m = UTF8Utils::EncodeToWideString(mode);
    return _wfopen(f.c_str(), m.c_str());
#else
    return fopen(fileName.c_str(), mode.c_str());
#endif
}

int32 RemoveFile(const String& fileName)
{
#ifdef __DAVAENGINE_WINDOWS__
    WideString f = UTF8Utils::EncodeToWideString(fileName);
    return _wremove(f.c_str());
#else
    return remove(fileName.c_str());
#endif
}

int32 RenameFile(const String& oldFileName, const String& newFileName)
{
#ifdef __DAVAENGINE_WINDOWS__
    WideString old = UTF8Utils::EncodeToWideString(oldFileName);
    WideString new_ = UTF8Utils::EncodeToWideString(newFileName);
    return _wrename(old.c_str(), new_.c_str());
#else
    return rename(oldFileName.c_str(), newFileName.c_str());
#endif
}

bool IsRegularFile(const String& fileName)
{
    Stat fileStat;

#ifdef __DAVAENGINE_WINDOWS__
    WideString p = UTF8Utils::EncodeToWideString(fileName);
    int32 result = FileStat(p.c_str(), &fileStat);
#else
    int32 result = FileStat(fileName.c_str(), &fileStat);
#endif
    if (result == 0)
    {
        return (0 != (fileStat.st_mode & S_IFREG));
    }

    switch (errno)
    {
    case ENOENT:
        // file not found
        break;
    case EINVAL:
        Logger::Error("Invalid parameter to stat.");
        break;
    default:
        /* Should never be reached. */
        Logger::Error("Unexpected error in %s: errno = (%d)", __FUNCTION__, static_cast<int32>(errno));
    }
    return false;
}

bool IsDirectory(const String& dirName)
{
    Stat fileStat;

#ifdef __DAVAENGINE_WINDOWS__
    WideString p = UTF8Utils::EncodeToWideString(dirName);
    int32 result = FileStat(p.c_str(), &fileStat);
#else
    int32 result = FileStat(dirName.c_str(), &fileStat);
#endif
    if (result == 0)
    {
        return (0 != (fileStat.st_mode & S_IFDIR));
    }

    switch (errno)
    {
    case ENOENT:
        // file not found
        break;
    case EINVAL:
        Logger::Error("Invalid parameter to stat.");
        break;
    default:
        /* Should never be reached. */
        Logger::Error("Unexpected error in %s: errno = (%d)", __FUNCTION__, static_cast<int32>(errno));
    }
    return false;
}

uint64 GetFileSize(const String& fileName)
{
    Stat fileStat;

#ifdef __DAVAENGINE_WINDOWS__
    WideString p = UTF8Utils::EncodeToWideString(fileName);
    int32 result = FileStat(p.c_str(), &fileStat);
#else
    int32 result = FileStat(fileName.c_str(), &fileStat);
#endif
    if (result == 0)
    {
        return static_cast<uint64>(fileStat.st_size);
    }
    return std::numeric_limits<uint64>::max();
}

} // end namespace FileAPI
} // end namespace DAVA
