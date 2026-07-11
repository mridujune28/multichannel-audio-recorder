#pragma once

#include <juce_core/juce_core.h>

/**
 * Builds recording output paths following the product's file-management rules:
 *
 *   <baseDir>/YYYY/MM/DD/<ChannelName>_YYYYMMDD_HHMMSS.wav
 *
 * e.g.  Recordings/2026/07/11/StudioA_20260711_090000.wav
 */
namespace RecordingPathBuilder
{
    /** Returns the default root folder for recordings (created on demand). */
    inline juce::File getDefaultRecordingsRoot()
    {
        return juce::File::getSpecialLocation(juce::File::userMusicDirectory)
                   .getChildFile("MultiChannelRecorder")
                   .getChildFile("Recordings");
    }

    /**
     * Builds (and creates the directories for) a dated output file.
     * The channel name is sanitised so it is always a legal filename.
     */
    inline juce::File buildRecordingFile(const juce::File& baseDir,
                                          const juce::String& channelName,
                                          const juce::String& extension = ".wav")
    {
        const auto now = juce::Time::getCurrentTime();

        auto dir = baseDir.getChildFile(now.formatted("%Y"))
                          .getChildFile(now.formatted("%m"))
                          .getChildFile(now.formatted("%d"));
        dir.createDirectory();

        auto legalName = juce::File::createLegalFileName(channelName.trim());
        if (legalName.isEmpty())
            legalName = "Channel1";

        return dir.getChildFile(legalName + now.formatted("_%Y%m%d_%H%M%S") + extension);
    }
}
