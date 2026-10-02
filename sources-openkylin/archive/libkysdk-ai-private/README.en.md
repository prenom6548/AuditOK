# libkyai-config

#### Description
This repository provides Kylin AI configuration and assistant libraries, including:
- `kyai-config`: model configuration and config service proxy
- `kyai-assistant`: assistant service proxy and OS Assistant integration

#### Software Architecture
- `ai-config/`: config library implementation and install files
- `ai-assistant/`: assistant library implementation and install files
- `debian/`: packaging configuration

#### Dependencies
- CMake >= 3.5
- C++17 compiler
- `jsoncpp`
- `glib` / `gio-unix-2.0`
- `KylinAiProto`

#### Build & Install
1. Generate build directory
2. Build
3. Install

```bash
cmake -S . -B build
cmake --build build
cmake --install build
```

#### Build Options
- `BUILD_AI_CONFIG`: build `kyai-config` (default `ON`)
- `BUILD_AI_ASSISTANT`: build `kyai-assistant` (default `ON`)
- `ENABLE_TEST`: enable tests (default `OFF`)

Example: build only `kyai-config`
```bash
cmake -S . -B build -DBUILD_AI_ASSISTANT=OFF
cmake --build build
```

#### Usage
- Headers are installed under `include/kylin-ai/`
- Libraries are installed into the system library directory (e.g. `lib`)

#### Contribution
1. Fork the repository
2. Create a `Feat_xxx` branch
3. Commit your code
4. Create a Pull Request
