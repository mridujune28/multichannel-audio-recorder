#include "MainComponent.h"

MainComponent::MainComponent()
{
    titleLabel.setText("Multi-Channel Audio Recorder Pro", juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setFont(juce::Font(24.0f, juce::Font::bold));
    addAndMakeVisible(titleLabel);

    statusLabel.setText("V0 scaffold \xe2\x80\x94 audio engine not yet implemented",
                         juce::dontSendNotification);
    statusLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(statusLabel);

    setSize(600, 300);
}

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void MainComponent::resized()
{
    auto bounds = getLocalBounds().reduced(20);
    titleLabel.setBounds(bounds.removeFromTop(40));
    statusLabel.setBounds(bounds.removeFromTop(30));
}
