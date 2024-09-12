#include "StringParameter.h"

StringParameter::StringParameter(juce::AudioProcessorParameter *juceParameter, QObject *parent)
    : Parameter(juceParameter, parent)
    , m_juceParameter(juceParameter)
{}

QString StringParameter::getValueString()
{
    QString result = "";
    if (m_juceParameter != nullptr) {
        result = QString::fromStdString(m_juceParameter->getText(getValue(), INT_MAX).toStdString());
    }
    return result;
}

QStringList StringParameter::getAllValueStrings()
{
    QStringList result;
    if (m_juceParameter != nullptr) {
        for (juce::String value : m_juceParameter->getAllValueStrings()) {
            result << QString::fromStdString(value.toStdString());
        }
    }
    return result;
}

bool StringParameter::isBoolean()
{
    return false;
}

bool StringParameter::isString()
{
    return true;
}
