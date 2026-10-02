cmake_minimum_required(VERSION 3.22)

project(PS VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(JUCE)

juce_add_gui_app(PS
    PRODUCT_NAME "PS"
    COMPANY_NAME "Pouria Motabean"
    ICON_BIG ""
    VERSION 1.0.0
)

juce_generate_juce_header(PS)

target_sources(PS PRIVATE
    Source/Main.cpp
    Source/MainComponent.cpp
    Source/LoudnessMeter.cpp
    Source/SpotifyProcessor.cpp
    Source/SegmentedControl.cpp
    Source/ResultsPanel.cpp
    Source/PSLookAndFeel.cpp
)

target_compile_definitions(PS PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:PS,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:PS,JUCE_VERSION>"
)

target_link_libraries(PS PRIVATE
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)
