#include "VST3PluginHost.h"

VST3PluginHost::VST3PluginHost(QString pluginIdentifier, QObject *parent)
    : PluginHost(pluginIdentifier, parent)
{}

bool VST3PluginHost::loadPlugin()
{
    juce::VST3PluginFormat *vst3PluginFormat = new juce::VST3PluginFormat();
    juce::AudioPluginFormatManager *pluginFormatManager = new juce::AudioPluginFormatManager();
    pluginFormatManager->addFormat(vst3PluginFormat);
    return PluginHost::loadPlugin(pluginFormatManager);
}
