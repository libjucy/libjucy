#pragma once

#include "JuceHeaders.h"
#include "Parameter.h"
#include <QtCore>

class StringParameter : public Parameter {
    Q_OBJECT
public:
    explicit StringParameter(juce::AudioProcessorParameter *juceParameter, QObject *parent = nullptr);
    QString getValueString();
    QList<float> getAllValues();
    QStringList getAllValueStrings();
    bool isBoolean() override;
    bool isString() override;
private:
    juce::AudioProcessorParameter *m_juceParameter{nullptr};
};
