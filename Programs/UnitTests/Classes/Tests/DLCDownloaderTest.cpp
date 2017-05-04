#include "UnitTests/UnitTests.h"

#include <DLCManager/DLCDownloader.h>
#include <Logger/Logger.h>
#include <DLC/Downloader/DownloadManager.h>
#include <FileSystem/FileSystem.h>
#include <Time/SystemTimer.h>
#include <Utils/CRC32.h>
#include <EmbeddedWebServer.h>

#include <iomanip>
#include <Engine/Engine.h>

class MemBufWriter final : public DAVA::DLCDownloader::IWriter
{
public:
    MemBufWriter(void* buff, size_t size)
    {
        DVASSERT(buff != nullptr);
        DVASSERT(size > 0);

        start = static_cast<char*>(buff);
        current = start;
        end = start + size;
    }

    DAVA::uint64 Save(const void* ptr, DAVA::uint64 size) override
    {
        using namespace DAVA;
        uint64 space = SpaceLeft();
        if (size > space)
        {
            DAVA_THROW(Exception, "memory corruption");
        }
        memcpy(current, ptr, static_cast<size_t>(size));
        current += size;
        return size;
    }

    DAVA::uint64 GetSeekPos() override
    {
        return current - start;
    }

    bool Truncate() override
    {
        current = start;
        return true;
    }

    DAVA::uint64 SpaceLeft() const
    {
        return end - current;
    }

private:
    char* start = nullptr;
    char* current = nullptr;
    char* end = nullptr;
};

static const DAVA::String URL = "http://127.0.0.1:8080/superpack_for_unittests.dvpk";
// "http://127.0.0.1:8080/superpack_for_unittests.dvpk"; // embedded web server
// "http://dl-wotblitz.wargaming.net/dlc/r11608713/3.7.0.236.dvpk"; // CDN
// "http://by1-builddlc-01.corp.wargaming.local/DLC_Blitz/smart_dlc/3.7.0.236.dvpk" // local net server

class EmbededWebServer
{
public:
    EmbededWebServer()
    {
        using namespace DAVA;
        FilePath downloadedPacksDir("~doc:/UnitTests/DLCManagerTest/packs/");

        FileSystem* fs = GetEngineContext()->fileSystem;
        // every time clear directory to download once again
        fs->DeleteDirectory(downloadedPacksDir);
        fs->CreateDirectory(downloadedPacksDir, true);

        FilePath destPath = downloadedPacksDir + "superpack_for_unittests.dvpk";
        FilePath srcPath = "~res:/superpack_for_unittests.dvpk";
        if (!fs->IsFile(srcPath))
        {
            Logger::Error("no super pack file!");
            TEST_VERIFY(false);
        }

        if (!fs->CopyFile(srcPath, destPath, true))
        {
            Logger::Error("can't copy super pack for unittest from res:/");
            TEST_VERIFY(false);
            return;
        }

        String path = downloadedPacksDir.GetAbsolutePathname();

        if (!StartEmbeddedWebServer(path.c_str(), "8080"))
        {
            DAVA_THROW(DAVA::Exception, "can't start embedded web server");
        }
    }
    ~EmbededWebServer()
    {
        DAVA::StopEmbeddedWebServer();
    }
};

DAVA_TESTCLASS (DLCDownloaderTest)
{
    EmbededWebServer embeddedServer;
    const DAVA::int64 FULL_SIZE_ON_SERVER = 29738138; // old full dlc build size 1618083461;

    DAVA_TEST (GetFileSizeTest)
    {
        using namespace DAVA;

        DLCDownloader* downloader = DLCDownloader::Create();
        String url = URL;
        DLCDownloader::Task* task = downloader->StartGetContentSize(url);

        downloader->WaitTask(task);

        auto& info = downloader->GetTaskInfo(task);
        auto& status = downloader->GetTaskStatus(task);

        TEST_VERIFY(info.rangeOffset == -1);
        TEST_VERIFY(info.rangeSize == -1);
        TEST_VERIFY(info.dstPath == "");
        TEST_VERIFY(info.srcUrl == url);
        TEST_VERIFY(info.timeoutSec >= 0);
        TEST_VERIFY(info.type == DLCDownloader::TaskType::SIZE);

        TEST_VERIFY(status.error.httpCode == 200);
        TEST_VERIFY(status.error.errorHappened == false);
        TEST_VERIFY(status.error.curlErr == 0);
        TEST_VERIFY(status.error.errStr == nullptr);
        TEST_VERIFY(status.error.curlMErr == 0);
        TEST_VERIFY(status.error.fileErrno == 0);
        TEST_VERIFY(status.sizeDownloaded == 0);
        TEST_VERIFY(status.state.load() == DLCDownloader::TaskState::Finished);
        TEST_VERIFY(status.sizeTotal == FULL_SIZE_ON_SERVER);

        downloader->RemoveTask(task);

        DLCDownloader::Destroy(downloader);
    }

    DAVA_TEST (RangeRequestTest)
    {
        using namespace DAVA;

        std::array<char, 4> buf;
        MemBufWriter writer(buf.data(), buf.size());

        DLCDownloader* downloader = DLCDownloader::Create();
        String url = URL;
        int64 startRangeIndex = FULL_SIZE_ON_SERVER - 4;
        int64 rangeSize = 4;
        DLCDownloader::Task* downloadLast4Bytes = downloader->StartTask(url, writer, DLCDownloader::Range(startRangeIndex, rangeSize));

        downloader->WaitTask(downloadLast4Bytes);

        auto& info = downloader->GetTaskInfo(downloadLast4Bytes);
        auto& status = downloader->GetTaskStatus(downloadLast4Bytes);

        TEST_VERIFY(info.rangeOffset == startRangeIndex);
        TEST_VERIFY(info.rangeSize == rangeSize);
        TEST_VERIFY(info.dstPath == "");
        TEST_VERIFY(info.srcUrl == url);
        TEST_VERIFY(info.timeoutSec >= 0);
        TEST_VERIFY(info.type == DLCDownloader::TaskType::FULL);

        TEST_VERIFY(status.error.errorHappened == false);
        TEST_VERIFY(status.error.httpCode <= 206);
        TEST_VERIFY(status.error.curlErr == 0);
        TEST_VERIFY(status.error.errStr == nullptr);
        TEST_VERIFY(status.error.curlMErr == 0);
        TEST_VERIFY(status.error.fileErrno == 0);
        TEST_VERIFY(status.sizeDownloaded == 4);
        TEST_VERIFY(status.state == DLCDownloader::TaskState::Finished);
        TEST_VERIFY(status.sizeTotal == 4);

        std::array<char, 4> shouldBe{ 'D', 'V', 'P', 'K' };
        TEST_VERIFY(shouldBe == buf);

        downloader->RemoveTask(downloadLast4Bytes);

        DLCDownloader::Destroy(downloader);
    }

    DAVA_TEST (DowloadLargeFileTest)
    {
        using namespace DAVA;

        FileSystem* fs = FileSystem::Instance();
        std::unique_ptr<DLCDownloader> downloader(DLCDownloader::Create());
        String url = URL;
        FilePath path("~doc:/big_tmp_file_from_server.remove.me");
        fs->DeleteFile(path);
        String p = path.GetAbsolutePathname();
        int64 start = 0;
        DLCDownloader::Task* task = nullptr;
        int64 finish = 0;
        float seconds = 0.f;
        float sizeInGb = FULL_SIZE_ON_SERVER / (1024.f * 1024.f * 1024.f);

        DownloadManager* dm = DownloadManager::Instance();

        FilePath pathOld("~doc:/big_tmp_file_from_server.old.remove.me");
        fs->DeleteFile(pathOld);

        //////----first--------------------------------------------------------
        start = SystemTimer::GetMs();

        int numOfParts = 4;

        uint32 id = dm->Download(url, pathOld, FULL);

        dm->Wait(id);

        finish = SystemTimer::GetMs();

        seconds = (finish - start) / 1000.f;

        Logger::Info("old downloader %f Gb parts(%d) download from in house server for: %f", sizeInGb, numOfParts, seconds);
        //// ----next-------------------------------------------------------
        {
            start = SystemTimer::GetMs();

            task = downloader->StartTask(url, p);

            downloader->WaitTask(task);
        }

        finish = SystemTimer::GetMs();

        seconds = (finish - start) / 1000.f;

        Logger::Info("new downloader %f Gb download from in house server for: %f", sizeInGb, seconds);

        downloader->RemoveTask(task);

        const uint32 crc32 = 0x89D4BC4E; // old crc32 for full build 0xDE5C2B62;

        uint32 crcFromFile = CRC32::ForFile(p);

        TEST_VERIFY(crcFromFile == crc32);
        ////-----resume-downloading------------------------------------------------
        File* file = File::Create(p, File::OPEN | File::READ | File::WRITE);
        if (file)
        {
            uint64 fileSize = file->GetSize();
            TEST_VERIFY(fileSize == FULL_SIZE_ON_SERVER);

            bool result = file->Truncate(fileSize / 2);
            TEST_VERIFY(result);

            file->Release();
        }
        task = downloader->ResumeTask(url, p);

        downloader->WaitTask(task);
        downloader->RemoveTask(task);

        crcFromFile = CRC32::ForFile(p);
        TEST_VERIFY(crcFromFile == crc32);

        ////-----multi-files-------------------------------------------------------
        DLCDownloader::Task* taskSize = downloader->StartGetContentSize(url);
        downloader->WaitTask(taskSize);
        uint64 sizeTotal = downloader->GetTaskStatus(taskSize).sizeTotal;

        uint64 firstIndex = 0;
        uint64 nextIndex = 0;
        const uint64 lastIndex = sizeTotal - 1;

        FilePath dir("~doc:/multy_tmp/");
        fs->DeleteDirectory(dir, true);
        fs->CreateDirectory(dir);

        const size_t numAll = 1024;
        const size_t onePart = static_cast<size_t>(sizeTotal / numAll);

        nextIndex += onePart;

        Vector<DLCDownloader::Task*> allTasks;
        allTasks.reserve(numAll);

        start = SystemTimer::GetMs();

        for (size_t i = 0; i < numAll; ++i, firstIndex += onePart, nextIndex += onePart)
        {
            StringStream ss;
            ss << "part_" << std::setw(5) << std::setfill('0') << i << '_' << firstIndex << '-' << nextIndex;
            String fileName = ss.str() + ".part";
            FilePath pathFull = dir + fileName;
            String full = pathFull.GetAbsolutePathname();
            task = downloader->StartTask(url, full, DLCDownloader::Range(firstIndex, nextIndex - firstIndex));
            allTasks.push_back(task);
        }

        for (auto t : allTasks)
        {
            downloader->WaitTask(t);
        }

        finish = SystemTimer::GetMs();

        seconds = (finish - start) / 1000.f;

        Logger::Info("1024 part of %f Gb download from in house server for: %f", sizeInGb, seconds);

        // free memory
        for (auto t : allTasks)
        {
            downloader->RemoveTask(t);
        }
        allTasks.clear();
    }
};