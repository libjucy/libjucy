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
        startThread();
    }

    void stop() {
        stopThread(500);
    }

private:
    juce::ScopedJuceInitialiser_GUI *initializer{nullptr};
};
