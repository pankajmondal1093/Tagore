#include "tagore/tagore-image-loader.h"

#include <string.h>

struct _TagoreLoadedImage
{
  GObject parent_instance;

  GFile *file;
  GdkTexture *texture;
};

G_DEFINE_TYPE(TagoreLoadedImage, tagore_loaded_image, G_TYPE_OBJECT)

typedef enum
{
  TAGORE_IMAGE_LOADER_ERROR_UNSUPPORTED_FORMAT,
} TagoreImageLoaderError;

static GQuark
tagore_image_loader_error_quark(void)
{
  return g_quark_from_static_string("tagore-image-loader-error");
}

static void
tagore_loaded_image_dispose(GObject *object)
{
  TagoreLoadedImage *self = TAGORE_LOADED_IMAGE(object);

  g_clear_object(&self->file);
  g_clear_object(&self->texture);

  G_OBJECT_CLASS(tagore_loaded_image_parent_class)->dispose(object);
}

static void
tagore_loaded_image_class_init(TagoreLoadedImageClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->dispose = tagore_loaded_image_dispose;
}

static void
tagore_loaded_image_init(TagoreLoadedImage *self)
{
  (void)self;
}

static TagoreLoadedImage *
tagore_loaded_image_new(GFile *file, GdkTexture *texture)
{
  TagoreLoadedImage *image = g_object_new(TAGORE_TYPE_LOADED_IMAGE, NULL);

  image->file = g_object_ref(file);
  image->texture = g_object_ref(texture);

  return image;
}

GFile *
tagore_loaded_image_get_file(TagoreLoadedImage *self)
{
  g_return_val_if_fail(TAGORE_IS_LOADED_IMAGE(self), NULL);

  return self->file;
}

GdkTexture *
tagore_loaded_image_get_texture(TagoreLoadedImage *self)
{
  g_return_val_if_fail(TAGORE_IS_LOADED_IMAGE(self), NULL);

  return self->texture;
}

static gboolean
tagore_filename_has_supported_extension(const char *basename)
{
  static const char *const supported_extensions[] = {
    ".jpeg",
    ".jpg",
    ".png",
    ".webp",
    NULL,
  };

  if (basename == NULL)
    return FALSE;

  for (size_t i = 0; supported_extensions[i] != NULL; i++) {
    if (g_str_has_suffix(basename, supported_extensions[i]))
      return TRUE;
  }

  return FALSE;
}

gboolean
tagore_image_loader_file_is_supported(GFile *file)
{
  g_autofree char *basename = NULL;
  g_autofree char *lowercase = NULL;

  g_return_val_if_fail(G_IS_FILE(file), FALSE);

  basename = g_file_get_basename(file);
  lowercase = g_utf8_strdown(basename, -1);

  return tagore_filename_has_supported_extension(lowercase);
}

TagoreLoadedImage *
tagore_image_loader_load_file(GFile *file, GError **error)
{
  g_autoptr(GdkTexture) texture = NULL;

  g_return_val_if_fail(G_IS_FILE(file), NULL);
  g_return_val_if_fail(error == NULL || *error == NULL, NULL);

  if (!tagore_image_loader_file_is_supported(file)) {
    g_set_error(
      error,
      tagore_image_loader_error_quark(),
      TAGORE_IMAGE_LOADER_ERROR_UNSUPPORTED_FORMAT,
      "Tagore supports PNG, JPEG, and WebP files in this milestone.");
    return NULL;
  }

  texture = gdk_texture_new_from_file(file, error);
  if (texture == NULL)
    return NULL;

  return tagore_loaded_image_new(file, texture);
}
