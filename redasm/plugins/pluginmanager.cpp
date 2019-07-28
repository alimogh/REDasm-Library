#include "pluginmanager.h"
#include <impl/plugins/pluginmanager_impl.h>
#include <impl/plugins/pluginloader.h>
#include <redasm/plugins/assembler/assembler.h>
#include <redasm/plugins/loader/loader.h>
#include <redasm/plugins/plugin.h>
#include <redasm/support/path.h>
#include <redasm/context.h>

namespace REDasm {

PluginManager::PluginManager(): m_pimpl_p(new PluginManagerImpl()) { }
PluginManager::~PluginManager() { this->unloadAll(); }
void PluginManager::unloadAll() { PIMPL_P(PluginManager); p->unloadAll(); }
void PluginManager::unload(const PluginInstance *pi) { PIMPL_P(PluginManager); p->unload(pi); }

void PluginManager::unload(const PluginList &pl)
{
    for(const PluginInstance* pi : pl)
        this->unload(pi);
}

PluginManager *PluginManager::instance()
{
    static PluginManager instance;
    return &instance;
}

const PluginManager::PluginMap &PluginManager::activePlugins() const { PIMPL_P(const PluginManager); return p->m_activeplugins; }
const PluginInstance *PluginManager::findLoader(const String &id) { PIMPL_P(PluginManager); return p->find(id, REDASM_INIT_LOADER_NAME); }
const PluginInstance *PluginManager::findAssembler(const String &id) { PIMPL_P(PluginManager); return p->find(id, REDASM_INIT_ASSEMBLER_NAME); }
const PluginInstance *PluginManager::findPlugin(const String &id) { PIMPL_P(PluginManager); return p->find(id, REDASM_INIT_PLUGIN_NAME); }

PluginManager::PluginList PluginManager::getLoaders(const LoadRequest& request)
{
    PIMPL_P(PluginManager);
    PluginList plugins;

    p->iteratePlugins(REDASM_INIT_LOADER_NAME, [&plugins, request](const PluginInstance* pi) -> PluginManagerImpl::IterateResult {
        Loader* loader = plugin_cast<Loader>(pi);
        bool res = loader->test(request);

        if(res) {
            loader->init(request);
            plugins.push_back(pi);
            return PluginManagerImpl::IterateResult::Continue;
        }

        return PluginManagerImpl::IterateResult::Unload;
    });

    // Sort by weight
    std::sort(plugins.begin(), plugins.end(), [](const PluginInstance* pi1, const PluginInstance* pi2) -> bool {
        return pi1->descriptor->plugin->weight() < pi2->descriptor->plugin->weight();
    });

    return plugins;
}

PluginManager::PluginList PluginManager::getAssemblers()
{
    PIMPL_P(PluginManager);
    PluginList plugins;

    p->iteratePlugins(REDASM_INIT_ASSEMBLER_NAME, [&plugins](const PluginInstance* pi) -> PluginManagerImpl::IterateResult {
        plugins.push_back(pi);
        return PluginManagerImpl::IterateResult::Continue;
    });

    // Sort alphabectically
    std::sort(plugins.begin(), plugins.end(), [](const PluginInstance* pi1, const PluginInstance* pi2) -> bool {
        return String(pi1->descriptor->description) < String(pi2->descriptor->description);
    });

    return plugins;
}

bool PluginManager::execute(const String &id, const ArgumentList &args) { PIMPL_P(PluginManager); return p->execute(id, args); }
bool PluginManager::execute(const String &id) { return this->execute(id, { }); }

} // namespace REDasm