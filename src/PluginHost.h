#pragma once

#include "JuceHeaders.h"
#include "JuceEventLoop.h"
#include "Parameter.h"
#include "PluginDescription.h"
#include <QtCore>

class PluginHostPrivate;

class PluginHost : public QObject {
    Q_OBJECT
public:
    explicit PluginHost(QString pluginIdentifier, QString jackClientName, QObject *parent = nullptr);
    ~PluginHost();

    Q_INVOKABLE bool loadPlugin(juce::AudioPluginFormatManager *pluginFormatManager);
    Q_INVOKABLE bool unloadPlugin();
    Q_INVOKABLE QString getPluginName();
    Q_INVOKABLE QString getPluginIdentifier();
    Q_INVOKABLE Parameter* getParameter(QString parameterName);
    Q_INVOKABLE QList<Parameter *> getAllParameters();
    Q_INVOKABLE QStringList getAllPresets();
    Q_INVOKABLE QString getCurrentPreset();
    Q_INVOKABLE bool setCurrentPreset(QString presetName);

    Q_INVOKABLE virtual bool loadPlugin() = 0;
    Q_INVOKABLE virtual QList<PluginDescription*> getAllPlugins() = 0;
private:
    PluginHostPrivate *d{nullptr};
};
