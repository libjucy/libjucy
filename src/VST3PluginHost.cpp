#include "VST3PluginHost.h"

VST3PluginHost::VST3PluginHost(QString pluginIdentifier, QObject *parent)
    : PluginHost(pluginIdentifier, parent)
    , m_audioPluginFormatManager(new juce::AudioPluginFormatManager())
{
    juce::VST3PluginFormat *vst3PluginFormat = new juce::VST3PluginFormat();
    m_audioPluginFormatManager->addFormat(vst3PluginFormat);
}

bool VST3PluginHost::loadPlugin()
{
    return PluginHost::loadPlugin(m_audioPluginFormatManager);
}
