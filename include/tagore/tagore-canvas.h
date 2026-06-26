#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define TAGORE_TYPE_CANVAS (tagore_canvas_get_type())

G_DECLARE_FINAL_TYPE(TagoreCanvas, tagore_canvas, TAGORE, CANVAS, GtkWidget)

typedef enum
{
  TAGORE_CANVAS_ZOOM_MODE_FIT,
  TAGORE_CANVAS_ZOOM_MODE_ACTUAL_SIZE,
  TAGORE_CANVAS_ZOOM_MODE_CUSTOM,
} TagoreCanvasZoomMode;

/**
 * tagore_canvas_new:
 *
 * Creates an image canvas widget.
 *
 * Returns: (transfer full): a new #TagoreCanvas.
 */
GtkWidget *tagore_canvas_new(void);

/**
 * tagore_canvas_set_texture:
 * @self: a #TagoreCanvas.
 * @texture: (nullable): texture to display.
 *
 * Sets the image texture displayed by the canvas.
 */
void tagore_canvas_set_texture(TagoreCanvas *self, GdkTexture *texture);

/**
 * tagore_canvas_clear:
 * @self: a #TagoreCanvas.
 *
 * Clears the displayed image.
 */
void tagore_canvas_clear(TagoreCanvas *self);

/**
 * tagore_canvas_zoom_in:
 * @self: a #TagoreCanvas.
 *
 * Increases the viewer zoom level.
 */
void tagore_canvas_zoom_in(TagoreCanvas *self);

/**
 * tagore_canvas_zoom_out:
 * @self: a #TagoreCanvas.
 *
 * Decreases the viewer zoom level.
 */
void tagore_canvas_zoom_out(TagoreCanvas *self);

/**
 * tagore_canvas_fit_to_window:
 * @self: a #TagoreCanvas.
 *
 * Scales the image to fit the visible canvas area.
 */
void tagore_canvas_fit_to_window(TagoreCanvas *self);

/**
 * tagore_canvas_actual_size:
 * @self: a #TagoreCanvas.
 *
 * Displays the image at 100% scale.
 */
void tagore_canvas_actual_size(TagoreCanvas *self);

/**
 * tagore_canvas_get_zoom:
 * @self: a #TagoreCanvas.
 *
 * Returns the current effective zoom factor.
 */
double tagore_canvas_get_zoom(TagoreCanvas *self);

G_END_DECLS
