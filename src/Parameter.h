#pragma once

#include "JuceHeaders.h"
#include <QtCore>

class Parameter : public QObject {
public:
    explicit Parameter(juce::AudioProcessorParameter *juceParameter, QObject *parent = nullptr);
    QString getParameterID() const;
    QString getName() const;
    float getDefaultValue() const;
    float getValue() const;
    void setValue(float value);
    QString getValueLabel() const;
    int numSteps() const;
    virtual bool isBoolean();
    virtual bool isString();
    virtual void increase();
    virtual void decrease();
    bool isBypassParameter() const;
    bool isProgramParameter() const;
private:
    friend class PluginHost;
    bool m_isBypassParameter{false};
    bool m_isProgramParameter{false};
    juce::AudioProcessorParameter *m_juceParameter{nullptr};
};
