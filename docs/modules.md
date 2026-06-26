# Module Documentation

Every module begins with purpose, responsibilities, public API shape,
dependencies, and extensibility notes. The first implementation slice includes
only application shell, UI window, and configuration paths.

## Core

### Why It Exists

Core owns application-level behavior that should not live inside widgets.

### Responsibilities

- Application lifecycle.
- Action registration.
- Service wiring.
- Future document/session coordination.

### Public APIs

- `TagoreApplication`: a libadwaita application subclass.

### Dependencies

Core may depend on GLib, GIO, GObject, GTK, and libadwaita for application
startup. Deeper services should avoid widget dependencies.

### Future Extensibility

Core will become the composition root for document models, history, plugins,
configuration, and service registries.

## UI

### Why It Exists

UI contains all GTK/libadwaita presentation code.

### Responsibilities

- Main window.
- Menus, actions, shortcuts, dialogs, and responsive layout.
- Future toolbars, sidebars, inspector panels, and canvas containers.

### Public APIs

- `TagoreWindow`: the main application window.

### Dependencies

GTK4 and libadwaita. UI may call core services through public interfaces.

### Future Extensibility

Window construction should remain modular so the canvas, metadata panel,
adjustment controls, and export flows can be added without turning the main
window into a large controller.

## Configuration

### Why It Exists

Configuration centralizes Linux-standard paths for settings, cache, and user
data.

### Responsibilities

- Resolve `~/.config/tagore`.
- Resolve `~/.cache/tagore`.
- Resolve `~/.local/share/tagore`.
- Keep path construction testable and independent of UI.

### Public APIs

- `tagore_paths_get_config_dir()`
- `tagore_paths_get_cache_dir()`
- `tagore_paths_get_data_dir()`

Each function returns a newly allocated path owned by the caller.

### Dependencies

GLib only.

### Future Extensibility

This module can later create directories, manage schema locations, and expose
profile-aware paths for plugin or workspace state.

## Canvas

### Why It Exists

The canvas renders the currently loaded image independently from file dialogs,
loaders, and future editing tools.

### Responsibilities

- Display a decoded image texture.
- Scale the image to fit the available viewport while preserving aspect ratio.
- Keep rendering isolated from loading and persistence code.
- Provide a stable place for future zoom, pan, crop overlays, selections, and
  tiled/GPU rendering.

### Public APIs

- `TagoreCanvas`: GTK widget responsible for image presentation.
- `tagore_canvas_set_texture()`: sets or clears the displayed texture.
- `tagore_canvas_clear()`: clears the current image.

### Dependencies

GTK4/GDK only. The canvas must not depend on file chooser code, recent-file
storage, or image-loader backends.

### Future Extensibility

The current implementation draws a `GdkTexture`. Later it can switch to tiled
render nodes, color-managed rendering, GPU-backed buffers, or overlay layers
without changing the window's file-open flow.

## Image Loader

### Why It Exists

The image loader hides the decoding backend from the UI. Milestone 2 uses
`GdkTexture` so the application can display PNG, JPEG, and WebP images with
GTK's native stack.

### Responsibilities

- Validate supported file types.
- Decode image files into a lightweight image object.
- Return structured errors.
- Keep backend-specific APIs away from the UI layer.

### Public APIs

- `TagoreLoadedImage`: immutable loaded-image object for the viewer layer.
- `tagore_image_loader_load_file()`: loads a `GFile`.
- `tagore_image_loader_file_is_supported()`: checks common raster types.

### Dependencies

GTK4/GDK and GIO for the initial backend. Future backends may use libvips,
OpenCV, or GEGL behind the same public loader interface.

### Future Extensibility

The UI should continue consuming a loader result rather than a concrete backend.
This allows async decoding, color profiles, thumbnails, huge-image tiling, and
processing-specific image models to be added later.

## Image Processing

Processing coordinates backend-neutral pixel operations. GEGL, OpenCV, and
libvips adapters should remain replaceable.

## Editing Engine

### Why It Exists

The editing engine provides the non-destructive foundation for every image
modification feature.

### Responsibilities

- Own the immutable original image for the current editing session.
- Own non-destructive edit state.
- Render live preview textures from original image plus state.
- Notify the UI when the preview changes.
- Keep editing logic out of GTK widgets.

### Public APIs

- `TagoreEditState`: plain state for rotation, flip, crop, resize, and
  adjustments.
- `TagoreEditEngine`: GObject service for preview rendering.
- Transform methods for rotate left/right and flip horizontal/vertical.

### Dependencies

The initial preview renderer depends on GTK/GDK texture APIs. The state module
depends only on GLib/GObject types.

### Future Extensibility

The renderer can later move to libvips, OpenCV, GEGL, GPU render nodes, or a
tiled processing graph without changing UI actions or canvas rendering.

## Transform Engine

Transforms cover rotate, flip, crop, and resize. They should be represented as
commands so history, preview, and non-destructive editing can share behavior.

## Adjustment Engine

Adjustments cover brightness, contrast, saturation, exposure, and gamma. The
engine should support previews, batching, and future GPU acceleration.

## Metadata

Metadata owns EXIF, resolution, dimensions, and future sidecar data extraction.
It must avoid UI dependencies.

## File Manager

### Why It Exists

File Manager owns file-oriented application state that should not live in the
main window.

### Responsibilities

- Maintain Tagore's recent image list.
- Persist recent files under the user's XDG data directory.
- Deduplicate and cap recent entries.
- Leave decoding to Image Loader and dialogs to UI.

### Public APIs

- `TagoreRecentFiles`: recent-file service.
- `tagore_recent_files_add_file()`
- `tagore_recent_files_get_files()`

### Dependencies

GLib/GIO plus Tagore configuration paths.

### Future Extensibility

This module can later coordinate open/save/save-as workflows, file monitoring,
and desktop recent-file integration while preserving the loader/UI boundary.

## History

History owns undo/redo stacks and command grouping. It should not know about
GTK widgets.

## Export

Export owns format-specific output options, validation, and encoding.

## Utilities

Utilities contain small, dependency-light helpers shared across modules.

## Logging

Logging will centralize structured diagnostics and future debug tracing.

## Plugin API

The future plugin API must be versioned, explicit, and isolated from private
application internals.
