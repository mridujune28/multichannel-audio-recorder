#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

/**
 * V0 placeholder component.
 * Proves the cross-platform JUCE build pipeline before any audio code lands.
 * Replaced with real channel strips / meters / toolbar starting in V1.
 */
class MainComponent : public juce::Component
{
public:
    MainComponent();

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    juce::Label titleLabel;
    juce::Label statusLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
