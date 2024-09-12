#pragma once

#include "JuceHeaders.h"
#include <QtCore>

class Parameter : public QObject {
public:
    explicit Parameter(juce::AudioProcessorParameter *juceParameter, QObject *parent = nullptr);
    QString getName();
    float getValue();
    void setValue(float value);
    int numSteps();
    virtual bool isBoolean();
    virtual bool isString();
    virtual void increase();
    virtual void decrease();
private:
    juce::AudioProcessorParameter *m_juceParameter{nullptr};
};
