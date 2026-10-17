cmake_minimum_required(VERSION 3.22)
project(CyberGodVocal VERSION 2.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# 1. Fetch JUCE 8
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.4
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(JUCE)

# 2. Fetch Signalsmith Stretch (with its Signalsmith Linear FFT dependency)
FetchContent_Declare(
    signalsmith_stretch
    GIT_REPOSITORY https://github.com/Signalsmith-Audio/signalsmith-stretch.git
    GIT_TAG        main
)
FetchContent_MakeAvailable(signalsmith_stretch)

# 3. Build VST3 + Standalone App (MIDI Input Enabled for Note Override)
juce_add_plugin(CyberGodVocal
    COMPANY_NAME "VII Audio"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    FORMATS VST3 Standalone
    PRODUCT_NAME "CyberGodVocal"
)

target_sources(CyberGodVocal
    PRIVATE
        PluginProcessor.cpp
        PluginProcessor.h
)

target_include_directories(CyberGodVocal
    PRIVATE
        ${signalsmith_stretch_SOURCE_DIR}
)

target_compile_definitions(CyberGodVocal
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(CyberGodVocal
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_gui_basics
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
