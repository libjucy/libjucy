#pragma once

#include "JuceHeaders.h"
#include "Parameter.h"
#include <QtCore>

class BooleanParameter : public Parameter {
    Q_OBJECT
public:
    explicit BooleanParameter(juce::AudioProcessorParameter *juceParameter, QObject *parent = nullptr);
    bool isBoolean() override;
    bool isString() override;
    void increase() override;
    void decrease() override;
private:
    juce::AudioProcessorParameter *m_juceParameter{nullptr};
};
