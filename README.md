# Tagore

Tagore is a professional Linux-native image viewer and editor built with C17,
GTK4, and libadwaita. The project targets GNOME first while keeping the
architecture modular enough to support other Linux desktop environments later.

## Vision

The long-term goal is a family of native creative applications:

- Tagore: image viewer and photo editor
- Tagore Vector
- Tagore Paint
- Tagore PDF

Tagore is intentionally not an Electron application. Startup performance, memory
discipline, desktop integration, and clean Linux conventions are core product
requirements.

## Current Status

This repository contains the initial architecture, folder structure, Meson build
configuration, GTK/libadwaita application shell, native image-viewer canvas,
GdkTexture-backed PNG/JPEG/WebP loading, drag-and-drop support, and recent-file
persistence.

## Build

Install the development dependencies for your distribution, then run:

```sh
meson setup build
meson compile -C build
meson test -C build
```

Required core dependencies:

- Meson
- Ninja
- C compiler with C17 support
- GTK4 development files
- libadwaita development files
- GLib/GIO/GObject development files

On Ubuntu, install the initial build dependencies with:

```sh
sudo apt-get install build-essential meson ninja-build pkg-config gettext libgtk-4-dev libadwaita-1-dev
```

## Architecture

Start with [docs/architecture.md](docs/architecture.md). Module documentation
lives under [docs/modules.md](docs/modules.md).
