#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define TAGORE_TYPE_LOADED_IMAGE (tagore_loaded_image_get_type())

G_DECLARE_FINAL_TYPE(TagoreLoadedImage, tagore_loaded_image, TAGORE, LOADED_IMAGE, GObject)

/**
 * tagore_loaded_image_get_file:
 * @self: a #TagoreLoadedImage.
 *
 * Returns the file this image was loaded from.
 *
 * Returns: (transfer none): the source file.
 */
GFile *tagore_loaded_image_get_file(TagoreLoadedImage *self);

/**
 * tagore_loaded_image_get_texture:
 * @self: a #TagoreLoadedImage.
 *
 * Returns the decoded image texture.
 *
 * Returns: (transfer none): the decoded texture.
 */
GdkTexture *tagore_loaded_image_get_texture(TagoreLoadedImage *self);

/**
 * tagore_image_loader_file_is_supported:
 * @file: file to check.
 *
 * Checks whether the file has a supported Milestone 2 image extension.
 */
gboolean tagore_image_loader_file_is_supported(GFile *file);

/**
 * tagore_image_loader_load_file:
 * @file: file to load.
 * @error: return location for a #GError.
 *
 * Loads PNG, JPEG, or WebP image files into a #GdkTexture-backed image object.
 *
 * Returns: (transfer full): a loaded image, or %NULL on error.
 */
TagoreLoadedImage *tagore_image_loader_load_file(GFile *file, GError **error);

G_END_DECLS
