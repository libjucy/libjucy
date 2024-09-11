#include "LV2PluginHost.h"

LV2PluginHost::LV2PluginHost(QString pluginIdentifier, QObject *parent)
    : PluginHost(pluginIdentifier, parent)
    , m_audioPluginFormatManager(new juce::AudioPluginFormatManager())
{
    juce::LV2PluginFormat *lv2PluginFormat = new juce::LV2PluginFormat();
    const char *LV2_PATH = std::getenv("LV2_PATH");
    if (LV2_PATH != nullptr) {
        lv2PluginFormat->searchPathsForPlugins(juce::FileSearchPath(juce::String(LV2_PATH)), false);
    }
    m_audioPluginFormatManager->addFormat(lv2PluginFormat);
}

bool LV2PluginHost::loadPlugin()
{
    return PluginHost::loadPlugin(m_audioPluginFormatManager);
}
