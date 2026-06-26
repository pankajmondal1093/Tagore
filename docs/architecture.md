# Tagore Architecture

Tagore is organized as a native Linux desktop application with a layered,
module-oriented architecture. The codebase must remain friendly to future
features such as plugins, GPU acceleration, AI-assisted editing, and additional
applications in the Tagore family.

## Architectural Principles

- Keep UI, image state, file operations, and processing engines separate.
- Prefer explicit dependencies over global state.
- Expose small public headers for each module.
- Keep modules independently testable wherever practical.
- Use GLib/GObject conventions where they improve lifetime management,
  signal-based integration, and GTK interoperability.
- Keep computational image operations out of GTK widgets.
- Preserve a path toward async loading, tiled rendering, and large-image
  workflows.

## Initial Layers

### Application Shell

Owns process startup, command-line activation, high-level actions, application
metadata, and top-level window creation. It may coordinate modules but must not
perform image processing itself.

### UI

Contains GTK/libadwaita widgets, windows, dialogs, menus, shortcuts, and visual
state. UI classes should communicate with core services through explicit APIs.

### Core Services

Owns application-level services and domain abstractions that are not tied to
specific widgets. Future services include document management, command routing,
history coordination, and plugin discovery.

### Image Pipeline

Loading, decoding, processing, transforms, adjustments, metadata extraction,
canvas rendering, and exporting are separated so that each concern can evolve
independently.

### Editing Pipeline

Editing is non-destructive. The original image remains immutable while
`TagoreEditState` stores operations such as rotation, flip, crop, resize, and
adjustments. `TagoreEditEngine` renders a preview texture from the original plus
that state and notifies the UI through signals.

GTK widgets dispatch actions to the engine; they do not modify pixels directly.
Save and export will later consume the same edit state to write modified pixels
to disk.

### Platform Integration

Configuration, cache, user data, logging, recent files, native file choosers,
and desktop resources follow Linux and GNOME conventions.

## Dependency Direction

UI depends on core interfaces. Core may depend on GLib/GIO but should not depend
on concrete widgets. Image-processing modules must not depend on UI. Export and
file modules may depend on image abstractions, but not on window classes.

The intended direction is:

```text
main -> core application -> UI
                      \-> services
services -> image/processing/metadata/history/export/config/utils
```

## Future Extension Points

- Plugin API loaded through explicit manifests and stable C interfaces.
- Processing backends that can swap CPU, GEGL, OpenCV, libvips, or GPU paths.
- Document model that can support raster, vector, paint, and PDF applications.
- Async image loading and tiled rendering for very large images.
- Optional AI features isolated behind service interfaces.
