/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "Base/BaseTypes.h"
#include "Base/FunctionTraits.h"
#include "GameCore.h"

#include "Platform/DateTime.h"
#include "TexturePacker/CommandLineParser.h"
#include "Utils/Utils.h"

#include "MemoryManager/MemoryManager.h"

#include "Tests/MathTest.h"
#include "Tests/FunctionBindSingalTest.h"
#include "Tests/ImageSizeTest.h"
#include "Tests/SaveImageTest.h"
#include "Tests/StringFormatTest.h"
#include "Tests/ComponentsTest.h"
#include "Tests/FileListTest.h"
#include "Tests/FileSystemTest.h"
#include "Tests/DateTimeTest.h"
#include "Tests/LocalizationTest.h"
#include "Tests/MemoryAllocatorsTest.h"
#include "Tests/HashMapTest.h"
#include "Tests/SplitTest.h"
#include "Tests/TextSizeTest.h"
#include "Tests/KeyedArchiveYamlTest.h"
#include "Tests/JobManagerTest.h"
#include "Tests/Cpp14.h"
#include "Tests/NetworkTest.h"
#include "Tests/JNITest.h"
#include "Tests/DataVaultTest.h"
#include "Tests/UnlimitedLogOutputTest.h"
#include "Tests/SpinLockTest.h"
#include "Tests/ThreadSyncTest.h"
//$UNITTEST_INCLUDE

void GameCore::RunOnlyThisTest()
{
    //runOnlyThisTest = "TestClassName";
    runOnlyThisTest = "NetworkTest";
}

void GameCore::OnError()
{
    DavaDebugBreak();
}

void GameCore::RegisterTests()
{
	/*
    new ThreadSyncTest();
    new DataVaultTest();
#if defined(__DAVAENGINE_ANDROID__)
    new JNITest();
#endif
    new MathTest();
    new FunctionBindSignalTest();
    new ImageSizeTest();
    new SaveImageTest();
    new StringFormatTest();
    new ComponentsTest();
    new FileListTest();
    new FileSystemTest();
    new DateTimeTest();
    new LocalizationTest();
    new MemoryAllocatorsTest();
    new HashMapTest();
    new SplitTest();
    new TextSizeTest();
    new KeyedArchiveYamlTest();
    new JobManagerTest();
    new Cpp14Test ();
    */
    new NetworkTest();
    new UnlimitedLogOutputTest();
    new SpinLockTest();
    //$UNITTEST_CTOR
}

#include <fstream>
#include <algorithm>

using namespace DAVA;

void GameCore::OnAppStarted()
{
	Logger::Debug("GameCore::OnAppStarted");
    
    InitLogging();
    InitNetwork();
    RunOnlyThisTest();
    RegisterTests();
    RunTests();
}

GameCore::GameCore() 
    : currentScreen(NULL)
    , currentScreenIndex(0)
    , currentTestIndex(0)
    , netLogger(true)
    , loggerInUse(false)
    , mmInUse(false)
{
    MemoryManager::RegisterAllocPoolName(3, "STL");
    MemoryManager::RegisterAllocPoolName(4, "Custom");
    MemoryManager::RegisterTagName(1, "TAG_1");
    MemoryManager::RegisterTagName(2, "TAG_2");
}

GameCore::~GameCore()
{
}

void GameCore::RegisterScreen(BaseScreen *screen)
{
    UIScreenManager::Instance()->RegisterScreen(screen->GetScreenId(), screen);
    screens.push_back(screen);
}


void GameCore::CreateDocumentsFolder()
{
    FilePath documentsPath = FileSystem::Instance()->GetUserDocumentsPath() + "UnitTests/";
    
    FileSystem::Instance()->CreateDirectory(documentsPath, true);
    FileSystem::Instance()->SetCurrentDocumentsDirectory(documentsPath);
}


File * GameCore::CreateDocumentsFile(const String &filePathname)
{
    FilePath workingFilepathname = FilePath::FilepathInDocuments(filePathname);

    FileSystem::Instance()->CreateDirectory(workingFilepathname.GetDirectory(), true);
    
    File *retFile = File::Create(workingFilepathname, File::CREATE | File::WRITE);
    return retFile;
}

void GameCore::OnAppFinished()
{
	Logger::Debug("GameCore::OnAppFinished");
    DAVA::Logger::Instance()->RemoveCustomOutput(&teamCityOutput);

    int32 screensSize = screens.size();
    for(int32 i = 0; i < screensSize; ++i)
    {
        SafeRelease(screens[i]);
    }
    screens.clear();
    
    netLogger.Uninstall();
/*
#if defined(__DAVAENGINE_WIN32__)
    const char8* fname = "c:\\projects\\unittest-memprof.log";
#elif defined(__DAVAENGINE_ANDROID__)
    const char8* fname = "/sdcard/unittest-memprof.log";
#elif defined(__DAVAENGINE_MACOS__)
    const char8* fname = "/Users/max/projects/unittest-memprof.log";
#endif
    FILE* file = fopen(fname, "wb");
    if (file)
    {
        MEMPROF_DUMP(file);
        fclose(file);
    }
*/
}

void GameCore::OnSuspend()
{
//    Logger::Debug("GameCore::OnSuspend");
#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
    ApplicationCore::OnSuspend();
#endif

}

void GameCore::OnResume()
{
    Logger::Debug("GameCore::OnResume");
    ApplicationCore::OnResume();
}


#if defined (__DAVAENGINE_IPHONE__) || defined (__DAVAENGINE_ANDROID__)
void GameCore::OnDeviceLocked()
{
//    Logger::Debug("GameCore::OnDeviceLocked");
    //Core::Instance()->Quit();
}

void GameCore::OnBackground()
{
    Logger::Debug("GameCore::OnBackground");
}

void GameCore::OnForeground()
{
    Logger::Debug("GameCore::OnForeground");
    ApplicationCore::OnForeground();
}

#endif //#if defined (__DAVAENGINE_IPHONE__) || defined (__DAVAENGINE_ANDROID__)


void GameCore::BeginFrame()
{
    ApplicationCore::BeginFrame();
    RenderManager::Instance()->ClearWithColor(0.f, 0.f, 0.f, 0.f);
}

void GameCore::Update(float32 timeElapsed)
{
    ProcessTests();
    ApplicationCore::Update(timeElapsed);
    netMM.Update(timeElapsed);
}

void GameCore::Draw()
{
    ApplicationCore::Draw();
}

void GameCore::RunTests()
{
    currentTestIndex = 0;
    int32 screensSize = screens.size();
    for(int32 iScr = 0; iScr < screensSize; ++iScr)
    {
        BaseScreen& screen = *screens[iScr];
        if (IsNeedSkipTest(screen))
        {
            continue;
        }
        int32 count = screen.GetTestCount();
        if(0 < count)
        {
            currentScreen = screens[iScr];
            currentScreenIndex = iScr;
            break;
        }
    }
    
    if(currentScreen)
    {
        Logger::Info(TeamcityTestsOutput::FormatTestStarted(currentScreen->GetTestName()).c_str());

        UIScreenManager::Instance()->SetFirst(currentScreen->GetScreenId());
    }
    else 
    {
        LogMessage(String("There are no tests."));
        Core::Instance()->Quit();
    }
}


void GameCore::FinishTests()
{
    // inform teamcity script we just finished all tests
    // useful on ios devices (run with lldb)
    teamCityOutput.Output(DAVA::Logger::LEVEL_DEBUG, "Finish all tests.");
    Core::Instance()->Quit();
}

void GameCore::LogMessage(const String &message)
{
    DAVA::Logger::Error(message.c_str());
}

int32 GameCore::TestCount()
{
    int32 count = 0;
    int32 screensSize = screens.size();
    for(int32 i = 0; i < screensSize; ++i)
    {
        count += screens[i]->GetTestCount();
    }
    
    return count;
}

void GameCore::ProcessTests()
{
    if(currentScreen && currentScreen->ReadyForTests())
    {
        bool ret = currentScreen->RunTest(currentTestIndex);
        if(ret)
        {
            ++currentTestIndex;
            if(currentScreen->GetTestCount() == currentTestIndex)
            {
                ++currentScreenIndex;
                if(currentScreenIndex == screens.size())
                {
                    Logger::Info(TeamcityTestsOutput::FormatTestFinished(currentScreen->GetTestName()).c_str());
                    FinishTests();
                }
                else 
                {
                    Logger::Info(TeamcityTestsOutput::FormatTestFinished(currentScreen->GetTestName()).c_str());

                    currentScreen = screens[currentScreenIndex];

                    while (IsNeedSkipTest(*currentScreen))
                    {
                        ++currentScreenIndex;
                        if (currentScreenIndex == screens.size())
                        {
                            FinishTests();
                            return;
                        }
                        currentScreen = screens[currentScreenIndex];
                    }

                    currentTestIndex = 0;

                    Logger::Info(TeamcityTestsOutput::FormatTestStarted(currentScreen->GetTestName()).c_str());

                    UIScreenManager::Instance()->SetScreen(currentScreen->GetScreenId());
                }
            }
        }
    }
}

void GameCore::RegisterError(const String &command, const String &fileName, int32 line, TestData *testData)
{
    OnError();

    const char* testName = currentScreen->GetTestName().c_str();

    String errorString = String(Format("%s(%d): ",
        fileName.c_str(), line));

    if (testData)
    {
        if(!testData->name.empty())
        {
            errorString += String(Format(" %s", testData->name.c_str())); // test function name
        }

        if(!testData->message.empty())
        {
            errorString += String(Format(" %s", testData->message.c_str()));
        }
    }
    LogMessage(TeamcityTestsOutput::FormatTestFailed(testName, command, errorString));
}

DAVA::String GameCore::CreateOutputLogFile()
{
    time_t logStartTime = time(0);
    const String logFileName = Format("Reports/%lld.errorlog", logStartTime);
    File* logFile = CreateDocumentsFile(logFileName);
    DVASSERT(logFile);
    SafeRelease(logFile);

    FilePath workingFilepathname = FilePath::FilepathInDocuments(logFileName);
    return workingFilepathname.GetAbsolutePathname();
}

void GameCore::InitLogging()
{
    if (CommandLineParser::Instance()->CommandIsFound("-only_test"))
    {
        runOnlyThisTest = CommandLineParser::Instance()->GetCommandParam("-only_test");
    }

    Logger::Instance()->AddCustomOutput(&teamCityOutput);
}

bool GameCore::IsNeedSkipTest(const BaseScreen& screen) const
{
    if (runOnlyThisTest.empty())
    {
        return false;
    }

    const String& name = screen.GetTestName();

    return 0 != CompareCaseInsensitive(runOnlyThisTest, name);
}

const char8 GameCore::announceMulticastGroup[] = "239.192.100.1";

void GameCore::InitNetwork()
{
    using namespace DAVA::Net;
    
    NetCore::Instance()->RegisterService(SERVICE_LOG, MakeFunction(this, &GameCore::CreateLogger), MakeFunction(this, &GameCore::DeleteLogger));
    NetCore::Instance()->RegisterService(SERVICE_MEMPROF, MakeFunction(this, &GameCore::CreateMMServer), MakeFunction(this, &GameCore::DeleteMMServer));
    
    NetConfig config(SERVER_ROLE);
    config.AddTransport(TRANSPORT_TCP, Endpoint(9999));
    config.AddService(SERVICE_LOG);
    config.AddService(SERVICE_MEMPROF);
    
    peerDescr = PeerDescription(config);
    
    Endpoint annoEndpoint(announceMulticastGroup, ANNOUNCE_PORT);
    NetCore::Instance()->CreateAnnouncer(annoEndpoint, ANNOUNCE_TIME_PERIOD, MakeFunction(this, &GameCore::AnnounceDataSupplier));
    NetCore::Instance()->CreateController(config, NULL);
}

size_t GameCore::AnnounceDataSupplier(size_t length, void* buffer)
{
    using namespace DAVA::Net;
    if (true == peerDescr.NetworkInterfaces().empty())
    {
        // Get list of available network interfaces
        // If list is empty then network is unavailable, this can happen on Android, maybe on iOS
        // It's strange if no network but AnnounceDataSupplier has been invoked
        peerDescr.SetNetworkInterfaces(NetCore::Instance()->InstalledInterfaces());
        if (true == peerDescr.NetworkInterfaces().empty())
            return 0;
    }
    return peerDescr.Serialize(buffer, length);
}

Net::IChannelListener* GameCore::CreateLogger(uint32 serviceId, void* context)
{
    if (!loggerInUse)
    {
        loggerInUse = true;
        return &netLogger;
    }
    return nullptr;
}

void GameCore::DeleteLogger(Net::IChannelListener* obj, void* context)
{
    loggerInUse = false;
}

Net::IChannelListener* GameCore::CreateMMServer(uint32 serviceId, void* context)
{
    if (!mmInUse)
    {
        mmInUse = true;
        return &netMM;
    }
    return nullptr;
}

void GameCore::DeleteMMServer(Net::IChannelListener* obj, void* context)
{
    mmInUse = false;
}
