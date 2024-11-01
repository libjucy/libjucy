#include "PluginHost.h"
#include "StringParameter.h"
#include "BooleanParameter.h"
#include <QtCore>
#include <QDebug>
#include <jack/jack.h>
#include <jack/midiport.h>

static int jackProcessCallback(jack_nframes_t nframes, void* arg);

class JucyPlayHead : public juce::AudioPlayHead {
public:
    virtual juce::Optional<juce::AudioPlayHead::PositionInfo> getPosition() const override
    {
        juce::AudioPlayHead::PositionInfo currentPosition;
        currentPosition.setIsPlaying(true);
        currentPosition.setIsRecording(false);
        currentPosition.setIsLooping(false);
        // currentPosition.setEditOriginTime(transport.getTimeWhenStarted());

        const double timeInSeconds{position.usecs / 1000000.0};
        currentPosition.setTimeInSamples(int64_t(timeInSeconds * sampleRate));
        currentPosition.setTimeInSeconds(timeInSeconds / 1000000.0);

        currentPosition.setBpm(position.beats_per_minute);
        juce::AudioPlayHead::TimeSignature signature;
        signature.numerator = position.ticks_per_beat;
        signature.denominator = position.beat_type;
        currentPosition.setTimeSignature(signature);

        currentPosition.setPpqPositionOfLastBarStart(position.bar_start_tick);
        currentPosition.setPpqPosition((((position.bar * position.beat_type) + position.beat) * position.ticks_per_beat) + position.tick);

        return currentPosition;
    }
    jack_transport_state_t transportState;
    jack_position_t position;
    uint64_t sampleRate{48000};
};

class PluginHostPrivate {
public:
    PluginHostPrivate(PluginHost *pluginHost, QString pluginIdentifier, QString jackClientName)
        : q(pluginHost)
        , m_pluginIdentifier(pluginIdentifier)
        , m_jackClientName(jackClientName)
        , m_juceMidiBuffer(new juce::MidiBuffer())
    {
        m_juceMidiBuffer->ensureSize(2048);
    }

    PluginHost *q{nullptr};
    QString m_pluginIdentifier;
    QString m_jackClientName;
    std::unique_ptr<juce::AudioPluginInstance> m_plugin{nullptr};
    jack_client_t *m_jackClient{nullptr};
    jack_port_t **m_jackAudioInputPorts{nullptr};
    jack_port_t **m_jackAudioOutputPorts{nullptr};
    jack_port_t *m_jackMidiInputPort{nullptr};
    jack_default_audio_sample_t **m_audioBuffers{nullptr};
    JucyPlayHead m_playhead;
    juce::AudioBuffer<jack_default_audio_sample_t> *m_juceAudioBuffer{nullptr};
    juce::MidiBuffer *m_juceMidiBuffer{nullptr};
    jack_midi_event_t m_midiEvent;
    bool m_pluginInstantiated{false};
    jack_nframes_t m_jackSampleRate{44100};
    int m_jackBufferSize{1024};
    int m_pluginNumInputPorts{0};
    int m_pluginNumOutputPorts{0};
    bool m_pluginAcceptsMidi;

    bool registerJackPorts() {
        // Register jack midi input port
        if (m_pluginAcceptsMidi) {
            m_jackMidiInputPort = jack_port_register(m_jackClient, QString("midi_in").toUtf8(), JACK_DEFAULT_MIDI_TYPE, JackPortIsInput | JackPortIsTerminal, 0);
        }
        // Register all jack audio input ports
        if (m_pluginNumInputPorts > 0) {
            m_jackAudioInputPorts = static_cast<jack_port_t **>(calloc(static_cast<size_t>(m_pluginNumInputPorts), sizeof(jack_port_t *)));
            if (m_jackAudioInputPorts != nullptr) {
                for (int portIndex = 0; portIndex < m_pluginNumInputPorts; ++portIndex) {
                    m_jackAudioInputPorts[portIndex] = jack_port_register(m_jackClient, QString("audio_in_%1").arg(portIndex + 1).toUtf8(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
                }
            }
        }
        // Register all jack audio output ports
        if (m_pluginNumOutputPorts > 0) {
            m_jackAudioOutputPorts = static_cast<jack_port_t **>(calloc(static_cast<size_t>(m_pluginNumOutputPorts), sizeof(jack_port_t *)));
            if (m_jackAudioOutputPorts != nullptr) {
                for (int portIndex = 0; portIndex < m_pluginNumOutputPorts; ++portIndex) {
                    m_jackAudioOutputPorts[portIndex] = jack_port_register(m_jackClient, QString("audio_out_%1").arg(portIndex + 1).toUtf8(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
                }
            }
        }
        // Allocate audio buffers. Number of audio buffers should be the maximum of the total number of input and output ports
        // Reference : https://docs.juce.com/master/classAudioProcessor.html#a77571fc8ce02ddf4fe3a56fe57ea9392
        m_audioBuffers = static_cast<jack_default_audio_sample_t **>(calloc(static_cast<size_t>(qMax(m_pluginNumInputPorts, m_pluginNumOutputPorts)), sizeof(jack_default_audio_sample_t *)));
        for (int bufferIndex = 0; bufferIndex < qMax(m_pluginNumInputPorts, m_pluginNumOutputPorts); ++bufferIndex) {
            m_audioBuffers[bufferIndex] = static_cast<jack_default_audio_sample_t *>(calloc(static_cast<size_t>(m_jackBufferSize), sizeof(jack_default_audio_sample_t)));
        }
        m_juceAudioBuffer = new juce::AudioBuffer<jack_default_audio_sample_t>(m_audioBuffers, qMax(m_pluginNumInputPorts, m_pluginNumOutputPorts), static_cast<int>(m_jackBufferSize));

        return true;
    }

    bool unregisterJackPorts() {
        // Unregister jack midi input port
        if (m_jackMidiInputPort != nullptr) {
            jack_port_unregister(m_jackClient, m_jackMidiInputPort);
            m_jackMidiInputPort = nullptr;
        }
        // Unregister all jack audio input ports
        for (int portIndex = 0; portIndex < m_pluginNumInputPorts; ++portIndex) {
            if (m_jackAudioInputPorts[portIndex] != nullptr) {
                jack_port_unregister(m_jackClient, m_jackAudioInputPorts[portIndex]);
            }
        }
        free(m_jackAudioInputPorts);
        m_jackAudioInputPorts = nullptr;
        // Unregister all jack audio output ports
        for (int portIndex = 0; portIndex < m_pluginNumOutputPorts; ++portIndex) {
            if (m_jackAudioOutputPorts[portIndex] != nullptr) {
                jack_port_unregister(m_jackClient, m_jackAudioOutputPorts[portIndex]);
            }
        }
        free(m_jackAudioOutputPorts);
        m_jackAudioOutputPorts = nullptr;
        // Free audio buffers
        for (int bufferIndex = 0; bufferIndex < qMax(m_pluginNumInputPorts, m_pluginNumOutputPorts); ++bufferIndex) {
            free(m_audioBuffers[bufferIndex]);
        }
        free(m_audioBuffers);
        m_audioBuffers = nullptr;
        // Delete juceAudioBuffers
        delete m_juceAudioBuffer;
        m_juceAudioBuffer = nullptr;

        return true;
    }

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
                        if (registerJackPorts()) {
                            qInfo() << "Port registration successful for client" << m_jackClientName;
                            if (jack_activate(m_jackClient) == 0) {
                                m_playhead.sampleRate = m_jackSampleRate;
                                m_plugin->setPlayHead(&m_playhead);
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
        if (m_jackClient != nullptr) {
            jack_deactivate(m_jackClient);
            unregisterJackPorts();
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
            m_playhead.transportState = jack_transport_query(m_jackClient, &m_playhead.position);
            if (m_pluginAcceptsMidi) {
                // Populate juce midi buffer to send to processBlock
                m_juceMidiBuffer->clear();
                void *midiInputPortBuffer = jack_port_get_buffer(m_jackMidiInputPort, nframes);
                for (jack_nframes_t midiEventIndex = 0; midiEventIndex < jack_midi_get_event_count(midiInputPortBuffer); ++midiEventIndex) {
                    if (jack_midi_event_get(&m_midiEvent, midiInputPortBuffer, midiEventIndex) == 0) {
                        juce::MidiMessage midiMessage = juce::MidiMessage(m_midiEvent.buffer, m_midiEvent.size);
                        // if (midiMessage.isNoteOnOrOff()) {
                        //     qDebug() << "Adding midi event at index" << midiEventIndex << "data" << m_midiEvent.buffer;
                        // }
                        m_juceMidiBuffer->addEvent(midiMessage, static_cast<int>(midiEventIndex));
                    } else {
                        qWarning() << "Error getting midi event data from buffer";
                    }
                }
                // Clear audio buffers before sending to processBlock
                for (int bufferIndex = 0; bufferIndex < qMax(m_pluginNumInputPorts, m_pluginNumOutputPorts); ++bufferIndex) {
                    jack_default_audio_sample_t *audioBuffer = m_juceAudioBuffer->getWritePointer(bufferIndex);
                    memset(audioBuffer, 0, nframes * sizeof(jack_default_audio_sample_t));
                }
            }
            // Populate audio buffers to send to processBlock
            for (int portIndex = 0; portIndex < m_pluginNumInputPorts; ++portIndex) {
                if (m_jackAudioInputPorts[portIndex] != nullptr) {
                    jack_default_audio_sample_t *portBuffer = static_cast<jack_default_audio_sample_t *>(jack_port_get_buffer(m_jackAudioInputPorts[portIndex], nframes));
                    jack_default_audio_sample_t *audioBuffer = m_juceAudioBuffer->getWritePointer(portIndex);
                    memcpy(audioBuffer, portBuffer, nframes * sizeof(jack_default_audio_sample_t));
                }
            }
            // Send the audio and midi data to plugin to process
            m_plugin->processBlock(*m_juceAudioBuffer, *m_juceMidiBuffer);
            // Send the processed audio output to respective ports
            for (int portIndex = 0; portIndex < m_pluginNumOutputPorts; ++portIndex) {
                jack_default_audio_sample_t *portBuffer = static_cast<jack_default_audio_sample_t *>(jack_port_get_buffer(m_jackAudioOutputPorts[portIndex], nframes));
                const jack_default_audio_sample_t *audioBuffer = m_juceAudioBuffer->getReadPointer(portIndex);
                memcpy(portBuffer, audioBuffer, nframes * sizeof(jack_default_audio_sample_t));
            }
        }
        return 0;
    }
};

static int jackProcessCallback(jack_nframes_t nframes, void* arg) {
    PluginHostPrivate *obj = static_cast<PluginHostPrivate*>(arg);
    return obj->pluginProcessCallback(nframes);
}

JuceEventLoop *PluginHost::juceEventLoop{nullptr};
PluginHost::PluginHost(QString pluginIdentifier, QString jackClientName, QObject *parent)
    : QObject(parent)
    , d(new PluginHostPrivate(this, pluginIdentifier, jackClientName))
{
    if (PluginHost::juceEventLoop == nullptr) {
        qDebug() << "juceEventLoop not initialized. Initializing and starting event loop";
        PluginHost::juceEventLoop = new JuceEventLoop();
        PluginHost::juceEventLoop->start();
    } else {
        qDebug() << "juceEventLoop already initialized. Skipping";
    }
}

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
        const auto parameterList = d->m_plugin->getParameters();
        static const juce::String programParameterName{"Program"};
        static const int numberOfPrograms{d->m_plugin->getNumPrograms()};
        for (auto *juceParameter : parameterList) {
            Parameter *parameter{nullptr};
            if (juceParameter->getAllValueStrings().size() > 0) {
                // Test for String Parameter
                parameter = new StringParameter(juceParameter, this);
                parameters << parameter;
            } else if (juceParameter->getNumSteps() == 2) {
                // Test for boolean parameter
                parameter = new BooleanParameter(juceParameter, this);
                parameters << parameter;
            } else {
                // If all tests fails, make it a generic parameter
                parameter = new Parameter(juceParameter, this);
                parameters << parameter;
            }
            if (juceParameter == d->m_plugin->getBypassParameter()) {
                parameter->m_isBypassParameter = true;
            } else if (numberOfPrograms > 0 && juceParameter->getParameterIndex() == parameterList.size() - 1 && juceParameter->getName(INT_MAX) == programParameterName) {
                // dirty heuristic, but... it matches the logic in the vst3 plugin code
                parameter->m_isProgramParameter = true;
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
