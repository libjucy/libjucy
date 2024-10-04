#pragma once

#include <QObject>
#include <QString>

class PluginDescription : public QObject {
    Q_OBJECT
public:
    explicit PluginDescription(QObject* parent = nullptr) : QObject(parent) {}

    /** The name of the plug-in. */
    QString name;

    /** A more descriptive name for the plug-in.
        This may be the same as the 'name' field, but some plug-ins may provide an
        alternative name.
    */
    QString descriptiveName;

    /** The plug-in format, e.g. "VST", "AudioUnit", etc. */
    QString pluginFormatName;

    /** A category, such as "Dynamics", "Reverbs", etc. */
    QString category;

    /** Either the file containing the plug-in module, or some other unique way
        of identifying it.

        E.g. for an AU, this would be an ID string that the component manager
        could use to retrieve the plug-in. For a VST, it's the file path.
    */
    QString fileOrIdentifier;

    /** True if the plug-in identifies itself as a synthesiser. */
    bool isInstrument = false;

    /** The number of inputs. */
    int numInputChannels = 0;

    /** The number of outputs. */
    int numOutputChannels = 0;
};
