# YouTube Summarizer

A C++ console app that chats with a local/cloud AI model via [Ollama](https://ollama.com). Eventually it will accept YouTube links and summarize transcripts automatically.

## Current state

- Interactive chat loop in the terminal
- Maintains conversation history within a session
- Default model: `nemotron-3-super:cloud` (configurable via `OllamaConfig` in `main.cpp`)

## Requirements

| Tool | Notes |
|------|-------|
| [Ollama](https://ollama.com) | Must be running (`ollama serve`) |
| CMake >= 3.16 | `brew install cmake` |
| Xcode Command Line Tools | `xcode-select --install` (provides clang + libcurl) |

No other dependencies — `nlohmann/json` is bundled as a single header in `include/`.

## Setup

**1. Install and start Ollama**
```bash
brew install ollama
ollama serve          # keep this running in a separate terminal
```

**2. Pull a model**

For a free local model:
```bash
ollama pull nemotron-3-nano
```

**3. Build**
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

## Run

```bash
# Default model (nemotron-3-super:cloud):
./build/yt-summarizer

# Override model by editing OllamaConfig in src/main.cpp
```

Type `exit` or press `Ctrl+D` to quit.
