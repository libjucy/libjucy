#pragma once

#include "JuceHeaders.h"
#include "JuceEventLoop.h"
#include <QtCore>

class PluginHostPrivate;

class PluginHost : public QObject {
    Q_OBJECT
public:
    explicit PluginHost(QString pluginIdentifier, QObject *parent = nullptr);
    ~PluginHost();

    Q_INVOKABLE bool loadPlugin(juce::AudioPluginFormatManager *pluginFormatManager);
    Q_INVOKABLE bool unloadPlugin();
    Q_INVOKABLE QString getPluginName();
    Q_INVOKABLE QString getPluginIdentifier();
    Q_INVOKABLE QStringList getAllParameterNames();
    Q_INVOKABLE QString getParameterValueByIndex(int parameterIndex);
    Q_INVOKABLE QString getParameterValueByName(QString parameterName);
    Q_INVOKABLE void setParameterValueRaw(int parameterIndex, float value);
    Q_INVOKABLE QStringList getAllPresetNames();
    Q_INVOKABLE int getCurrentPresetIndex();
    Q_INVOKABLE QString getCurrentPresetName();
    Q_INVOKABLE bool setCurrentPresetByIndex(int presetIndex);
    Q_INVOKABLE bool setCurrentPresetByName(QString presetName);

    Q_INVOKABLE virtual bool loadPlugin() = 0;
private:
    PluginHostPrivate *d{nullptr};
};
