#pragma once

#include "JuceHeaders.h"
#include "Parameter.h"
#include <QtCore>

class BooleanParameter : public Parameter {
    Q_OBJECT
public:
    void increase() override;
    void decrease() override;
    static BooleanParameter *from(juce::AudioProcessorParameter *juceParameter, QObject *parent = nullptr);
private:
    explicit BooleanParameter(juce::AudioProcessorParameter *juceParameter, QObject *parent = nullptr);
    juce::AudioProcessorParameter *m_juceParameter{nullptr};
};
