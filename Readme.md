# filesorter

A lightweight Linux daemon that automatically sorts files inside a watched directory based on configurable rules.
The watcher detects newly created or moved files (using **inotify**) and relocates them according to `rules.json`.

---

## 📦 Prerequisites

Before installing, ensure you have:

* **Linux system** with systemd (user services supported)
* **CMake ≥ 3.16**
* **g++ / clang** with **C++17** support
* **pthread** (usually included in glibc)
* **git** (for submodules)
* Permissions to write to:

  * `/usr/local/bin`
  * `~/.config/`

Clone with submodules:

```bash
git clone --recursive <repo-url>
```

If you already cloned without submodules:

```bash
git submodule update --init --recursive
```

---

## 🔧 Installation

Run the provided setup script:

```bash
cd config
./setup.sh
```

The script performs:

1. **Build the project** using CMake (`build/` directory)
2. **Install binary** into `/usr/local/bin/filesorter`
3. **Create user config** in `~/.config/filesorter/rules.json`
4. **Generate systemd user service** at `~/.config/systemd/user/filesorter.service`
5. **Enable and start** the user service automatically

---

## ▶️ Usage

### Start, stop, inspect service

```bash
systemctl --user start filesorter.service
systemctl --user stop filesorter.service
systemctl --user restart filesorter.service
```

### Follow log output

```bash
journalctl --user -u filesorter.service -f
```

---

## 📝 Configuration

Edit your config at:

```
~/.config/filesorter/rules.json
```

The file defines:

* The **watch_folder**
* An array of **rules**, each mapping a file extension to a destination folder

Example snippet:

```json
{
  "watch_folder": "$HOME/Downloads",
  "rules": [
    {
      "extension": ".pdf",
      "destination": "$HOME/Downloads/PDFs"
    }
  ]
}
```

After modifying the config:

* Save the file
* The service reloads automatically via inotify
  *(No need to restart the daemon)*

---

## 🧩 How It Works

* `FileWatcher` observes:

  * the watched folder
  * the configuration file (auto-reload)
* `FileSorter` moves files based on rules and resolves name conflicts safely
* `ConfigLoader` parses and validates the JSON rules
* `Utils` expands `$HOME` variables and resolves the user’s home directory

---

## 🗑️ Uninstall

```bash
systemctl --user stop filesorter.service
systemctl --user disable filesorter.service
rm ~/.config/systemd/user/filesorter.service
rm ~/.config/filesorter/rules.json
sudo rm /usr/local/bin/filesorter
systemctl --user daemon-reload
```

---

## 📄 License

MIT
