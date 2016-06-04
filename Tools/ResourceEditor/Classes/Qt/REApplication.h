#pragma once

#include "NgtTools/Application/NGTApplication.h"

class QtMainWindow;
class NGTCommand;
namespace wgt
{
class ICommandManager;
}

class REApplication : public NGTLayer::BaseApplication
{
public:
    REApplication(int argc, char** argv);
    ~REApplication();

    void Run();

protected:
    void GetPluginsForLoad(DAVA::Vector<DAVA::WideString>& names) const override;
    void OnPostLoadPugins() override;
    void OnPreUnloadPlugins() override;

private:
    wgt::ICommandManager* commandManager = nullptr;
    std::unique_ptr<NGTCommand> ngtCommand;
    QtMainWindow* mainWindow = nullptr;
};
