#include "Parameter.h"

Parameter::Parameter(juce::AudioProcessorParameter *juceParameter, QObject *parent)
    : QObject(parent)
    , m_juceParameter(juceParameter)
{}

QString Parameter::getName()
{
    QString result = "";
    if (m_juceParameter != nullptr) {
        result = QString::fromStdString(m_juceParameter->getName(INT_MAX).toStdString());
    }
    return result;
}

float Parameter::getValue()
{
    float result = -1.0f;
    if (m_juceParameter != nullptr) {
        result = m_juceParameter->getValue();
    }
    return result;
}

void Parameter::setValue(float value)
{
    if (m_juceParameter != nullptr) {
        m_juceParameter->setValue(value);
    }
}

int Parameter::numSteps()
{
    int result = 0;
    if (m_juceParameter != nullptr) {
        result = m_juceParameter->getNumSteps();
    }
    return result;
}

bool Parameter::isBoolean()
{
    return false;
}

bool Parameter::isString()
{
    return false;
}

void Parameter::increase()
{
    if (m_juceParameter != nullptr) {
        m_juceParameter->setValue(qBound(0.0f, getValue() + 1.0f/numSteps(), 1.0f));
    }
}

void Parameter::decrease()
{
    if (m_juceParameter != nullptr) {
        m_juceParameter->setValue(qBound(0.0f, getValue() - 1.0f/numSteps(), 1.0f));
    }
}
