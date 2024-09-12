#include "BooleanParameter.h"

BooleanParameter::BooleanParameter(juce::AudioProcessorParameter *juceParameter, QObject *parent)
    : Parameter(juceParameter, parent)
    , m_juceParameter(juceParameter)
{}

bool BooleanParameter::isBoolean()
{
    return true;
}

bool BooleanParameter::isString()
{
    return false;
}

void BooleanParameter::increase()
{
    m_juceParameter->setValue(1.0);
}

void BooleanParameter::decrease()
{
    m_juceParameter->setValue(0.0);
}
