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

BooleanParameter *BooleanParameter::from(juce::AudioProcessorParameter *juceParameter, QObject *parent)
{
    BooleanParameter *parameter{nullptr};
    // Heuristics to check if a juce parameter needs to be treated as a Boolean parameter
    if (juceParameter->getNumSteps() == 2) {
        parameter = new BooleanParameter(juceParameter, parent);
    }
    return parameter;
}
