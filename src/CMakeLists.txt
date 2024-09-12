set(TYPESYSTEM_FILE ${CMAKE_CURRENT_SOURCE_DIR}/plugin_typesystem.xml)

add_library(jucy SHARED)

target_sources(jucy
    PRIVATE
        PluginHost.cpp
        LV2PluginHost.cpp
        VST3PluginHost.cpp
        Parameter.cpp
        StringParameter.cpp
        BooleanParameter.cpp
)
target_compile_definitions(jucy
    PUBLIC
        JUCE_PLUGINHOST_AU=1
        JUCE_PLUGINHOST_LADSPA=1
        JUCE_PLUGINHOST_VST3=1
        JUCE_PLUGINHOST_LV2=1
        JUCE_USE_CURL=0
        JUCE_WEB_BROWSER=0
        JUCER_ENABLE_GPL_MODE=1
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_REPORT_APP_USAGE=0
        JUCE_STRICT_REFCOUNTEDPOINTER=1
        JUCE_DSP_USE_SHARED_FFTW=1
        JUCE_JACK=0
        JUCE_ALSA=0
        JUCE_FORCE_DEBUG=0
    INTERFACE
        $<TARGET_PROPERTY:jucy,COMPILE_DEFINITIONS>
)

target_link_libraries(jucy
    PRIVATE
        ${Jack_LIBRARIES}
        juce::juce_core
        juce::juce_events
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_processors
        juce::juce_audio_utils
        "-latomic"
        "-lcurl"
        "-lfftw3"
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
        Qt5::Core
)

target_include_directories(jucy
    PRIVATE
        ${Jack_INCLUDE_DIRS}
    INTERFACE
        $<TARGET_PROPERTY:jucy,INCLUDE_DIRECTORIES>
)

install(
    TARGETS jucy
    LIBRARY DESTINATION lib
)
