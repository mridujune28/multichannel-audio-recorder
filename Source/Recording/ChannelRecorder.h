#pragma once

#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_formats/juce_audio_formats.h>

/**
 * Records the incoming audio of one device to a WAV file.
 *
 * Real-time safety: the audio callback never allocates or performs disk I/O.
 * Samples are handed to a juce::AudioFormatWriter::ThreadedWriter, whose
 * internal FIFO is drained by a dedicated background TimeSliceThread, so a
 * slow disk can never glitch the capture path.
 *
 * Level metering (peak / RMS / clip) is published through atomics that the
 * UI thread polls; the audio thread only ever stores to them.
 */
class ChannelRecorder : public juce::AudioIODeviceCallback
{
public:
    ChannelRecorder();
    ~ChannelRecorder() override;

    /**
     * Starts recording to the given file. Returns false (and records nothing)
     * if the device is not running or has no active input channels.
     */
    bool startRecording(const juce::File& outputFile);

    /** Stops recording and flushes/closes the file. Safe to call when idle. */
    void stopRecording();

    bool isRecording() const noexcept { return activeWriter.load() != nullptr; }

    /** Seconds of audio written to the current/last file. */
    double getRecordedSeconds() const noexcept;

    juce::File getCurrentFile() const;

    /** Reads and resets the peak level accumulated since the last call (0..1+). */
    float readAndResetPeak() noexcept { return peakLevel.exchange(0.0f); }

    /** Reads and resets the RMS level accumulated since the last call. */
    float readAndResetRms() noexcept { return rmsLevel.exchange(0.0f); }

    /** True if any sample reached full scale since the last reset. */
    bool readAndResetClip() noexcept { return clipped.exchange(false); }

    //==============================================================================
    void audioDeviceAboutToStart(juce::AudioIODevice* device) override;
    void audioDeviceStopped() override;
    void audioDeviceIOCallbackWithContext(const float* const* inputChannelData,
                                           int numInputChannels,
                                           float* const* outputChannelData,
                                           int numOutputChannels,
                                           int numSamples,
                                           const juce::AudioIODeviceCallbackContext& context) override;

private:
    juce::TimeSliceThread writerThread { "Disk Writer" };
    std::unique_ptr<juce::AudioFormatWriter::ThreadedWriter> threadedWriter;

    // Guards the writer swap between the message thread (start/stop) and the
    // audio thread (write). The audio thread only ever try-locks, so it can
    // never block on the message thread.
    juce::CriticalSection writerLock;
    std::atomic<juce::AudioFormatWriter::ThreadedWriter*> activeWriter { nullptr };

    double sampleRate = 0.0;
    int deviceInputChannels = 0;

    std::atomic<int64_t> samplesRecorded { 0 };
    std::atomic<float> peakLevel { 0.0f };
    std::atomic<float> rmsLevel { 0.0f };
    std::atomic<bool> clipped { false };

    juce::File currentFile;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChannelRecorder)
};
