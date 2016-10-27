#include "SpritesPacker/SpritesPackerModule.h"

#include "Functional/Function.h"

#include "AssetCache/AssetCacheClient.h"
#include "QtTools/ReloadSprites/DialogReloadSprites.h"
#include "QtTools/ReloadSprites/SpritesPacker.h"

#include "Main/mainwindow.h"
#include "Project/ProjectManager.h"

#include <QAction>
#include <QDir>

SpritesPackerModule::SpritesPackerModule(const std::shared_ptr<GlobalOperations>& globalOperations_)
    : QObject(nullptr)
    , spritesPacker(new SpritesPacker())
    , globalOperations(globalOperations_)
{
    qRegisterMetaType<DAVA::eGPUFamily>("DAVA::eGPUFamily");
    qRegisterMetaType<DAVA::TextureConverter::eConvertQuality>("DAVA::TextureConverter::eConvertQuality");
}

SpritesPackerModule::~SpritesPackerModule()
{
    spritesPacker->Cancel();
    spritesPacker->ClearTasks();

    if (cacheClient != nullptr)
    {
        DisconnectCacheClient();
    }

    DAVA::JobManager::Instance()->WaitWorkerJobs();
}

QAction* SpritesPackerModule::GetReloadAction() const
{
    return reloadSpritesAction;
}

void SpritesPackerModule::SetAction(QAction* reloadSpritesAction_)
{
    if (reloadSpritesAction != nullptr)
    {
        disconnect(reloadSpritesAction, &QAction::triggered, this, &SpritesPackerModule::RepackWithDialog);
    }

    reloadSpritesAction = reloadSpritesAction_;

    if (reloadSpritesAction != nullptr)
    {
        connect(reloadSpritesAction, &QAction::triggered, this, &SpritesPackerModule::RepackWithDialog);
    }
}

void SpritesPackerModule::RepackWithDialog()
{
    SetupSpritesPacker(ProjectManager::Instance()->GetProjectPath());

    DAVA::JobManager::Instance()->CreateWorkerJob(DAVA::MakeFunction(this, &SpritesPackerModule::ConnectCacheClient));

    ShowPackerDialog();

    DisconnectCacheClient();

    ReloadObjects();
}

void SpritesPackerModule::RepackImmediately(const DAVA::FilePath& projectPath, DAVA::eGPUFamily gpu)
{
    SetupSpritesPacker(projectPath);

    DAVA::Function<void()> fn = DAVA::Bind(&SpritesPackerModule::ProcessSilentPacking, this, true, false, gpu, DAVA::TextureConverter::ECQ_DEFAULT);
    DAVA::JobManager::Instance()->CreateWorkerJob(fn);

    CreateWaitDialog(projectPath);
}

void SpritesPackerModule::SetupSpritesPacker(const DAVA::FilePath& projectPath)
{
    DAVA::FilePath inputDir = projectPath + "/DataSource/Gfx/Particles";
    DAVA::FilePath outputDir = projectPath + "/Data/Gfx/Particles";

    spritesPacker->ClearTasks();
    spritesPacker->AddTask(QString::fromStdString(inputDir.GetAbsolutePathname()), QString::fromStdString(outputDir.GetAbsolutePathname()));
}

void SpritesPackerModule::ProcessSilentPacking(bool clearDirs, bool forceRepack, const DAVA::eGPUFamily gpu, const DAVA::TextureConverter::eConvertQuality quality)
{
    ConnectCacheClient();
    spritesPacker->ReloadSprites(clearDirs, forceRepack, gpu, quality);
    DisconnectCacheClient();

    DAVA::JobManager::Instance()->CreateMainJob(DAVA::MakeFunction(this, &SpritesPackerModule::CloseWaitDialog));
    DAVA::JobManager::Instance()->CreateMainJob(DAVA::MakeFunction(this, &SpritesPackerModule::ReloadObjects));
}

void SpritesPackerModule::ShowPackerDialog()
{
    DialogReloadSprites dialogReloadSprites(spritesPacker.get(), globalOperations->GetGlobalParentWidget());
    dialogReloadSprites.exec();
}

void SpritesPackerModule::CreateWaitDialog(const DAVA::FilePath& projectPath)
{
    globalOperations->ShowWaitDialog("Sprites Reloading", DAVA::String("Reloading Particles for ") + projectPath.GetAbsolutePathname());
}

void SpritesPackerModule::CloseWaitDialog()
{
    globalOperations->HideWaitDialog();
}

void SpritesPackerModule::ReloadObjects()
{
    DAVA::Sprite::ReloadSprites();

    const DAVA::Vector<DAVA::eGPUFamily>& gpus = spritesPacker->GetResourcePacker().requestedGPUs;
    if (gpus.empty() == false)
    {
        DAVA::uint32 gpu = gpus[0];
        SettingsManager::SetValue(Settings::Internal_SpriteViewGPU, DAVA::VariantType(gpu));
    }

    emit SpritesReloaded();
}

void SpritesPackerModule::ConnectCacheClient()
{
    DVASSERT(cacheClient == nullptr);
    if (SettingsManager::GetValue(Settings::General_AssetCache_UseCache).AsBool())
    {
        DAVA::String ipStr = SettingsManager::GetValue(Settings::General_AssetCache_Ip).AsString();
        DAVA::uint16 port = static_cast<DAVA::uint16>(SettingsManager::GetValue(Settings::General_AssetCache_Port).AsUInt32());
        DAVA::uint64 timeoutSec = SettingsManager::GetValue(Settings::General_AssetCache_Timeout).AsUInt32();

        DAVA::AssetCacheClient::ConnectionParams params;
        params.ip = (ipStr.empty() ? DAVA::AssetCache::GetLocalHost() : ipStr);
        params.port = port;
        params.timeoutms = timeoutSec * 1000; //in ms

        cacheClient = new DAVA::AssetCacheClient();
        DAVA::AssetCache::Error connected = cacheClient->ConnectSynchronously(params);
        if (connected != DAVA::AssetCache::Error::NO_ERRORS)
        {
            DisconnectCacheClient();
        }
    }

    SetCacheClientForPacker();
}

void SpritesPackerModule::DisconnectCacheClient()
{
    if (cacheClient != nullptr)
    {
        DAVA::AssetCacheClient* disconnectingClient = cacheClient;
        cacheClient = nullptr;
        SetCacheClientForPacker();

        //we should destroy cache client on main thread
        DAVA::JobManager::Instance()->CreateMainJob(DAVA::Bind(&SpritesPackerModule::DisconnectCacheClientInternal, this, disconnectingClient));
    }
}

void SpritesPackerModule::SetCacheClientForPacker()
{
    spritesPacker->SetCacheClient(cacheClient, "ResourceEditor.ReloadParticles");
}

void SpritesPackerModule::DisconnectCacheClientInternal(DAVA::AssetCacheClient* cacheClientForDisconnect)
{
    DVASSERT(cacheClientForDisconnect != nullptr);

    cacheClientForDisconnect->Disconnect();
    SafeDelete(cacheClientForDisconnect);
}

bool SpritesPackerModule::IsRunning() const
{
    return spritesPacker->IsRunning();
}
