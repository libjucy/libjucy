#pragma once
#define JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED 1

#include "JUCEHeaders.h"
#include <QtCore>

class PluginHostPrivate;

class PluginHost : public QObject {
    Q_OBJECT
public:
    explicit PluginHost(QObject *parent = nullptr);
    ~PluginHost();
private:
    PluginHostPrivate *d{nullptr};
};
