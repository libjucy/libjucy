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
    Q_INVOKABLE QStringList getAllProgramNames();
    Q_INVOKABLE QString getCurrentProgramName();
    Q_INVOKABLE int getCurrentProgramIndex();
    Q_INVOKABLE bool setCurrentProgramIndex(int programIndex);
private:
    PluginHostPrivate *d{nullptr};
};
