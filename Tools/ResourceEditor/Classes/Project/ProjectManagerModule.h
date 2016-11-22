#pragma once

#include "TArc/Core/ClientModule.h"
#include "TArc/Utils/QtConnections.h"

#include "QtTools/Utils/QtDelayedExecutor.h"

class RecentMenuItems;
class ProjectManagerData;
class ProjectManagerModule : public DAVA::TArc::ClientModule
{
protected:
    void PostInit() override;

private:
    void CreateActions();
    void RegisterOperations();

    void OpenProject();
    void OpenProjectByPath(const DAVA::FilePath& incomePath);
    void OpenProjectImpl(const DAVA::FilePath& incomePath);
    void OpenLastProject();
    void CloseProject();
    void ReloadSprites();

private:
    void LoadMaterialsSettings(ProjectManagerData* data);
    ProjectManagerData* GetData();

private:
    std::unique_ptr<RecentMenuItems> recentProject;
    DAVA::TArc::QtConnections connections;
    QtDelayedExecutor delayedExecutor;
};