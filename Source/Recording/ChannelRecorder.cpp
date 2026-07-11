#include "ChannelRecorder.h"

ChannelRecorder::ChannelRecorder()
{
    writerThread.startThread();
}

ChannelRecorder::~ChannelRecorder()
{
    stopRecording();
    writerThread.stopThread(2000);
}

bool ChannelRecorder::startRecording(const juce::File& outputFile)
{
    stopRecording();

    if (sampleRate <= 0.0 || deviceInputChannels <= 0)
        return false;

    outputFile.getParentDirectory().createDirectory();
    outputFile.deleteFile();

    auto stream = outputFile.createOutputStream();
    if (stream == nullptr)
        return false;

    juce::WavAudioFormat wavFormat;
    // 16-bit for V1; per-channel bit-depth selection arrives in V3.
    std::unique_ptr<juce::OutputStream> ownedStream(std::move(stream));
    auto writer = wavFormat.createWriterFor(ownedStream,
                                             juce::AudioFormatWriterOptions{}
                                                 .withSampleRate(sampleRate)
                                                 .withNumChannels(deviceInputChannels)
                                                 .withBitsPerSample(16));
    if (writer == nullptr)
        return false;

    threadedWriter = std::make_unique<juce::AudioFormatWriter::ThreadedWriter>(
        writer.release(), writerThread, 65536);

    samplesRecorded.store(0);
    clipped.store(false);
    currentFile = outputFile;

    {
        const juce::ScopedLock sl(writerLock);
        activeWriter.store(threadedWriter.get());
    }

    return true;
}

void ChannelRecorder::stopRecording()
{
    // Detach the writer from the audio callback first, then destroy it: the
    // destructor flushes the FIFO and finalises the WAV header.
    {
        const juce::ScopedLock sl(writerLock);
        activeWriter.store(nullptr);
    }
    threadedWriter.reset();
}

double ChannelRecorder::getRecordedSeconds() const noexcept
{
    return sampleRate > 0.0
               ? static_cast<double>(samplesRecorded.load()) / sampleRate
               : 0.0;
}

juce::File ChannelRecorder::getCurrentFile() const
{
    return currentFile;
}

void ChannelRecorder::audioDeviceAboutToStart(juce::AudioIODevice* device)
{
    sampleRate = device->getCurrentSampleRate();
    deviceInputChannels = device->getActiveInputChannels().countNumberOfSetBits();
}

void ChannelRecorder::audioDeviceStopped()
{
    // Device is going away (or being reconfigured); a live writer's format no
    // longer matches, so a recording in progress must end here. The UI layer
    // observes the device manager and reports this to the user.
    stopRecording();
    sampleRate = 0.0;
    deviceInputChannels = 0;
}

void ChannelRecorder::audioDeviceIOCallbackWithContext(const float* const* inputChannelData,
                                                        int numInputChannels,
                                                        float* const* outputChannelData,
                                                        int numOutputChannels,
                                                        int numSamples,
                                                        const juce::AudioIODeviceCallbackContext&)
{
    // Metering: accumulate peak and RMS across every active input channel.
    float blockPeak = 0.0f;
    double sumSquares = 0.0;
    int counted = 0;

    for (int ch = 0; ch < numInputChannels; ++ch)
    {
        if (const float* data = inputChannelData[ch])
        {
            for (int i = 0; i < numSamples; ++i)
            {
                const float sample = data[i];
                const float mag = std::abs(sample);
                blockPeak = juce::jmax(blockPeak, mag);
                sumSquares += static_cast<double>(sample) * sample;
            }
            counted += numSamples;
        }
    }

    // Publish max-since-last-UI-read so short transients are never missed.
    float expected = peakLevel.load();
    while (blockPeak > expected && ! peakLevel.compare_exchange_weak(expected, blockPeak)) {}

    if (counted > 0)
    {
        const auto blockRms = static_cast<float>(std::sqrt(sumSquares / counted));
        float expectedRms = rmsLevel.load();
        while (blockRms > expectedRms && ! rmsLevel.compare_exchange_weak(expectedRms, blockRms)) {}
    }

    if (blockPeak >= 0.999f)
        clipped.store(true);

    // Hand samples to the background writer. Try-lock only: if the message
    // thread is mid start/stop we drop this block rather than ever blocking
    // the audio thread.
    {
        const juce::ScopedTryLock sl(writerLock);
        if (sl.isLocked())
        {
            if (auto* writer = activeWriter.load())
            {
                writer->write(inputChannelData, numSamples);
                samplesRecorded.fetch_add(numSamples);
            }
        }
    }

    // No monitoring in V1: keep outputs silent.
    for (int ch = 0; ch < numOutputChannels; ++ch)
        if (outputChannelData[ch] != nullptr)
            juce::FloatVectorOperations::clear(outputChannelData[ch], numSamples);
}
