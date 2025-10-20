# subhanMovie

A lightweight C++ wrapper around `aria2c` designed to simplify magnet‑link and torrent downloading for movie-related workflows.

## 🧾 Table of Contents
- [What the Project Does](#what-the-project-does)
- [Key Features](#key-features)
- [Requirements](#requirements)
- [Installation & Build](#installation--build)
- [Usage](#usage)
- [Configuration Options](#configuration-options)
- [How it Works (Internals)](#how-it-works-internals)
- [Contributing](#contributing)
- [License](#license)

## What the Project Does
`subhanMovie_aria2cWrapper` provides a clean C++ interface for launching and managing downloads using the `aria2c` command-line tool. It focuses on handling magnet links or torrent files with minimal setup, making it ideal for movie/media automation or integration into larger applications.

Instead of manually writing system commands to interact with `aria2c`, this wrapper makes it easy to trigger downloads directly from C++ code.

## Key Features
- Supports magnet:// links and torrent downloads.
- Simple wrapper for executing `aria2c` commands.
- Can be extended for download progress monitoring.
- Ideal for automation or integration with media tools.
- Cross-platform capability (Linux, Windows, macOS with `aria2c` installed).

## Requirements
- C++11 or later
- `aria2c` installed and accessible in system PATH
- Compiler such as GCC, Clang, or MSVC
- Basic understanding of C++ build tools

## Installation & Build

```bash
git clone https://github.com/0xSubhan/subhanMovie_aria2cWrapper.git
cd subhanMovie_aria2cWrapper
./subhanMovie
```

Ensure `aria2c` is installed and available:
```bash
aria2c --version
```

## Usage

Run the wrapper with a magnet link and target directory:

```bash
./aria2wrapper "magnet:?xt=urn:btih:YOUR_HASH_HERE" "/downloads/movies"
```

### Example (in code)
```cpp
int result = runDownload("magnet:?xt=urn:btih:...", "/downloads");
if (result == 0)
    std::cout << "Download finished successfully!";
else
    std::cerr << "Download failed: " << result;
```

## Configuration Options
You can enhance the wrapper to support additional `aria2c` flags such as:

- `--dir=<DIR>` — Download directory
- `--split=<N>` — Connections per download
- `--continue=true` — Resume support
- `--seed-time=<MIN>` — Torrent seeding time

Modify the C++ source to pass these flags programmatically.

## How it Works (Internals)
- The wrapper builds a command string for `aria2c`.
- Executes via `system()` or `std::process` equivalents.
- Waits for completion and returns success or failure codes.
- Simple, modular, and ready to integrate into larger tools.

## Contributing

1. Fork the repository
2. Create a feature branch
3. Commit and push changes
4. Open a Pull Request

Issues and feature suggestions are welcome!

## License

Licensed under the **MIT License**. See `LICENSE` file for details.


_No license provided yet — you may add MIT, Apache-2.0, or any preferred license._

---

**🚀 Happy downloading with aria2c + C++!**

---

👤 **Author:** [0xSubhan](https://github.com/0xSubhan)
