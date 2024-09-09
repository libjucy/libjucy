#pragma once

#include "JuceHeaders.h"
#include "JuceEventLoop.h"
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
    Q_INVOKABLE QStringList getAllParameterNames();
    Q_INVOKABLE QString getParameterValue(int parameterIndex);
    Q_INVOKABLE QString getParameterValue(QString parameterName);
    Q_INVOKABLE void setParameterValue(int parameterIndex, float value);
    Q_INVOKABLE QStringList getAllProgramNames();
    Q_INVOKABLE QString getCurrentProgramName();
    Q_INVOKABLE int getCurrentProgramIndex();
    Q_INVOKABLE bool setCurrentProgram(int programIndex);
    Q_INVOKABLE bool setCurrentProgram(QString programName);
private:
    PluginHostPrivate *d{nullptr};
};
