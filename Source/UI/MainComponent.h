#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include "../Recording/ChannelRecorder.h"
#include "LevelMeter.h"

/**
 * V1 main window: one recording channel.
 *
 *  - input device / sample rate selection (AudioDeviceSelectorComponent)
 *  - channel name, Record / Stop, Open Recording Folder
 *  - live peak/RMS meter with clip indicator
 *  - recording duration and output file display
 */
class MainComponent : public juce::Component,
                       private juce::Timer,
                       private juce::ChangeListener
{
public:
    MainComponent();
    ~MainComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void timerCallback() override;
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    void startRecording();
    void stopRecording(const juce::String& reason = {});
    void updateButtonStates();

    juce::AudioDeviceManager deviceManager;
    ChannelRecorder recorder;

    std::unique_ptr<juce::AudioDeviceSelectorComponent> deviceSelector;

    juce::Label titleLabel;
    juce::Label channelNameCaption;
    juce::TextEditor channelNameEditor;
    juce::TextButton recordButton { "Record" };
    juce::TextButton stopButton { "Stop" };
    juce::TextButton openFolderButton { "Open Recording Folder" };
    LevelMeter meter;
    juce::Label durationLabel;
    juce::Label fileLabel;
    juce::Label statusLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
