#pragma once

#include "tagore/tagore-edit-state.h"

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define TAGORE_TYPE_EDIT_ENGINE (tagore_edit_engine_get_type())

G_DECLARE_FINAL_TYPE(TagoreEditEngine, tagore_edit_engine, TAGORE, EDIT_ENGINE, GObject)

/**
 * tagore_edit_engine_new:
 *
 * Creates a non-destructive edit engine.
 *
 * Returns: (transfer full): a new #TagoreEditEngine.
 */
TagoreEditEngine *tagore_edit_engine_new(void);

/**
 * tagore_edit_engine_set_original_texture:
 * @self: a #TagoreEditEngine.
 * @texture: (nullable): original texture.
 *
 * Sets the immutable source image for the current edit session and resets the
 * edit state.
 */
void tagore_edit_engine_set_original_texture(TagoreEditEngine *self, GdkTexture *texture);

/**
 * tagore_edit_engine_get_preview_texture:
 * @self: a #TagoreEditEngine.
 *
 * Returns the current rendered preview texture.
 *
 * Returns: (transfer none) (nullable): current preview texture.
 */
GdkTexture *tagore_edit_engine_get_preview_texture(TagoreEditEngine *self);

/**
 * tagore_edit_engine_get_state:
 * @self: a #TagoreEditEngine.
 *
 * Returns the current non-destructive edit state.
 *
 * Returns: (transfer none): current edit state.
 */
const TagoreEditState *tagore_edit_engine_get_state(TagoreEditEngine *self);

void tagore_edit_engine_rotate_left(TagoreEditEngine *self);
void tagore_edit_engine_rotate_right(TagoreEditEngine *self);
void tagore_edit_engine_flip_horizontal(TagoreEditEngine *self);
void tagore_edit_engine_flip_vertical(TagoreEditEngine *self);

G_END_DECLS
