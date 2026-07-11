# Multi-Channel Audio Recorder Pro

Cross-platform (macOS + Windows) professional multi-channel audio recorder.
Records up to 8 independent audio input streams simultaneously, each to its
own file, for broadcast logging, podcast studios, and similar use cases.

**Status: V0 — build scaffold only.** No audio recording yet. This proves
the cross-platform CMake + JUCE pipeline builds cleanly on both macOS and
Windows before any audio code is written. See [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md)
and the roadmap below.

## Tech stack

- **Language:** C++17
- **Framework:** [JUCE](https://juce.com) (audio I/O, GUI, audio file formats)
- **Build system:** CMake, JUCE fetched via `FetchContent` (no manual setup)
- **CI:** GitHub Actions, matrix build on `macos-latest` and `windows-latest`,
  producing a downloadable `.app` / `.exe` artifact on every push

## Building locally (optional — CI builds this for you on every push)

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

The built app/exe will be under `build/MultiChannelAudioRecorder_artefacts/Release/`.
You do not need to build locally — every push to `main` triggers a CI build;
grab the artifact from the **Actions** tab on GitHub.

## Important caveats

- **JUCE licensing:** JUCE is dual-licensed (GPL3 / commercial). This repo
  uses it under the open-source terms. If this project is ever distributed
  as closed-source binaries, a
  [JUCE commercial license](https://juce.com/get-juce) is required.
- **ASIO on Windows:** the ASIO SDK is licensed by Steinberg and cannot be
  redistributed in this repo. WASAPI (shared/exclusive) and DirectSound work
  out of the box with no extra setup. To enable ASIO, download the SDK from
  Steinberg and point the build at it locally (documented once ASIO support
  lands — not yet implemented).
- **macOS code signing:** CI builds are **unsigned** (no Apple Developer
  certificate configured). Gatekeeper will warn on first launch; right-click
  → Open bypasses it. Proper notarization requires enrolling in the Apple
  Developer Program.
- **Multi-channel input on macOS:** Core Audio doesn't have a direct
  equivalent of Windows' WASAPI/ASIO/DirectSound/MME device model. To
  capture more channels than one interface provides, create a Core Audio
  **aggregate device** in Audio MIDI Setup — the app selects among whatever
  devices macOS exposes.

## Roadmap

| Phase | Scope |
|---|---|
| V0 | ✅ Repo scaffold, CMake+JUCE bootstrap, CI matrix building a trivial window on both OSes |
| V1 | Single channel, WAV only, one device, start/stop, basic meter |
| V2 | Up to 8 channels, independent per-channel device/name/folder config |
| V3 | MP3 encoding + bitrate options, full sample-rate/bit-depth matrix |
| V4 | Duration limits, auto-split by time/size, seamless continuous rollover |
| V5 | Settings window, themes, JSON-persisted defaults |
| V6 | Error handling (device removal, disk full, permissions), log polish, soak testing |
| V7 (stretch) | Interface stubs for future network audio / REST API expansion — design only |

Each phase lands as its own pull request.
