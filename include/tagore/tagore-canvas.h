#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define TAGORE_TYPE_CANVAS (tagore_canvas_get_type())

G_DECLARE_FINAL_TYPE(TagoreCanvas, tagore_canvas, TAGORE, CANVAS, GtkWidget)

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

G_END_DECLS
