#ifndef JucyJuceEventLoop
#define JucyJuceEventLoop

#include <QDebug>

class JuceEventLoop : public juce::Thread {
public:
    JuceEventLoop()
        : Thread("Juce EventLoop Thread")
        , initializer(new juce::ScopedJuceInitialiser_GUI())
    {}

    void run() override {
        juce::MessageManager::getInstance()->runDispatchLoop();
    }

    void start() {
        ++startCount;
        if (startCount == 1) {
            qDebug() << Q_FUNC_INFO << "Starting the Jucy event loop";
            startThread();
        }
    }

    void stop() {
        --startCount;
        if (startCount == 0) {
            qDebug() << Q_FUNC_INFO << "Stopping the Jucy event loop";
            juce::MessageManager::getInstance()->stopDispatchLoop();
            stopThread(500);
        }
    }

private:
    juce::ScopedJuceInitialiser_GUI *initializer{nullptr};
    int startCount{0};
};

#endif//JucyJuceEventLoop
