#include "IntegerParameter.h"

IntegerParameter::IntegerParameter(juce::AudioProcessorParameter *juceParameter, QObject *parent)
    : Parameter(juceParameter, parent)
    , m_juceParameter(juceParameter)
{}

QString IntegerParameter::getValueLabel() const
{
    // Convert normalized value to discrete as per vst3 spec : https://steinbergmedia.github.io/vst3_dev_portal/pages/Technical+Documentation/Parameters+Automation/Index.html#conversion-of-normalized-values
    return QString::number(qMin(m_juceParameter->getNumSteps(), qFloor(m_juceParameter->getValue() * (m_juceParameter->getNumSteps() + 1))));
}

IntegerParameter *IntegerParameter::from(juce::AudioProcessorParameter *juceParameter, QObject *parent)
{
    IntegerParameter *parameter{nullptr};
    // Heuristics to check if a juce parameter needs to be treated as integer parameter
    // Reference : https://steinbergmedia.github.io/vst3_dev_portal/pages/Technical+Documentation/Parameters+Automation/Index.html#conversion-of-normalized-values
    if (juceParameter->isDiscrete() && juceParameter->getNumSteps() > 0) {
        parameter = new IntegerParameter(juceParameter, parent);
    }
    return parameter;
}
