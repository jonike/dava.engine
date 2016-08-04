#if defined(__DAVAENGINE_COREV2__)

#pragma once

#include "Base/BaseTypes.h"

namespace DAVA
{
class Logger;
class FileSystem;
class SystemTimer;
class AllocatorFactory;
class Random;
class PerformanceSettings;
class VersionInfo;

class JobManager;
class LocalizationSystem;
class DownloadManager;

class InputSystem;
class UIControlSystem;
class VirtualCoordinatesSystem;

class SoundSystem;
class AnimationManager;
class FontManager;
class FrameOcclusionQueryManager;
class RenderSystem2D;
class UIScreenManager;
class LocalNotificationController;

namespace Net
{
class NetCore;
}

class EngineContext final
{
public:
    EngineContext();
    ~EngineContext();

    // Subsystems that are created on demand
    JobManager* jobManager = nullptr;
    LocalizationSystem* localizationSystem = nullptr;
    DownloadManager* downloadManager = nullptr;
    Net::NetCore* netCore = nullptr;
    SoundSystem* soundSystem = nullptr;

    // Subsystems that are always created
    Logger* logger = nullptr;
    FileSystem* fileSystem = nullptr;
    SystemTimer* systemTimer = nullptr;
    AllocatorFactory* allocatorFactory = nullptr;
    Random* random = nullptr;
    PerformanceSettings* performanceSettings = nullptr;
    VersionInfo* versionInfo = nullptr;

    InputSystem* inputSystem = nullptr;
    UIControlSystem* uiControlSystem = nullptr;
    VirtualCoordinatesSystem* virtualCoordSystem = nullptr;

    AnimationManager* animationManager = nullptr;
    FontManager* fontManager = nullptr;
    RenderSystem2D* renderSystem2D = nullptr;
    UIScreenManager* uiScreenManager = nullptr;
    LocalNotificationController* localNotificationController = nullptr;
};

} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
