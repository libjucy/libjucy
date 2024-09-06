#include "JuceHeaders.h"
#include "PluginHost.h"
#include <QtCore>
#include <QDebug>
#include <jack/jack.h>

static int jackProcessCallback(jack_nframes_t nframes, void* arg);

class PluginHostPrivate {
public:
    PluginHostPrivate(PluginHost *pluginHost) : q(pluginHost) {}

    PluginHost *q{nullptr};
    std::unique_ptr<juce::AudioPluginInstance> m_plugin{nullptr};
    QString m_jackClientName{"jucy-pluginhost"};
    jack_client_t *m_jackClient{nullptr};
    jack_port_t *m_jackClientAudioInLeftPort{nullptr};
    jack_port_t *m_jackClientAudioInRightPort{nullptr};
    jack_port_t *m_jackClientAudioOutLeftPort{nullptr};
    jack_port_t *m_jackClientAudioOutRightPort{nullptr};

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
            qCritical() << "Invalid plugin identifier :" << pluginIdentifier;
        } else {
            jack_status_t jackStatus{};
            m_jackClient = jack_client_open(m_jackClientName.toUtf8(), JackNullOption, &jackStatus);
            if (m_jackClient != nullptr) {
                if (jack_set_process_callback(m_jackClient, jackProcessCallback, this) == 0) {
                    if (jack_activate(m_jackClient) == 0) {
                        qInfo() << "Jack client creation successful";
                        m_jackClientAudioInLeftPort = jack_port_register(m_jackClient, QString("audio_in_left").toUtf8(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
                        m_jackClientAudioInRightPort = jack_port_register(m_jackClient, QString("audio_in_right").toUtf8(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
                        m_jackClientAudioOutLeftPort = jack_port_register(m_jackClient, QString("audio_out_left").toUtf8(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
                        m_jackClientAudioOutRightPort = jack_port_register(m_jackClient, QString("audio_out_right").toUtf8(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);

                        if (m_jackClientAudioInLeftPort == nullptr || m_jackClientAudioInRightPort == nullptr || m_jackClientAudioOutLeftPort == nullptr || m_jackClientAudioOutRightPort == nullptr) {
                            qCritical() << "Error registering ports for client" << m_jackClientName;
                        } else {
                            qInfo() << "Port registration successful for client" << m_jackClientName;
                            pluginDescription = *discoveredPlugins[0];
                            m_plugin = audioPluginFormatManager.createPluginInstance(pluginDescription, jack_get_sample_rate(m_jackClient), jack_get_buffer_size(m_jackClient), err);
                            if (!m_plugin) {
                                qCritical() << "Error creating plugin instance :" << QString::fromStdString(err.toStdString());
                            } else {
                                qInfo() << "Plugin instantiated :" << pluginIdentifier;
                                m_plugin->enableAllBuses();
                                m_plugin->prepareToPlay(jack_get_sample_rate(m_jackClient), jack_get_buffer_size(m_jackClient));
                            }
                            result = true;
                        }
                    } else {
                        qCritical() << "Error activating jack client" << m_jackClientName;
                    }
                } else {
                    qCritical() << "Error setting jack process callback for client" << m_jackClientName;
                }
            } else {
                qCritical() << "Error creating jack client" << m_jackClientName;
            }
        }

        return result;
    }

    int pluginProcessCallback(jack_nframes_t nframes) {
        if (m_plugin != nullptr) {
            jack_default_audio_sample_t *audioInLeftBuffer = static_cast<jack_default_audio_sample_t*>(jack_port_get_buffer(m_jackClientAudioInLeftPort, nframes));
            jack_default_audio_sample_t *audioInRightBuffer = static_cast<jack_default_audio_sample_t*>(jack_port_get_buffer(m_jackClientAudioInRightPort, nframes));
            jack_default_audio_sample_t *audioOutLeftBuffer = static_cast<jack_default_audio_sample_t*>(jack_port_get_buffer(m_jackClientAudioOutLeftPort, nframes));
            jack_default_audio_sample_t *audioOutRightBuffer = static_cast<jack_default_audio_sample_t*>(jack_port_get_buffer(m_jackClientAudioOutRightPort, nframes));
            jack_default_audio_sample_t *inputBuffers[2]{audioInLeftBuffer, audioInRightBuffer};
            juce::MidiBuffer midiBuffer;
            juce::AudioBuffer<float> audioBuffer = juce::AudioBuffer<float>(inputBuffers, 2, static_cast<int>(nframes));
            m_plugin->processBlock(audioBuffer, midiBuffer);
            auto *outLeftBuffer = audioBuffer.getReadPointer(0);
            auto *outRightBuffer = audioBuffer.getReadPointer(1);
            memcpy(audioOutLeftBuffer, outLeftBuffer, nframes);
            memcpy(audioOutRightBuffer, outRightBuffer, nframes);
        }
        return 0;
    }
};

static int jackProcessCallback(jack_nframes_t nframes, void* arg) {
    PluginHostPrivate *obj = static_cast<PluginHostPrivate*>(arg);
    return obj->pluginProcessCallback(nframes);
}

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
        return QString::fromStdString(d->m_plugin->getName().toStdString());
    } else {
        return "";
    }
}

QString PluginHost::getPluginIdentifier()
{
    if (d->m_plugin != nullptr) {
        return QString::fromStdString(d->m_plugin->getPluginDescription().fileOrIdentifier.toStdString());
    } else {
        return "";
    }
}

QStringList PluginHost::getAllParameterNames()
{
    QStringList parameterNames;
    if (d->m_plugin != nullptr) {
        d->m_plugin->refreshParameterList();
        for (auto *param : d->m_plugin->getParameters()) {
            parameterNames << QString::fromStdString(param->getName(INT_MAX).toStdString());
        }
    }
    return parameterNames;
}

QStringList PluginHost::getAllProgramNames()
{
    QStringList programNames;
    if (d->m_plugin != nullptr) {
        for (int programIndex = 0; programIndex < d->m_plugin->getNumPrograms(); ++programIndex) {
            programNames << QString::fromStdString(d->m_plugin->getProgramName(programIndex).toStdString());
        }
    }
    return programNames;
}

QString PluginHost::getCurrentProgramName()
{
    if (d->m_plugin != nullptr) {
        return QString::fromStdString(d->m_plugin->getProgramName(d->m_plugin->getCurrentProgram()).toStdString());
    } else {
        return "";
    }
}

int PluginHost::getCurrentProgramIndex() {
    if (d->m_plugin != nullptr) {
        return d->m_plugin->getCurrentProgram();
    } else {
        return -1;
    }
}

bool PluginHost::setCurrentProgram(int programIndex)
{
    bool result = false;
    if (d->m_plugin != nullptr) {
        if (programIndex >= 0 && programIndex < d->m_plugin->getNumPrograms()) {
            d->m_plugin->setCurrentProgram(programIndex);
            if (d->m_plugin->getCurrentProgram() == programIndex) {
                result = true;
            } else {
                qWarning() << "Error changing program index to" << programIndex;
            }
        } else {
            qWarning() << "programIndex is out of range. Enter a value between 0 -" << d->m_plugin->getNumPrograms();
        }
    }
    return result;
}

bool PluginHost::setCurrentProgram(QString programName)
{
    bool result = false;
    if (d->m_plugin != nullptr) {
        const QStringList allProgramNames = getAllProgramNames();
        const int programIndex = allProgramNames.indexOf(programName);
        if (programIndex != -1) {
            result = setCurrentProgram(programIndex);
        } else {
            qWarning() << "Cannot find program" << programName;
        }
    }
    return result;
}
