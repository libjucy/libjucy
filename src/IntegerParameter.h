#pragma once

#include "JuceHeaders.h"
#include "Parameter.h"
#include <QtCore>

class IntegerParameter : public Parameter {
    Q_OBJECT
public:
    QString getValueLabel() const override;
    static IntegerParameter *from(juce::AudioProcessorParameter *juceParameter, QObject *parent = nullptr);
private:
    explicit IntegerParameter(juce::AudioProcessorParameter *juceParameter, QObject *parent = nullptr);
    juce::AudioProcessorParameter *m_juceParameter{nullptr};
};
