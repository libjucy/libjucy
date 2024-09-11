#include "LV2PluginHost.h"

LV2PluginHost::LV2PluginHost(QString pluginIdentifier, QObject *parent)
    : PluginHost(pluginIdentifier, parent)
{}

bool LV2PluginHost::loadPlugin()
{
    juce::LV2PluginFormat *lv2PluginFormat = new juce::LV2PluginFormat();
    const char *LV2_PATH = std::getenv("LV2_PATH");
    if (LV2_PATH != nullptr) {
        lv2PluginFormat->searchPathsForPlugins(juce::FileSearchPath(juce::String(LV2_PATH)), false);
    }
    juce::AudioPluginFormatManager *pluginFormatManager = new juce::AudioPluginFormatManager();
    pluginFormatManager->addFormat(lv2PluginFormat);
    return PluginHost::loadPlugin(pluginFormatManager);
}
