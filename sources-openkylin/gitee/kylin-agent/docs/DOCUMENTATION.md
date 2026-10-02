# Hermes Desktop — Complete Technical Documentation

## Table of Contents

1. [Architecture](#architecture)
2. [Detailed Features](#detailed-features)
3. [Workflow](#workflow)
4. [Key Technologies](#key-technologies)
5. [Settings System](#settings-system)
6. [Data Storage](#data-storage)
7. [API Protocol](#api-protocol)
8. [Dependencies Reference](#dependencies-reference)

---

## Architecture

### Layered Architecture

```
┌─────────────────────────────────────────────────────┐
│                   UI Layer                          │
│  MainWindow, Sidebar, ChatWidget, SettingsDialog    │
│  SessionListWidget, All*SettingsWidgets             │
├─────────────────────────────────────────────────────┤
│                 Service Layer                       │
│  ApiService, DatabaseService, SessionService,       │
│  ModelService                                       │
├─────────────────────────────────────────────────────┤
│                 Model Layer                         │
│  SavedModel, ModelProvider, SessionMessage,         │
│  SessionSummary, SearchResult                       │
└─────────────────────────────────────────────────────┘
```

### Module Dependency & Class Diagram

```
                 ┌──────────────┐
                 │   App        │ (QApplication subclass)
                 │  main.cpp    │
                 └──────┬───────┘
                        │ creates
                 ┌──────▼───────┐
                 │  MainWindow  │ (Frameless QMainWindow)
                 │              │
                 │ ┌──────────┐ │ ┌──────────────┐
                 │ │ Sidebar  │ │ │  ChatWidget  │
                 │ │          │◄├─┤              │
                 │ │250px wide│ │ │ MessagesArea │
                 │ │          │ │ │ InputArea    │
                 │ └────┬─────┘ │ │ ModelSelector│
                 │      │       │ └──────┬───────┘
                 └──────┼───────┘       │
                        │               │
              ┌─────────▼───┐   ┌───────▼────────┐
              │ SessionList │   │   ApiService   │
              │  Widget     │   │ (Singleton)    │
              │             │   │                │
              │ Streaming   │   │ SSE Streaming  │
              │ Animation   │   │ Chat with API  │
              │ Unread Dots │   └───────┬────────┘
              └─────────────┘           │
                                        │ HTTP calls to local
                                        │ Hermes Agent server
                                        │ (port 8642)
                                        ▼
                              ┌──────────────────┐
                              │  Hermes Agent    │
                              │  AI Backend API  │
                              └──────────────────┘

          ┌──────────────┐     ┌──────────────────┐
          │SessionService│◄───►│ DatabaseService  │
          │ (Singleton)  │     │ (Singleton)      │
          │              │     │                  │
          │ Creates UUIDs│     │ SQLite3 + FTS5   │
          │ Title extras │     │ full-text search │
          └──────────────┘     └──────────────────┘
                                       │
                                       ▼
                               ┌──────────────────┐
                               │  ~/.hermes/      │
                               │  state.db        │
                               │  models.json     │
                               │  SOUL.md         │
                               │  MEMORY.md       │
                               │  USER.md         │
                               │  skills/         │
                               └──────────────────┘

          ┌──────────────┐
          │ ModelService │
          │ (Singleton)  │
          │              │
          │ 8 providers: │
          │ OpenRouter,  │
          │ Anthropic,   │
          │ OpenAI,      │
          │ Custom,      │
          │ LM Studio,   │
          │ Ollama,      │
          │ vLLM,        │
          │ llama.cpp    │
          └──────────────┘
```

### Design Patterns

- **Singleton Pattern**: All service classes (ApiService, DatabaseService, SessionService, ModelService) use the singleton pattern with static `instance()` methods
- **Observer Pattern**: Qt's signal/slot mechanism is used extensively for decoupled communication between components
- **Model-View Pattern**: Data models (SavedModel, SessionSummary, etc.) are separate from UI components

---

## Detailed Features

### 1. Session Management

- **Create**: Via sidebar "New Session" button or by sending a message in a fresh chat
- **List**: Sorted by last activity (newest first), displayed in sidebar
- **Search**: FTS5 full-text search on message content
- **Rename**: Right-click context menu -> Rename
- **Delete**: Local SQLite deletion + remote API sync (via ApiService)
- **Per-session state**: Switching between sessions preserves scroll position, input content, and streaming state in memory (QMap<QString, SessionChatState>)

### 2. Chat Interface

- **SSE Streaming**: Real-time response streaming via Server-Sent Events
- **Two formats**: OpenAI-compatible delta format + Hermes Agent segment format
- **History**: Loads from local SQLite database
- **Model selector**: Combo box in the toolbar
- **Message bubbles**: User (right, green) / Assistant (left, blue) with avatars
- **Send**: Enter to send, Shift+Enter for newline
- **Export**: Downloadable session file
- **Cancel**: Stop button to cancel ongoing generation
- **Unread marking**: Background sessions that complete get marked unread

### 3. Model Management

- **CRUD**: Add/Edit/Delete models via settings dialog or dedicated model page
- **8 providers**: OpenRouter, Anthropic, OpenAI, Custom, LM Studio, Ollama, vLLM, llama.cpp
- **3 defaults** (auto-seeded on first launch):
  - Claude Sonnet 4 (OpenRouter): `anthropic/claude-sonnet-4-20250514`
  - Claude Sonnet 4 (Anthropic): `claude-sonnet-4-20250514`
  - GPT-4.1 (OpenAI): `gpt-4.1`
- **Duplicate detection**: Prevents adding duplicate models
- **Responsive grid**: Model cards adapt to window width (1 or 2 columns)

### 4. UI Features

- **Frameless window**: Custom title bar (minimize, maximize, close)
- **Drag to move**: Via title bar (mousePressEvent + mouseMoveEvent)
- **Sidebar toggle**: Hamburger menu button, state persisted via QSettings
- **Pin to right edge**: 600px wide, full height, restorable
- **Resizable panels**: QSplitter between sidebar and chat
- **Streaming animation**: Rotating dots spinner (12-frame animation, 80ms interval)
- **Green dot**: Unread session indicator
- **Export button**: Floating at bottom-right of messages area
- **Size grip**: Bottom-right corner for window resizing

---

## API Boundary

The desktop client should treat HTTP APIs as the boundary for generation and
remote state. Direct file access under `~/.hermes` is acceptable only for local
mode.

Expected backend capabilities include:

- Health checks through `/health`.
- Model discovery through `/v1/models`.
- Streaming chat through `/v1/chat/completions` or structured runs.
- Optional capability discovery through `/v1/capabilities`.

Remote mode should avoid reading a remote SQLite database directly. Session,
message, model, skill, persona, memory, log, and usage data should be provided
by explicit backend APIs.

## Storage

Local mode uses the Hermes data directory for assistant state:

| Path | Purpose |
|---|---|
| `~/.hermes/state.db` | Session and message storage |
| `~/.hermes/SOUL.md` | Persona text |
| `~/.hermes/MEMORY.md` | Long-term memory |
| `~/.hermes/USER.md` | User profile |
| `~/.hermes/skills/` | Local skills |

Kylin Agent keeps desktop model configuration under:

```text
~/.config/kylin-agent/models.json
```

When `$HERMES_HOME` is set, the desktop client should prefer that directory for
Hermes runtime files.

## Implementation Notes

- Prefer Qt signals and slots for cross-widget communication.
- Keep services as the place where widgets cross process, network, or database boundaries.
- Preserve per-session draft text, scroll position, and streaming state when switching sessions.
- Store API keys and provider secrets outside project files.
- Keep future remote-mode work behind service interfaces so local and remote data sources can coexist.
