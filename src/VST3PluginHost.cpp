#include "VST3PluginHost.h"
#include <QDebug>

VST3PluginHost::VST3PluginHost(QString pluginIdentifier, QString jackClientName, QObject *parent)
    : PluginHost(pluginIdentifier, jackClientName, parent)
    , m_audioPluginFormatManager(new juce::AudioPluginFormatManager())
{
    juce::VST3PluginFormat *vst3PluginFormat = new juce::VST3PluginFormat();
    m_audioPluginFormatManager->addFormat(vst3PluginFormat);
}

bool VST3PluginHost::loadPlugin()
{
    return PluginHost::loadPlugin(m_audioPluginFormatManager);
}

QList<PluginDescription*> VST3PluginHost::getAllPlugins()
{
    juce::KnownPluginList kpl;
    QList<PluginDescription*> result;
    qDebug() << "VST3_PATH :" << qgetenv("VST3_PATH");
    const juce::File deadMansPedalFile("/tmp/test");
    juce::FileSearchPath vst3Path(juce::String(qgetenv("VST3_PATH").replace(":", ";").toStdString()));
    juce::PluginDirectoryScanner pluginDirScanner(kpl, *m_audioPluginFormatManager->getFormat(0), vst3Path, true, deadMansPedalFile);
    juce::String nameOfPluginBeingScanned;

    while (pluginDirScanner.scanNextFile(true, nameOfPluginBeingScanned)) {
        qDebug() << "Scanning plugin" << QString::fromStdString(nameOfPluginBeingScanned.toStdString());
    }

    for (auto plugin : kpl.getTypes()) {
        PluginDescription* pluginDescription = new PluginDescription(this);
        pluginDescription->name = QString::fromStdString(plugin.name.toStdString());
        pluginDescription->descriptiveName = QString::fromStdString(plugin.descriptiveName.toStdString());
        pluginDescription->pluginFormatName = QString::fromStdString(plugin.pluginFormatName.toStdString());
        pluginDescription->category = QString::fromStdString(plugin.category.toStdString());
        pluginDescription->fileOrIdentifier = QString::fromStdString(plugin.fileOrIdentifier.toStdString());
        pluginDescription->isInstrument = plugin.isInstrument;
        pluginDescription->numInputChannels = plugin.numInputChannels;
        pluginDescription->numOutputChannels = plugin.numOutputChannels;

        result << pluginDescription;
    }
    return result;
}
