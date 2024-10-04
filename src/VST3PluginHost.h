#pragma once

#include "PluginHost.h"

class VST3PluginHost : public PluginHost {
public:
    explicit VST3PluginHost(QString pluginIdentifier, QString jackClientName, QObject *parent = nullptr);
    bool loadPlugin() override;
    QList<PluginDescription*> getAllPlugins() override;
private:
    juce::AudioPluginFormatManager *m_audioPluginFormatManager{nullptr};
};
