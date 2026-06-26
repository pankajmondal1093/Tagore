# Navigation Design

Phase 1 navigation is implemented as viewer state, not image-editing state. Zoom,
fit, actual-size, pan, and fullscreen change how the image is viewed; they do
not modify the loaded image and therefore do not belong in the destructive or
non-destructive processing pipeline.

## Why This Design

The canvas owns rendering and view navigation because it already knows widget
dimensions, texture dimensions, pointer gestures, and redraw timing. The window
owns actions, buttons, shortcuts, and fullscreen because those are application
shell concerns.

This keeps the UI from duplicating rendering math and keeps future processing
modules independent from GTK widgets.

## Responsibilities

### Canvas

- Track zoom mode: fit, actual size, or custom.
- Compute effective zoom from widget and image size.
- Render the texture at the correct scale and pan offset.
- Handle mouse-wheel zoom.
- Handle space-drag and middle-button drag panning.
- Emit `zoom-changed` for UI status updates.

### Window

- Expose GNOME-style header bar controls.
- Bind keyboard shortcuts to window actions.
- Toggle fullscreen.
- Display status information such as the current zoom percentage.

## Public APIs

Navigation is exposed through intent-level APIs:

- `tagore_canvas_zoom_in()`
- `tagore_canvas_zoom_out()`
- `tagore_canvas_fit_to_window()`
- `tagore_canvas_actual_size()`
- `tagore_canvas_get_zoom()`

## Future Extensibility

The same canvas boundary can later support tiled rendering, GPU render nodes,
color-managed output, minimaps, crop overlays, selection handles, and viewport
synchronization without changing the file loader or image-processing modules.

When non-destructive edit operations are added, they should live in image state
and history modules. The canvas should consume a rendered preview of that state
instead of owning processing logic.
