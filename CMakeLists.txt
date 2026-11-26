cmake_minimum_required(VERSION 3.22)
project(RealLab VERSION 1.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.4
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(JUCE)

# Preset artwork, embedded in the plugin binary
file(GLOB RL_ASSETS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Assets/*.jpg)
juce_add_binary_data(RealLabAssets SOURCES ${RL_ASSETS})
set_target_properties(RealLabAssets PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

juce_add_plugin(RealLab
    COMPANY_NAME                "lendariorafael"
    PLUGIN_MANUFACTURER_CODE    Rlbz
    PLUGIN_CODE                 RlLb
    PRODUCT_NAME                "Real Lab"
    FORMATS                     VST3
    IS_SYNTH                    TRUE
    NEEDS_MIDI_INPUT            TRUE
    NEEDS_MIDI_OUTPUT           FALSE
    IS_MIDI_EFFECT              FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD     FALSE
    VST3_CATEGORIES             Instrument Synth Sampler)

juce_generate_juce_header(RealLab)

target_sources(RealLab PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp)

target_compile_definitions(RealLab PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0)

target_link_libraries(RealLab PRIVATE
    RealLabAssets
    juce::juce_audio_basics
    juce::juce_audio_formats
    juce::juce_audio_utils
    juce::juce_audio_plugin_client
    juce::juce_gui_basics
    juce::juce_gui_extra)
