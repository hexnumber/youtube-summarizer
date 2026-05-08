# YouTube Summarizer

A C++ console app that chats with a local/cloud AI model via [Ollama](https://ollama.com). Paste a YouTube URL to automatically download its subtitles, clean the transcript, and receive a summary. Or just chat freely.

## Features

- Paste a YouTube URL → downloads subtitles via `yt-dlp`, cleans the VTT transcript, and sends it to the model for summarization
- Interactive chat loop with persistent conversation history within a session
- Default model: `nemotron-3-super:cloud` (configurable via `OllamaConfig` in `src/main.cpp`)
- Default subtitle language: `de` (configurable via `URLHandler::SUBTITLE_LANG` in `include/urlhandler.h`)

## Requirements

| Tool | Notes |
|------|-------|
| [Ollama](https://ollama.com) | Must be running (`ollama serve`) |
| [yt-dlp](https://github.com/yt-dlp/yt-dlp) | `brew install yt-dlp` |
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

**3. Install yt-dlp**
```bash
brew install yt-dlp
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

```
=== YouTube Summarizer - Chat ===
Paste a YouTube URL to summarize, or chat directly.
Type 'exit' to quit

You: https://www.youtube.com/watch?v=dQw4w9WgXcQ
Downloading subtitles (lang: de)...
Cleaning transcript...

Summary: ...

You: What did you think of that?
Assistant: ...
```

Type `exit` or `quit` to quit.
