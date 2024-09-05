#include "JuceHeaders.h"
#include "PluginHost.h"
#include <QtCore>
#include <QDebug>

class PluginHostPrivate {
public:
    PluginHostPrivate(PluginHost *pluginHost) : q(pluginHost) {}

    PluginHost *q{nullptr};
    std::unique_ptr<juce::AudioPluginInstance> m_plugin{nullptr};

    bool loadPlugin(QString pluginIdentifier) {
        juce::OwnedArray<juce::PluginDescription> discoveredPlugins;
        juce::PluginDescription pluginDescription;
        juce::String err;
        juce::LV2PluginFormat *lv2PluginFormat = new juce::LV2PluginFormat();
        juce::AudioPluginFormatManager audioPluginFormatManager;
        juce::KnownPluginList kpl;
        const char *LV2_PATH = std::getenv("LV2_PATH");
        bool result = false;

        if (LV2_PATH != nullptr) {
            lv2PluginFormat->searchPathsForPlugins(juce::FileSearchPath(juce::String(LV2_PATH)), false);
        }
        audioPluginFormatManager.addFormat(lv2PluginFormat);
        // parse the plugin path into a PluginDescription instance
        kpl.scanAndAddDragAndDroppedFiles(audioPluginFormatManager, juce::StringArray(pluginIdentifier.toStdString()), discoveredPlugins);
        // check if the requested plugin was found
        if (discoveredPlugins.isEmpty()) {
            qDebug() << "Invalid plugin identifier :" << pluginIdentifier;
        } else {
            pluginDescription = *discoveredPlugins[0];
            m_plugin = audioPluginFormatManager.createPluginInstance(pluginDescription, 48000, 1024, err);
            if (!m_plugin) {
                qDebug() << "Error creating plugin instance :" << QString::fromStdString(err.toStdString());
            } else {
                qDebug() << "Plugin instantiated :" << pluginIdentifier;
                result = true;
            }
        }

        return result;
    }
};

PluginHost::PluginHost(QObject *parent)
    : QObject(parent)
    , d(new PluginHostPrivate(this))
{

}

PluginHost::~PluginHost()
{
    delete d;
}

bool PluginHost::loadPlugin(QString pluginIdentifier)
{
    return d->loadPlugin(pluginIdentifier);
}

QString PluginHost::getPluginName()
{
    if (d->m_plugin != nullptr) {
        return QString::fromStdString(d->m_plugin.get()->getName().toStdString());
    } else {
        return "";
    }
}

QString PluginHost::getPluginIdentifier()
{
    if (d->m_plugin != nullptr) {
        return QString::fromStdString(d->m_plugin.get()->getPluginDescription().fileOrIdentifier.toStdString());
    } else {
        return "";
    }
}

QStringList PluginHost::listPluginParameters()
{
    QStringList parameterNames;
    if (d->m_plugin != nullptr) {
        for (auto *param : d->m_plugin.get()->getParameters()) {
            parameterNames << QString::fromStdString(param->getName(INT_MAX).toStdString());
        }
    }
    return parameterNames;
}

QStringList PluginHost::getAllProgramNames()
{
    QStringList programNames;
    if (d->m_plugin != nullptr) {
        for (int programIndex = 0; programIndex < d->m_plugin.get()->getNumPrograms(); ++programIndex) {
            programNames << QString::fromStdString(d->m_plugin.get()->getProgramName(programIndex).toStdString());
        }
    }
    return programNames;
}

QString PluginHost::getCurrentProgramName()
{
    if (d->m_plugin != nullptr) {
        return QString::fromStdString(d->m_plugin.get()->getProgramName(d->m_plugin.get()->getCurrentProgram()).toStdString());
    } else {
        return "";
    }
}

int PluginHost::getCurrentProgramIndex() {
    if (d->m_plugin != nullptr) {
        return d->m_plugin.get()->getCurrentProgram();
    } else {
        return -1;
    }
}

bool PluginHost::setCurrentProgramIndex(int programIndex)
{
    bool result = false;
    if (d->m_plugin != nullptr) {
        if (programIndex >= 0 && programIndex < d->m_plugin.get()->getNumPrograms()) {
            d->m_plugin.get()->setCurrentProgram(programIndex);
            if (d->m_plugin.get()->getCurrentProgram() == programIndex) {
                result = true;
            } else {
                qDebug() << "Error changing program index to" << programIndex;
            }
        } else {
            qDebug() << "programIndex is out of range. Enter a value between 0 -" << d->m_plugin.get()->getNumPrograms();
        }
    }
    return result;
}
