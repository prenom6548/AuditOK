# KylinAgent

An endpoint-side operating system AI agent desktop application built with **Qt5 C++**.

## Overview

KylinAgent communicates with the agent runtime backend through a local HTTP API, providing real-time streaming chat, session management, model configuration, and more.

## Documentation

Project documentation except this README is organized under `docs/`:

- `docs/INDEX.md` — documentation index
- `docs/DOCUMENTATION_CN.md` — full Chinese documentation
- `docs/AGENT_RUNTIME_INTEGRATION_CN.md.md` — backend integration notes

## Features

- **Real-time Streaming Chat** — SSE-based streaming responses with OpenAI delta and segment formats
- **Session Management** — Create, search, rename, delete sessions with per-session state persistence
- **Multi-Model Support** — 8 AI providers: OpenRouter, Anthropic, OpenAI, LM Studio, Ollama, vLLM, llama.cpp, Custom
- **Frameless UI** — Custom title bar, draggable window, sidebar toggle, pin-to-edge mode
- **Settings System** — 8 categories: Profile, Models, Skills, Persona, Memory, Tools, Schedules, General
- **SQLite Storage** — Local session & message persistence with FTS5 full-text search
- **Per-Session State** — Independent scroll position, input content, and streaming state for each session

## Build Dependencies

This project uses **CMake** with **Qt5** and requires the following Qt5 modules:
`Core`, `Gui`, `Widgets`, `Sql` (with SQLite driver), `Network`, `Xml`, `Svg`, `WebSockets`, `Multimedia`.

### openKylin
```bash
sudo apt install build-essential cmake zlib1g-dev qtbase5-dev libqt5sql5-sqlite libqt5svg5-dev libqt5websockets5-dev qtmultimedia5-dev libkysdk-qtwidgets-dev libgsettings-qt-dev
```

| Package | Provides |
|---|---|
| `build-essential` | C++17 compiler (g++), make |
| `cmake` | Build system (≥ 3.10) |
| `zlib1g-dev` | ZLIB headers, library & CMake config used by `ZLIB::ZLIB` |
| `qtbase5-dev` | Qt5 Core, Gui, Widgets, Sql, Network, Xml — headers, libs & CMake configs |
| `libqt5websockets5-dev` | Qt5 WebSockets headers, library & CMake config |
| `qtmultimedia5-dev` | Qt5 Multimedia headers, library & CMake config |
| `libqt5sql5-sqlite` | Qt SQLite driver plugin (required at runtime for `state.db`) |
| `libqt5svg5-dev` | Qt5 Svg module — headers, libs & CMake configs |

To build a Debian package with `scripts/build_and_package.sh`, install the packaging tools as well:

```bash
sudo apt install dpkg-dev dpkg rsync tar
```

| Package | Provides |
|---|---|
| `dpkg-dev` | `dpkg-shlibdeps` for generated package dependencies |
| `dpkg` | `dpkg`, `dpkg-deb` |
| `rsync` | Packaging workspace file sync |
| `tar` | Archive handling used by the package workflow |

> **Note**: If CMake reports a missing `Qt5WebSocketsConfig.cmake`, `Qt5SvgConfig.cmake`, or `Qt5MultimediaConfig.cmake`, install the matching development package: `libqt5websockets5-dev`, `libqt5svg5-dev`, or `qtmultimedia5-dev`.

## Build & Run

### CMake
```bash
mkdir build && cd build
cmake ..
make
./kylin-agent
```

> **Note**: Requires Qt 5.12+, CMake 3.10+, and C++17 compiler.
