#pragma once

#include "JuceHeaders.h"
#include "Parameter.h"
#include <QtCore>

class StringListParameter : public Parameter {
    Q_OBJECT
public:
    QString getValueString();
    QList<float> getAllValues();
    QStringList getAllValueStrings();
    static StringListParameter *from(juce::AudioProcessorParameter *juceParameter, QObject *parent = nullptr);
private:
    explicit StringListParameter(juce::AudioProcessorParameter *juceParameter, QObject *parent = nullptr);
    juce::AudioProcessorParameter *m_juceParameter{nullptr};    
};
