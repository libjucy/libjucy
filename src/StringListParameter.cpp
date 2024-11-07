#include "StringListParameter.h"

StringListParameter::StringListParameter(juce::AudioProcessorParameter *juceParameter, QObject *parent)
    : Parameter(juceParameter, parent)
    , m_juceParameter(juceParameter)
{}

QString StringListParameter::getValueString()
{
    QString result = "";
    if (m_juceParameter != nullptr) {
        result = QString::fromStdString(m_juceParameter->getText(getValue(), INT_MAX).toStdString());
    }
    return result;
}

QList<float> StringListParameter::getAllValues()
{
    QList<float> result;

    result << 0.0f;
    for (int i = 0; i < numSteps() - 1; i++) {
        result << result.at(result.size() - 1) + 1.0/(numSteps() - 1);
    }
    return result;
}

QStringList StringListParameter::getAllValueStrings()
{
    QStringList result;
    if (m_juceParameter != nullptr) {
        if (m_juceParameter->getAllValueStrings().size() > 0) {
            for (juce::String value : m_juceParameter->getAllValueStrings()) {
                result << QString::fromStdString(value.toStdString());
            }
        } else {
            for (auto value : getAllValues()) {
                result << QString::fromStdString(m_juceParameter->getText(value, INT_MAX).toStdString());
            }
        }
    }
    return result;
}

bool StringListParameter::isBoolean()
{
    return false;
}

bool StringListParameter::isString()
{
    return true;
}

StringListParameter *StringListParameter::from(juce::AudioProcessorParameter *juceParameter, QObject *parent)
{
    StringListParameter *parameter{nullptr};
    // Heuristics to check if a juce parameter needs to be treated as a string list parameter
    if (juceParameter->getAllValueStrings().size() > 0) {
        parameter = new StringListParameter(juceParameter, parent);
    } else if (juceParameter->isDiscrete()) {
        try {
            // If stof is correctly able to parse string as float, then the parameter is not a string list parameter
            std::stof(juceParameter->getCurrentValueAsText().toStdString());
        } catch(std::invalid_argument &) {
            // Since stof was not able to parse the string as float, it means value contains text and is probably a string list
            parameter = new StringListParameter(juceParameter, parent);
        }
    }
    return parameter;
}
