#include "MainComponent.h"
#include "../Utilities/RecordingPathBuilder.h"

MainComponent::MainComponent()
{
    titleLabel.setText("Multi-Channel Audio Recorder Pro", juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setFont(juce::Font(juce::FontOptions(22.0f, juce::Font::bold)));
    addAndMakeVisible(titleLabel);

    // Ask for microphone access before opening the device. On desktop the
    // callback fires synchronously; on macOS the OS shows its consent dialog
    // the first time the input device actually starts.
    juce::RuntimePermissions::request(juce::RuntimePermissions::recordAudio,
        [this](bool granted)
        {
            const auto error = deviceManager.initialise(granted ? 2 : 0, 0, nullptr, true);
            if (error.isNotEmpty())
                statusLabel.setText("Audio device error: " + error, juce::dontSendNotification);
        });

    deviceManager.addAudioCallback(&recorder);
    deviceManager.addChangeListener(this);

    deviceSelector = std::make_unique<juce::AudioDeviceSelectorComponent>(
        deviceManager,
        1, 2,     // input channels min/max
        0, 0,     // no outputs in V1 (record-only)
        false, false,
        true,     // channels shown as stereo pairs
        false);   // keep advanced options (sample rate / buffer size) visible
    addAndMakeVisible(*deviceSelector);

    channelNameCaption.setText("Channel name", juce::dontSendNotification);
    addAndMakeVisible(channelNameCaption);

    channelNameEditor.setText("Channel1");
    channelNameEditor.setInputRestrictions(60);
    addAndMakeVisible(channelNameEditor);

    recordButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xffb01818));
    recordButton.onClick = [this] { startRecording(); };
    addAndMakeVisible(recordButton);

    stopButton.onClick = [this] { stopRecording(); };
    addAndMakeVisible(stopButton);

    openFolderButton.onClick = []
    {
        auto root = RecordingPathBuilder::getDefaultRecordingsRoot();
        root.createDirectory();
        root.startAsProcess();
    };
    addAndMakeVisible(openFolderButton);

    addAndMakeVisible(meter);

    durationLabel.setText("00:00:00", juce::dontSendNotification);
    durationLabel.setFont(juce::Font(juce::FontOptions(20.0f, juce::Font::bold)));
    durationLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(durationLabel);

    fileLabel.setJustificationType(juce::Justification::centredLeft);
    fileLabel.setColour(juce::Label::textColourId, juce::Colours::grey);
    addAndMakeVisible(fileLabel);

    statusLabel.setText("Idle", juce::dontSendNotification);
    addAndMakeVisible(statusLabel);

    updateButtonStates();
    startTimerHz(30);
    setSize(680, 560);
}

MainComponent::~MainComponent()
{
    stopTimer();
    deviceManager.removeChangeListener(this);
    recorder.stopRecording();
    deviceManager.removeAudioCallback(&recorder);
}

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void MainComponent::resized()
{
    auto bounds = getLocalBounds().reduced(16);

    titleLabel.setBounds(bounds.removeFromTop(34));
    bounds.removeFromTop(8);

    deviceSelector->setBounds(bounds.removeFromTop(230));
    bounds.removeFromTop(12);

    auto nameRow = bounds.removeFromTop(28);
    channelNameCaption.setBounds(nameRow.removeFromLeft(110));
    channelNameEditor.setBounds(nameRow.removeFromLeft(220));
    durationLabel.setBounds(nameRow);
    bounds.removeFromTop(10);

    meter.setBounds(bounds.removeFromTop(26));
    bounds.removeFromTop(12);

    auto buttonRow = bounds.removeFromTop(34);
    recordButton.setBounds(buttonRow.removeFromLeft(110));
    buttonRow.removeFromLeft(8);
    stopButton.setBounds(buttonRow.removeFromLeft(110));
    buttonRow.removeFromLeft(8);
    openFolderButton.setBounds(buttonRow.removeFromLeft(190));
    bounds.removeFromTop(10);

    fileLabel.setBounds(bounds.removeFromTop(24));
    statusLabel.setBounds(bounds.removeFromTop(24));
}

void MainComponent::timerCallback()
{
    meter.pushLevels(recorder.readAndResetPeak(),
                      recorder.readAndResetRms(),
                      recorder.readAndResetClip());

    if (recorder.isRecording())
    {
        const auto totalSeconds = static_cast<int>(recorder.getRecordedSeconds());
        durationLabel.setText(juce::String::formatted("%02d:%02d:%02d",
                                                       totalSeconds / 3600,
                                                       (totalSeconds / 60) % 60,
                                                       totalSeconds % 60),
                               juce::dontSendNotification);
    }
}

void MainComponent::changeListenerCallback(juce::ChangeBroadcaster*)
{
    // Fired when the device setup changes. If the device disappeared or was
    // reconfigured mid-recording, the recorder has already closed its file
    // safely (audioDeviceStopped) — reflect that in the UI.
    if (! recorder.isRecording() && stopButton.isEnabled())
        stopRecording("Recording stopped: audio device changed or was removed.");
}

void MainComponent::startRecording()
{
    const auto file = RecordingPathBuilder::buildRecordingFile(
        RecordingPathBuilder::getDefaultRecordingsRoot(),
        channelNameEditor.getText());

    if (! recorder.startRecording(file))
    {
        statusLabel.setText("Could not start recording - check that an input device "
                             "with at least one enabled channel is selected.",
                             juce::dontSendNotification);
        return;
    }

    meter.reset();
    durationLabel.setText("00:00:00", juce::dontSendNotification);
    fileLabel.setText("Recording to: " + file.getFullPathName(), juce::dontSendNotification);
    statusLabel.setText("Recording", juce::dontSendNotification);
    updateButtonStates();
}

void MainComponent::stopRecording(const juce::String& reason)
{
    recorder.stopRecording();

    const auto file = recorder.getCurrentFile();
    if (file != juce::File())
        fileLabel.setText("Saved: " + file.getFullPathName(), juce::dontSendNotification);

    statusLabel.setText(reason.isNotEmpty() ? reason : "Idle", juce::dontSendNotification);
    updateButtonStates();
}

void MainComponent::updateButtonStates()
{
    const bool recording = recorder.isRecording();
    recordButton.setEnabled(! recording);
    channelNameEditor.setEnabled(! recording);
    deviceSelector->setEnabled(! recording);
    stopButton.setEnabled(recording);
}
