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
    return QString::fromStdString(d->m_plugin.get()->getName().toStdString());
}

QString PluginHost::getPluginIdentifier()
{
    return QString::fromStdString(d->m_plugin.get()->getPluginDescription().fileOrIdentifier.toStdString());
}

QStringList PluginHost::listPluginParameters()
{
    QStringList parameterNames;
    for (auto *param : d->m_plugin.get()->getParameters()) {
        parameterNames << QString::fromStdString(param->getName(INT_MAX).toStdString());
    }

    return parameterNames;
}
