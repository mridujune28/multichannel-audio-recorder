cmake_minimum_required(VERSION 3.22)

project(MultiChannelAudioRecorder VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.14
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(JUCE)

juce_add_gui_app(MultiChannelAudioRecorder
    PRODUCT_NAME    "MultiChannelAudioRecorder"
    COMPANY_NAME    "MultichannelAudioRecorder"
    BUNDLE_ID       "com.mridujune28.multichannelaudiorecorder"
)

target_sources(MultiChannelAudioRecorder
    PRIVATE
        Source/Main.cpp
        Source/UI/MainComponent.cpp
)

target_compile_definitions(MultiChannelAudioRecorder
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:MultiChannelAudioRecorder,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:MultiChannelAudioRecorder,JUCE_VERSION>"
)

target_link_libraries(MultiChannelAudioRecorder
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
