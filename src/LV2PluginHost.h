#pragma once

#include "PluginHost.h"

class LV2PluginHost : public PluginHost {
public:
    explicit LV2PluginHost(QString pluginIdentifier, QString jackClientName, QObject *parent = nullptr);
    bool loadPlugin() override;
    QList<PluginDescription*> getAllPlugins() override;
private:
    juce::AudioPluginFormatManager *m_audioPluginFormatManager{nullptr};
};
