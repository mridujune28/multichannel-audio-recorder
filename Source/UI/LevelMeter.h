#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

/**
 * Horizontal peak/RMS level meter with a latching clip indicator.
 *
 * The owner feeds it fresh linear levels (0..1+) from its own poll timer via
 * pushLevels(); the meter applies display ballistics (instant attack, smooth
 * exponential decay) and repaints itself. Zones follow broadcast convention:
 * green below -12 dBFS, yellow -12..-3 dBFS, red above.
 */
class LevelMeter : public juce::Component
{
public:
    LevelMeter() = default;

    /** Feed the latest measured levels; call regularly (e.g. 30 Hz). */
    void pushLevels(float newPeak, float newRms, bool newClip)
    {
        // Instant attack, exponential release.
        displayedPeak = juce::jmax(newPeak, displayedPeak * decayFactor);
        displayedRms  = juce::jmax(newRms,  displayedRms  * decayFactor);

        if (newClip)
            clipLatchedUntil = juce::Time::getMillisecondCounter() + clipHoldMs;

        repaint();
    }

    /** Clears the meter and the clip latch (e.g. when a recording starts). */
    void reset()
    {
        displayedPeak = displayedRms = 0.0f;
        clipLatchedUntil = 0;
        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();

        const float ledSize = bounds.getHeight();
        auto clipArea = bounds.removeFromRight(ledSize).reduced(ledSize * 0.2f);
        bounds.removeFromRight(4.0f);

        // Bar background
        g.setColour(juce::Colour(0xff1a1a1a));
        g.fillRoundedRectangle(bounds, 3.0f);

        const float peakX = levelToX(displayedPeak, bounds);
        const float rmsX  = levelToX(displayedRms, bounds);

        // Zone boundaries in x
        const float yellowX = dbToX(-12.0f, bounds);
        const float redX    = dbToX(-3.0f, bounds);

        auto drawSegmented = [&g, &bounds, yellowX, redX](float untilX, float alpha)
        {
            g.saveState();
            g.reduceClipRegion(juce::Rectangle<float>(bounds.getX(), bounds.getY(),
                                                       untilX - bounds.getX(),
                                                       bounds.getHeight()).toNearestInt());
            g.setColour(juce::Colours::limegreen.withAlpha(alpha));
            g.fillRoundedRectangle(bounds.withRight(yellowX), 3.0f);
            g.setColour(juce::Colours::yellow.withAlpha(alpha));
            g.fillRect(juce::Rectangle<float>::leftTopRightBottom(yellowX, bounds.getY(), redX, bounds.getBottom()));
            g.setColour(juce::Colours::red.withAlpha(alpha));
            g.fillRect(juce::Rectangle<float>::leftTopRightBottom(redX, bounds.getY(), bounds.getRight(), bounds.getBottom()));
            g.restoreState();
        };

        drawSegmented(peakX, 0.4f);   // peak: translucent
        drawSegmented(rmsX, 1.0f);    // rms: solid

        // Peak tick mark
        g.setColour(juce::Colours::white.withAlpha(0.8f));
        g.fillRect(juce::Rectangle<float>(peakX - 1.0f, bounds.getY(), 2.0f, bounds.getHeight()));

        // Clip LED
        const bool clipOn = juce::Time::getMillisecondCounter() < clipLatchedUntil;
        g.setColour(clipOn ? juce::Colours::red : juce::Colour(0xff3a1010));
        g.fillEllipse(clipArea);
        g.setColour(juce::Colours::black);
        g.drawEllipse(clipArea, 1.0f);
    }

private:
    static constexpr float minDb = -60.0f;
    static constexpr float decayFactor = 0.82f;
    static constexpr juce::uint32 clipHoldMs = 2000;

    static float dbToX(float db, juce::Rectangle<float> area)
    {
        const float norm = juce::jlimit(0.0f, 1.0f, (db - minDb) / -minDb);
        return area.getX() + norm * area.getWidth();
    }

    static float levelToX(float linear, juce::Rectangle<float> area)
    {
        return dbToX(linear > 0.0f ? juce::Decibels::gainToDecibels(linear, minDb) : minDb, area);
    }

    float displayedPeak = 0.0f;
    float displayedRms = 0.0f;
    juce::uint32 clipLatchedUntil = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LevelMeter)
};
