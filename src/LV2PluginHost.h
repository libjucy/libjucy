#pragma once

#include "PluginHost.h"

class LV2PluginHost : public PluginHost {
public:
    explicit LV2PluginHost(QString pluginIdentifier, QObject *parent = nullptr);
    bool loadPlugin() override;
};
