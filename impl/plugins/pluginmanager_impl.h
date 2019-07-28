#pragma once

#include <functional>
#include <redasm/pimpl.h>
#include <redasm/plugins/pluginmanager.h>

namespace REDasm {

class PluginManagerImpl
{
    PIMPL_DECLARE_Q(PluginManager)
    PIMPL_DECLARE_PUBLIC(PluginManager)

    public:
        enum class IterateResult { Done = 0, Continue, Unload };
        typedef std::function<IterateResult(const PluginInstance*)> PluginManager_Callback;

    public:
        PluginManagerImpl() = default;
        const PluginInstance* load(const String& pluginpath, const String& initname);
        const PluginInstance* find(const String& id, const String& initname);
        void iteratePlugins(const String& initname, const PluginManager_Callback& cb);
        void unload(const PluginInstance* pi);
        void unloadAll();

    public:
        bool execute(const String& id, const ArgumentList& args);

    private:
        bool iteratePlugins(const String& path, const String& initname, const PluginManager_Callback& cb);
        const PluginInstance* find(const String& path, const String& id, const String& initname);

    private:
        PluginManager::PluginMap m_activeplugins;
};

} // namespace REDasm