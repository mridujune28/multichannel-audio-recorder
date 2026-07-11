# Architecture

## Overview

Cross-platform (macOS + Windows) multi-channel audio recorder built on
[JUCE](https://juce.com) (C++17), configured with CMake so CI can build
headlessly on both operating systems without Xcode or Visual Studio.

## Module layout

```
Source/
  Main.cpp            Application entry point, top-level window
  UI/                 Views: channel strips, meters, toolbar, settings dialog
  Models/             Plain data types: ChannelConfig, RecordingSession, etc.
  Audio/              Device enumeration/selection wrapping juce::AudioDeviceManager
  Recording/          Per-channel capture pipeline: ring buffer -> writer thread
  Encoders/           WavAudioFormat wrapper, LAME MP3 encoder wrapper
  Services/           App-level orchestration (start/stop/pause all channels)
  Configuration/      JSON-backed settings persistence
  Logging/            Daily-rotating file logger
  Utilities/          Shared helpers (filename formatting, disk space checks, etc.)
```

## Audio pipeline (target design, built out phase by phase)

1. A single `juce::AudioIODeviceCallback` receives interleaved/multi-channel
   input from the active device.
2. Each enabled channel has its own lock-free ring buffer
   (`juce::AbstractFifo`) that the audio callback writes into — no locks,
   no allocation, no I/O on the audio thread.
3. A dedicated writer thread per channel drains its ring buffer and streams
   to disk via `juce::AudioFormatWriter` (WAV) or a LAME-backed MP3 encoder.
4. Auto-split (by time or size) closes the current writer and opens a new
   file without missing any buffered audio.

## Platform notes

- **macOS**: device I/O via Core Audio. Multi-channel capture beyond a
  single interface's channel count requires the user to set up a Core
  Audio **aggregate device** in Audio MIDI Setup — this app selects from
  whatever devices/aggregates macOS exposes, it doesn't create them itself
  (that could be added later via the Core Audio HAL APIs if needed).
- **Windows**: device I/O via WASAPI (shared/exclusive) and DirectSound
  out of the box. ASIO support requires the Steinberg ASIO SDK, which is
  **not redistributable** and is therefore not vendored in this repo — see
  README for how to add it locally if you need ASIO.
- **MME** (Windows) is legacy and low priority; can be enabled via JUCE's
  built-in support if actually needed later.

## Roadmap

See the phase table in the top-level README. Each phase is delivered as
its own PR so the diffs stay reviewable.
