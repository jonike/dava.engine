#pragma once

#include "NGTCmdLineParser.h"

#include "Base/BaseTypes.h"

#include <core_generic_plugin_manager/generic_plugin_manager.hpp>
#include <core_generic_plugin/interfaces/i_component_context.hpp>

class QMainWindow;
namespace NGTLayer
{
class BaseApplication
{
public:
    BaseApplication(int argc, char** argv);
    virtual ~BaseApplication();

    void LoadPlugins();
    IComponentContext& GetComponentContext();
    int StartApplication(QMainWindow* appMainWindow);

protected:
    virtual void GetPluginsForLoad(DAVA::Vector<DAVA::WideString>& names) const = 0;
    virtual void OnPostLoadPugins()
    {
    }
    virtual void OnPreUnloadPlugins()
    {
    }

private:
    DAVA::WideString GetPluginsFolder() const;

private:
    GenericPluginManager pluginManager;
    NGTCmdLineParser commandLineParser;
};
} // namespace NGTLayer