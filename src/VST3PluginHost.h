#pragma once

#include "PluginHost.h"

class VST3PluginHost : public PluginHost {
public:
    explicit VST3PluginHost(QString pluginIdentifier, QObject *parent = nullptr);
    bool loadPlugin() override;
private:
    juce::AudioPluginFormatManager *m_audioPluginFormatManager{nullptr};
};
