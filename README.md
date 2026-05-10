# YouTube Summarizer

A C++ GUI app that summarizes YouTube videos and generates Anki flashcards using a local/cloud AI model via [Ollama](https://ollama.com). Paste a YouTube URL to automatically download subtitles, clean the transcript, and get a summary or Anki-ready flashcards — or just chat with the model directly.

## Features

- Paste a YouTube URL → downloads subtitles via `yt-dlp`, cleans the VTT transcript, and sends it to the model
- Three actions to choose from: **Summarize**, **Anki Cards**, or **Both**
- Anki flashcards exported as a `.csv` file importable directly into Anki (tab-separated, `#separator:tab` header included)
- Interactive chat with persistent conversation history within a session
- Dark / light mode toggle
- Non-blocking UI — downloads and LLM calls run in a background thread
- Default model: `nemotron-3-super:cloud` (configurable via `OllamaConfig` in `include/ollama.h`)
- Default subtitle language: `de` (configurable via `URLHandler::SUBTITLE_LANG` in `include/urlhandler.h`)

## Requirements

| Tool | Notes |
|------|-------|
| [Ollama](https://ollama.com) | Must be running (`ollama serve`) |
| [yt-dlp](https://github.com/yt-dlp/yt-dlp) | `brew install yt-dlp` |
| [Qt6](https://www.qt.io) | `brew install qt` |
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
```bash
ollama pull nemotron-3-nano   # free local model
```

**3. Install dependencies**
```bash
brew install yt-dlp qt cmake
xcode-select --install
```

**4. Build**
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

## Run

```bash
./build/yt-summarizer
```

## Usage

**Summarize a video**
1. Paste a YouTube URL into the input field
2. Select an action: *Summarize*, *Anki Cards*, or *Both*
3. Press Enter or click Send

**Chat**

Type any message (not a YouTube URL) and press Enter to chat with the model directly. Conversation history is preserved for the session.

**Importing Anki flashcards**

After generating Anki cards, an `anki_<videoId>.csv` file is created in the working directory. In Anki: **File → Import** → select the file. The `#separator:tab` header is included so Anki detects the format automatically.

## Project structure

```
include/
  ollama.h        — ChatSession, OllamaConfig, sendMessage declaration
  summarize.h     — VTTCleaner (strips timestamps, tags, duplicates from .vtt files)
  urlhandler.h    — URLHandler (YouTube URL parsing, yt-dlp execution, Anki export)
  json.hpp        — bundled nlohmann/json
src/
  main.cpp        — Qt app entry point, curl init/cleanup
  mainwindow.h/cpp — Qt window, dark/light theme, chat output
  worker.h/cpp    — background QThread for yt-dlp, VTT cleaning, LLM calls
  ollama.cpp      — sendMessage implementation (libcurl → Ollama API)
```
