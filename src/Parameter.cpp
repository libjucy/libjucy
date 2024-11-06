#include "Parameter.h"

Parameter::Parameter(juce::AudioProcessorParameter *juceParameter, QObject *parent)
    : QObject(parent)
    , m_juceParameter(juceParameter)
{}

QString Parameter::getParameterID() const
{
    QString result = "";
    if (m_juceParameter != nullptr) {
        juce::AudioProcessorParameterWithID *parameterWithId = static_cast<juce::AudioProcessorParameterWithID*>(m_juceParameter);
        if (parameterWithId) {
            result = QString::fromStdString(parameterWithId->getParameterID().toStdString());
        }
    }
    return result;
}

QString Parameter::getName() const
{
    QString result = "";
    if (m_juceParameter != nullptr) {
        result = QString::fromStdString(m_juceParameter->getName(INT_MAX).toStdString());
    }
    return result;
}

float Parameter::getDefaultValue() const
{
    float result = 0.0f;
    if (m_juceParameter != nullptr) {
        result = m_juceParameter->getDefaultValue();
    }
    return result;
}

float Parameter::getValue() const
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

QString Parameter::getValueLabel() const
{
    QString result;
    if (m_juceParameter != nullptr) {
        result = QString::fromStdString(m_juceParameter->getCurrentValueAsText().toStdString());
    }
    return result;
}

int Parameter::numSteps() const
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
        m_juceParameter->setValue(qBound(0.0, getValue() + 1.0/(numSteps() - 1), 1.0));
    }
}

void Parameter::decrease()
{
    if (m_juceParameter != nullptr) {
        m_juceParameter->setValue(qBound(0.0, getValue() - 1.0/(numSteps() - 1), 1.0));
    }
}

bool Parameter::isBypassParameter() const
{
    return m_isBypassParameter;
}

bool Parameter::isProgramParameter() const
{
    return m_isProgramParameter;
}
