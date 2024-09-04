#include "PluginHost.h"
#include <QtCore>
#include <QDebug>

class PluginHostPrivate {
public:
    PluginHostPrivate(PluginHost *pluginHost)
        : m_pluginHost(pluginHost)
    {
        // QString pluginPath = "/zynthian/zynthian-plugins/lv2/aether.lv2/aether_dsp.so";
        QString pluginPath = "https://airwindows.com//plugins/Airwindows_Consolidated";
        LV2PluginFormat *lv2PluginFormat = new LV2PluginFormat();
        juce::AudioPluginFormatManager audioPluginFormatManager;
        audioPluginFormatManager.addFormat(lv2PluginFormat);
        for (juce::String path : lv2PluginFormat->searchPathsForPlugins(juce::FileSearchPath(juce::String("/zynthian/zynthian-plugins/lv2/")), false)) {
            qDebug() << "Plugin :" << QString::fromStdString(path.toStdString());
        }

        // parse the plugin path into a PluginDescription instance
        juce::PluginDescription pluginDescription;
        {
            juce::OwnedArray<juce::PluginDescription> pluginDescriptions;

            juce::KnownPluginList kpl;
            kpl.scanAndAddDragAndDroppedFiles(audioPluginFormatManager, juce::StringArray(pluginPath.toStdString()), pluginDescriptions);

            // check if the requested plugin was found
            if (pluginDescriptions.isEmpty()) {
                qDebug() << "Invalid plugin identifier:" << pluginPath;
            }

            pluginDescription = *pluginDescriptions[0];
        }

        // create plugin instance
        std::unique_ptr<juce::AudioPluginInstance> plugin;
        {
            juce::String err;
            plugin = audioPluginFormatManager.createPluginInstance(pluginDescription, 48000, 1024, err);

            if (!plugin) {
                qDebug() << "Error creating plugin instance:" << QString::fromStdString(err.toStdString());
            }
        }
    }
private:
    PluginHost *m_pluginHost{nullptr};
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
