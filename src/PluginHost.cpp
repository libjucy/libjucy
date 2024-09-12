#include "PluginHost.h"
#include "StringParameter.h"
#include "BooleanParameter.h"
#include <QtCore>
#include <QDebug>
#include <jack/jack.h>
#include <jack/midiport.h>

static int jackProcessCallback(jack_nframes_t nframes, void* arg);

class PluginHostPrivate {
public:
    PluginHostPrivate(PluginHost *pluginHost, QString pluginIdentifier)
        : q(pluginHost)
        , m_pluginIdentifier(pluginIdentifier)
    {
        if (!PluginHostPrivate::juceEventLoop->isThreadRunning()) {
            PluginHostPrivate::juceEventLoop->start();
        }
    }

    static JuceEventLoop *juceEventLoop;
    PluginHost *q{nullptr};
    QString m_pluginIdentifier;
    QString m_jackClientName{"jucy-pluginhost"};
    std::unique_ptr<juce::AudioPluginInstance> m_plugin{nullptr};
    jack_client_t *m_jackClient{nullptr};
    jack_port_t *m_jackClientAudioInLeftPort{nullptr};
    jack_port_t *m_jackClientAudioInRightPort{nullptr};
    jack_port_t *m_jackClientMidiInPort{nullptr};
    jack_port_t *m_jackClientAudioOutLeftPort{nullptr};
    jack_port_t *m_jackClientAudioOutRightPort{nullptr};    

    bool loadPlugin(juce::AudioPluginFormatManager *pluginFormatManager) {
        juce::OwnedArray<juce::PluginDescription> discoveredPlugins;
        juce::PluginDescription pluginDescription;
        juce::String err;
        juce::KnownPluginList kpl;
        bool result = false;

        // parse the plugin path into a PluginDescription instance
        kpl.scanAndAddDragAndDroppedFiles(*pluginFormatManager, juce::StringArray(m_pluginIdentifier.toStdString()), discoveredPlugins);
        // check if the requested plugin was found
        if (!discoveredPlugins.isEmpty()) {
            jack_status_t jackStatus{};
            m_jackClient = jack_client_open(m_jackClientName.toUtf8(), JackNullOption, &jackStatus);
            if (m_jackClient != nullptr) {
                if (jack_set_process_callback(m_jackClient, jackProcessCallback, this) == 0) {
                    m_jackClientAudioInLeftPort = jack_port_register(m_jackClient, QString("audio_in_1").toUtf8(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
                    m_jackClientAudioInRightPort = jack_port_register(m_jackClient, QString("audio_in_2").toUtf8(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
                    m_jackClientMidiInPort = jack_port_register(m_jackClient, QString("midi_in").toUtf8(), JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);
                    m_jackClientAudioOutLeftPort = jack_port_register(m_jackClient, QString("audio_out_1").toUtf8(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
                    m_jackClientAudioOutRightPort = jack_port_register(m_jackClient, QString("audio_out_2").toUtf8(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);

                    if (m_jackClientAudioInLeftPort != nullptr &&
                        m_jackClientAudioInRightPort != nullptr &&
                        m_jackClientMidiInPort != nullptr &&
                        m_jackClientAudioOutLeftPort != nullptr &&
                        m_jackClientAudioOutRightPort != nullptr
                    ) {
                        qInfo() << "Port registration successful for client" << m_jackClientName;
                        pluginDescription = *discoveredPlugins[0];
                        m_plugin = pluginFormatManager->createPluginInstance(pluginDescription, jack_get_sample_rate(m_jackClient), static_cast<int>(jack_get_buffer_size(m_jackClient)), err);
                        if (m_plugin != nullptr) {
                            qInfo() << "Plugin instantiated :" << m_pluginIdentifier;
                            m_plugin->enableAllBuses();
                            m_plugin->prepareToPlay(jack_get_sample_rate(m_jackClient), static_cast<int>(jack_get_buffer_size(m_jackClient)));
                            if (jack_activate(m_jackClient) == 0) {
                                qInfo() << "Jack client creation successful";
                                result = true;
                            } else {
                                qCritical() << "Error activating jack client" << m_jackClientName;
                            }
                        } else {
                            qCritical() << "Error creating plugin instance :" << QString::fromStdString(err.toStdString());
                        }
                    } else {
                        qCritical() << "Error registering ports for client" << m_jackClientName;
                    }
                } else {
                    qCritical() << "Error setting jack process callback for client" << m_jackClientName;
                }
            } else {
                qCritical() << "Error creating jack client" << m_jackClientName;
            }
        } else {
            qCritical() << "Invalid plugin identifier :" << m_pluginIdentifier;
        }

        return result;
    }

    bool unloadPlugin() {
        bool result=false;
        if (m_jackClient != nullptr) {
            jack_deactivate(m_jackClient);
            jack_client_close(m_jackClient);
            m_jackClient = nullptr;
        } else {
            qWarning() << "Jack client not active";
        }
        if (m_plugin != nullptr) {
            m_plugin->releaseResources();
            m_plugin.reset();
            result=true;
        } else {
            qWarning() << "Plugin not instantiated";
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
            juce::AudioBuffer<float> audioBuffer = juce::AudioBuffer<float>(inputBuffers, 2, static_cast<int>(nframes));
            juce::MidiBuffer midiBuffer;
            // void *midiInBuffer = jack_port_get_buffer(m_jackClientMidiInPort, nframes);
            // for (int midiEventIndex = 0; midiEventIndex < jack_midi_get_event_count(midiInBuffer); ++midiEventIndex) {
            //     // juce::MidiMessage midiMessage = juce::MidiMessage(
            //     jack_midi_event_t *midiEvent;
            //     if (jack_midi_event_get(midiEvent, midiBuffer, midiEventIndex) == 0) {
            //         midiBuffer.addEvent(juce::MidiMessage(midiEvent->buffer, midiEvent->size), midiEventIndex);
            //     } else {
            //         qWarning() << "Error geting midi event data from buffer";
            //     }
            // }

            m_plugin->processBlock(audioBuffer, midiBuffer);
            auto *outLeftBuffer = audioBuffer.getReadPointer(0);
            auto *outRightBuffer = audioBuffer.getReadPointer(1);
            memcpy(audioOutLeftBuffer, outLeftBuffer, nframes * sizeof(jack_default_audio_sample_t));
            memcpy(audioOutRightBuffer, outRightBuffer, nframes * sizeof(jack_default_audio_sample_t));
        }
        return 0;
    }
};
JuceEventLoop *PluginHostPrivate::juceEventLoop = new JuceEventLoop();

static int jackProcessCallback(jack_nframes_t nframes, void* arg) {
    PluginHostPrivate *obj = static_cast<PluginHostPrivate*>(arg);
    return obj->pluginProcessCallback(nframes);
}

PluginHost::PluginHost(QString pluginIdentifier, QObject *parent)
    : QObject(parent)
    , d(new PluginHostPrivate(this, pluginIdentifier))
{}

PluginHost::~PluginHost()
{
    unloadPlugin();
    delete d;
}

bool PluginHost::loadPlugin(juce::AudioPluginFormatManager *pluginFormatManager)
{
    return d->loadPlugin(pluginFormatManager);
}

bool PluginHost::unloadPlugin()
{
    return d->unloadPlugin();
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
    return d->m_pluginIdentifier;
}

QList<Parameter *> PluginHost::getAllParameters()
{
    QList<Parameter *> parameters;
    if (d->m_plugin != nullptr) {
        d->m_plugin->refreshParameterList();
        for (auto *juceParameter : d->m_plugin->getParameters()) {
            if (juceParameter->getAllValueStrings().size() > 0) {
                // Test for String Parameter
                StringParameter *parameter = new StringParameter(juceParameter, this);
                parameters << parameter;
            } else if (juceParameter->getNumSteps() == 2) {
                // Test for boolean parameter
                BooleanParameter *parameter = new BooleanParameter(juceParameter, this);
                parameters << parameter;
            } else {
                // If all tests fails, make it a generic parameter
                Parameter *parameter = new Parameter(juceParameter, this);
                parameters << parameter;
            }
        }
    }
    return parameters;
}

QStringList PluginHost::getAllPresets()
{
    QStringList presetNames;
    if (d->m_plugin != nullptr) {
        for (int presetIndex = 0; presetIndex < d->m_plugin->getNumPrograms(); ++presetIndex) {
            presetNames << QString::fromStdString(d->m_plugin->getProgramName(presetIndex).toStdString());
        }
    }
    return presetNames;
}

QString PluginHost::getCurrentPreset()
{
    if (d->m_plugin != nullptr) {
        return QString::fromStdString(d->m_plugin->getProgramName(d->m_plugin->getCurrentProgram()).toStdString());
    } else {
        return "";
    }
}

bool PluginHost::setCurrentPreset(QString presetName)
{
    bool result = false;
    if (d->m_plugin != nullptr) {
        const QStringList allPresetNames = getAllPresets();
        const int presetIndex = allPresetNames.indexOf(presetName);
        if (presetIndex != -1) {
            d->m_plugin->setCurrentProgram(presetIndex);
            if (d->m_plugin->getCurrentProgram() == presetIndex) {
                result = true;
            } else {
                qWarning() << "Error changing preset to" << presetName;
            }
        } else {
            qWarning() << "Cannot find preset" << presetName;
        }
    }
    return result;
}
