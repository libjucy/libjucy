#pragma once

#include "JuceHeaders.h"
#include <QtCore>

class PluginHostPrivate;

class PluginHost : public QObject {
    Q_OBJECT
public:
    explicit PluginHost(QObject *parent = nullptr);
    ~PluginHost();

    Q_INVOKABLE bool loadPlugin(QString pluginIdentifier);
    Q_INVOKABLE QString getPluginName();
    Q_INVOKABLE QString getPluginIdentifier();
    Q_INVOKABLE QStringList listPluginParameters();
private:
    PluginHostPrivate *d{nullptr};
};
