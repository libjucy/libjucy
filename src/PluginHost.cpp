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
    PluginHostPrivate(PluginHost *pluginHost, QString pluginIdentifier, QString jackClientName)
        : q(pluginHost)
        , m_pluginIdentifier(pluginIdentifier)
        , m_jackClientName(jackClientName)
        , m_juceMidiBuffer(new juce::MidiBuffer())
        , m_juceEventLoop(new JuceEventLoop())
    {
        m_juceMidiBuffer->ensureSize(2048);
    }

    PluginHost *q{nullptr};
    QString m_pluginIdentifier;
    QString m_jackClientName;
    std::unique_ptr<juce::AudioPluginInstance> m_plugin{nullptr};
    jack_client_t *m_jackClient{nullptr};
    jack_port_t *m_jackClientAudioInLeftPort{nullptr};
    jack_port_t *m_jackClientAudioInRightPort{nullptr};
    jack_port_t *m_jackClientMidiInPort{nullptr};
    jack_port_t *m_jackClientAudioOutLeftPort{nullptr};
    jack_port_t *m_jackClientAudioOutRightPort{nullptr};
    jack_default_audio_sample_t *m_audioInLeftBuffer;
    jack_default_audio_sample_t *m_audioInRightBuffer;
    jack_default_audio_sample_t *m_audioOutLeftBuffer;
    jack_default_audio_sample_t *m_audioOutRightBuffer;
    juce::MidiBuffer *m_juceMidiBuffer{nullptr};
    jack_midi_event_t m_midiEvent;
    JuceEventLoop *m_juceEventLoop{nullptr};
    bool m_pluginInstantiated{false};
    jack_nframes_t m_jackSampleRate;
    int m_jackBufferSize;
    int m_pluginNumInputPorts;
    int m_pluginNumOutputPorts;
    bool m_pluginAcceptsMidi;

    bool loadPlugin(juce::AudioPluginFormatManager *pluginFormatManager) {
        juce::OwnedArray<juce::PluginDescription> discoveredPlugins;
        juce::PluginDescription pluginDescription;
        juce::String err;
        juce::KnownPluginList kpl;
        jack_status_t jackStatus{};
        bool result = false;

        // parse the plugin path into a PluginDescription instance
        kpl.scanAndAddDragAndDroppedFiles(*pluginFormatManager, juce::StringArray(m_pluginIdentifier.toStdString()), discoveredPlugins);
        // check if the requested plugin was found
        if (!discoveredPlugins.isEmpty()) {
            m_jackClient = jack_client_open(m_jackClientName.toUtf8(), JackNullOption, &jackStatus);
            if (m_jackClient != nullptr) {
                qInfo() << "Jack client creation successful";
                if (jack_set_process_callback(m_jackClient, jackProcessCallback, this) == 0) {
                    if (jack_activate(m_jackClient) == 0) {
                        // Activate jack client temporarily to retrieve sampleRate and bufferSize
                        m_jackSampleRate = jack_get_sample_rate(m_jackClient);
                        m_jackBufferSize = static_cast<int>(jack_get_buffer_size(m_jackClient));
                        jack_deactivate(m_jackClient);
                    } else {
                        qCritical() << "Error activating jack client" << m_jackClientName;
                    }
                    pluginDescription = *discoveredPlugins[0];
                    m_plugin = pluginFormatManager->createPluginInstance(pluginDescription, m_jackSampleRate, m_jackBufferSize, err);
                    if (m_plugin != nullptr) {
                        qInfo() << "Plugin instantiated :" << m_pluginIdentifier;
                        m_plugin->enableAllBuses();
                        m_pluginNumInputPorts = m_plugin->getTotalNumInputChannels();
                        m_pluginNumOutputPorts = m_plugin->getTotalNumOutputChannels();
                        m_pluginAcceptsMidi = m_plugin->acceptsMidi();
                        if (m_pluginNumInputPorts > 0) {
                            // Register left audio input port if there are atleast 1 input port
                            qInfo() << "Registering left audio input port";
                            m_jackClientAudioInLeftPort = jack_port_register(m_jackClient, QString("audio_in_1").toUtf8(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
                        }
                        if (m_pluginNumInputPorts > 1) {
                            // Register right audio input port if there are atleast 2 input ports
                            qInfo() << "Registering right audio input port";
                            m_jackClientAudioInRightPort = jack_port_register(m_jackClient, QString("audio_in_2").toUtf8(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
                        }
                        if (m_pluginAcceptsMidi) {
                            // Register midi input port if applicable
                            qInfo() << "Registering midi input port";
                            m_jackClientMidiInPort = jack_port_register(m_jackClient, QString("midi_in").toUtf8(), JACK_DEFAULT_MIDI_TYPE, JackPortIsInput | JackPortIsTerminal, 0);
                        }
                        if (m_pluginNumOutputPorts > 0) {
                            // Register left audio output port if there are atleast 1 output port
                            qInfo() << "Registering left audio output port";
                            m_jackClientAudioOutLeftPort = jack_port_register(m_jackClient, QString("audio_out_1").toUtf8(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
                        }
                        if (m_pluginNumOutputPorts > 1) {
                            // Register right audio output port if there are atleast 2 output ports
                            qInfo() << "Registering right audio output port";
                            m_jackClientAudioOutRightPort = jack_port_register(m_jackClient, QString("audio_out_2").toUtf8(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
                        }
                        // TODO : Handle plugins having >2 ports

                        if (m_jackClientAudioInLeftPort != nullptr ||
                            m_jackClientAudioInRightPort != nullptr ||
                            m_jackClientMidiInPort != nullptr ||
                            m_jackClientAudioOutLeftPort != nullptr ||
                            m_jackClientAudioOutRightPort != nullptr
                            ) {
                            qInfo() << "Port registration successful for client" << m_jackClientName;
                            if (jack_activate(m_jackClient) == 0) {
                                if (!m_juceEventLoop->isThreadRunning()) {
                                    m_juceEventLoop->start();
                                }
                                m_plugin->prepareToPlay(m_jackSampleRate, m_jackBufferSize);
                                m_pluginInstantiated = true;
                                result = true;
                            } else {
                                qCritical() << "Error activating jack client" << m_jackClientName;
                            }
                        } else {
                            qCritical() << "Error registering ports for client" << m_jackClientName;
                        }
                    } else {
                        qCritical() << "Error creating plugin instance :" << QString::fromStdString(err.toStdString());
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
        if (m_juceEventLoop->isThreadRunning()) {
            m_juceEventLoop->stop();
        }
        if (m_jackClient != nullptr) {
            jack_deactivate(m_jackClient);
            jack_client_close(m_jackClient);
            m_jackClient = nullptr;
        }
        if (m_plugin != nullptr) {
            m_plugin->releaseResources();
            m_plugin.reset();
            m_pluginInstantiated = false;
            result=true;
        }
        return result;
    }

    int pluginProcessCallback(jack_nframes_t nframes) {
        if (m_plugin != nullptr && m_pluginInstantiated) {
            if (m_pluginNumInputPorts > 0) {
                m_audioInLeftBuffer = static_cast<jack_default_audio_sample_t*>(jack_port_get_buffer(m_jackClientAudioInLeftPort, nframes));
            }
            if (m_pluginNumInputPorts > 1) {
                m_audioInRightBuffer = static_cast<jack_default_audio_sample_t*>(jack_port_get_buffer(m_jackClientAudioInRightPort, nframes));
            }
            if (m_pluginNumOutputPorts > 0) {
                m_audioOutLeftBuffer = static_cast<jack_default_audio_sample_t*>(jack_port_get_buffer(m_jackClientAudioOutLeftPort, nframes));
            }
            if (m_pluginNumOutputPorts > 1) {
                m_audioOutRightBuffer = static_cast<jack_default_audio_sample_t*>(jack_port_get_buffer(m_jackClientAudioOutRightPort, nframes));
            }
            if (m_pluginAcceptsMidi && m_jackClientMidiInPort != nullptr) {
                m_juceMidiBuffer->clear();
                void *midiInputPortBuffer = jack_port_get_buffer(m_jackClientMidiInPort, nframes);
                for (jack_nframes_t midiEventIndex = 0; midiEventIndex < jack_midi_get_event_count(midiInputPortBuffer); ++midiEventIndex) {
                    if (jack_midi_event_get(&m_midiEvent, midiInputPortBuffer, midiEventIndex) == 0) {
                        juce::MidiMessage midiMessage = juce::MidiMessage(m_midiEvent.buffer, m_midiEvent.size);
                        // qDebug() << "Adding midi event at index" << midiEventIndex << "data" << m_midiEvent.buffer;
                        m_juceMidiBuffer->addEvent(midiMessage, static_cast<int>(midiEventIndex));
                    } else {
                        qWarning() << "Error getting midi event data from buffer";
                    }
                }
            }

            // Process input audio/midi events and write audio output to respective ports
            if (m_pluginAcceptsMidi) {
                // When working with synths, there are no jack input ports, hence no input buffers are available.
                // processBlock expects buffers so that it can write audio data after processing. Hence create
                // buffers as per the number of output ports and pass it to processBlock
                if (m_pluginNumOutputPorts == 1) {
                    float *leftBuffer;
                    leftBuffer = static_cast<float *>(calloc(static_cast<size_t>(m_jackBufferSize), sizeof(float)));
                    float *inputBuffers[1] = {leftBuffer};
                    juce::AudioBuffer<float> audioBuffer = juce::AudioBuffer<float>(inputBuffers, 1, static_cast<int>(nframes));
                    m_plugin->processBlock(audioBuffer, *m_juceMidiBuffer);
                    auto *outLeftBuffer = audioBuffer.getReadPointer(0);
                    memcpy(m_audioOutLeftBuffer, outLeftBuffer, nframes * sizeof(jack_default_audio_sample_t));
                } else if (m_pluginNumOutputPorts > 1) {
                    float *leftBuffer;
                    float *rightBuffer;
                    leftBuffer = static_cast<float *>(calloc(static_cast<size_t>(m_jackBufferSize), sizeof(float)));
                    rightBuffer = static_cast<float *>(calloc(static_cast<size_t>(m_jackBufferSize), sizeof(float)));
                    float *inputBuffers[2] = {leftBuffer, rightBuffer};
                    juce::AudioBuffer<float> audioBuffer = juce::AudioBuffer<float>(inputBuffers, 2, static_cast<int>(nframes));
                    m_plugin->processBlock(audioBuffer, *m_juceMidiBuffer);
                    auto *outLeftBuffer = audioBuffer.getReadPointer(0);
                    auto *outRightBuffer = audioBuffer.getReadPointer(1);
                    memcpy(m_audioOutLeftBuffer, outLeftBuffer, nframes * sizeof(jack_default_audio_sample_t));
                    memcpy(m_audioOutRightBuffer, outRightBuffer, nframes * sizeof(jack_default_audio_sample_t));
                }
            } else {
                if (m_pluginNumInputPorts == 1) {
                    jack_default_audio_sample_t *inputBuffers[1]{m_audioInLeftBuffer};
                    juce::AudioBuffer<float> audioBuffer = juce::AudioBuffer<float>(inputBuffers, 1, static_cast<int>(nframes));
                    m_plugin->processBlock(audioBuffer, *m_juceMidiBuffer);
                    auto *outLeftBuffer = audioBuffer.getReadPointer(0);
                    memcpy(m_audioOutLeftBuffer, outLeftBuffer, nframes * sizeof(jack_default_audio_sample_t));
                } else if (m_pluginNumInputPorts > 1) {
                    jack_default_audio_sample_t *inputBuffers[2]{m_audioInLeftBuffer, m_audioInRightBuffer};
                    juce::AudioBuffer<float> audioBuffer = juce::AudioBuffer<float>(inputBuffers, 2, static_cast<int>(nframes));
                    m_plugin->processBlock(audioBuffer, *m_juceMidiBuffer);
                    auto *outLeftBuffer = audioBuffer.getReadPointer(0);
                    auto *outRightBuffer = audioBuffer.getReadPointer(1);
                    memcpy(m_audioOutLeftBuffer, outLeftBuffer, nframes * sizeof(jack_default_audio_sample_t));
                    memcpy(m_audioOutRightBuffer, outRightBuffer, nframes * sizeof(jack_default_audio_sample_t));
                }
            }
        }
        return 0;
    }
};

static int jackProcessCallback(jack_nframes_t nframes, void* arg) {
    PluginHostPrivate *obj = static_cast<PluginHostPrivate*>(arg);
    return obj->pluginProcessCallback(nframes);
}

PluginHost::PluginHost(QString pluginIdentifier, QString jackClientName, QObject *parent)
    : QObject(parent)
    , d(new PluginHostPrivate(this, pluginIdentifier, jackClientName))
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

Parameter *PluginHost::getParameter(QString parameterName)
{
    Parameter *result = nullptr;
    for (auto parameter : getAllParameters()) {
        if (parameter->getName() == parameterName) {
            result = parameter;
        }
    }
    return result;
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
