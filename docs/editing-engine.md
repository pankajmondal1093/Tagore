# Editing Engine Design

The editing engine is the shared foundation for Tagore's non-destructive image
editing tools. It owns image-editing state and preview generation while GTK UI
code only dispatches high-level editing actions.

## Why This Design

Tagore needs future support for undo/redo, layers, filters, plugins, AI tools,
RAW processing, and alternate processing backends. Those features become fragile
if each tool mutates pixels directly or hides logic inside widgets.

The engine therefore keeps the original image immutable and renders previews
from the original plus an explicit edit state. Save and export will later consume
the same state to write modified pixels to disk.

## Modules

### Edit State

`TagoreEditState` is a plain data structure containing non-destructive
parameters:

- rotation
- horizontal and vertical flips
- crop rectangle
- resize dimensions and interpolation
- brightness, contrast, saturation, exposure, and gamma

The state module provides small mutation helpers for operations that are already
implemented. Future tools should add state helpers before adding UI controls.

### Edit Engine

`TagoreEditEngine` is a GObject service that owns:

- the immutable original texture
- the current edit state
- the rendered preview texture
- the `preview-changed` signal

The first renderer uses GTK's texture download/upload APIs and a known
`R8G8B8A8` format. This is intentionally a backend seam: libvips, OpenCV, GEGL,
GPU render nodes, or tile-based processing can replace the renderer without
changing window actions or canvas code.

## Public APIs

- `tagore_edit_engine_set_original_texture()`
- `tagore_edit_engine_get_preview_texture()`
- `tagore_edit_engine_get_state()`
- `tagore_edit_engine_rotate_left()`
- `tagore_edit_engine_rotate_right()`
- `tagore_edit_engine_flip_horizontal()`
- `tagore_edit_engine_flip_vertical()`

## Current Feature Scope

This milestone implements non-destructive rotate and flip. Crop, resize, and
adjustments are represented in the state structure but are not partially wired
into the UI yet.

## Future Extensibility

- History can store previous `TagoreEditState` snapshots.
- Plugins can contribute operations that update state or render preview stages.
- Batch processing can reuse the same state without GTK.
- Large images can render preview tiles instead of whole-image buffers.
- Save/export can render from original plus state using a high-quality backend.
