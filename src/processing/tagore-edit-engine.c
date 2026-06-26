#include "tagore/tagore-edit-engine.h"

struct _TagoreEditEngine
{
  GObject parent_instance;

  GdkTexture *original_texture;
  GdkTexture *preview_texture;
  TagoreEditState state;
};

G_DEFINE_TYPE(TagoreEditEngine, tagore_edit_engine, G_TYPE_OBJECT)

enum
{
  PREVIEW_CHANGED,
  N_SIGNALS,
};

static guint signals[N_SIGNALS];

static void tagore_edit_engine_render_preview(TagoreEditEngine *self);

static void
tagore_edit_engine_dispose(GObject *object)
{
  TagoreEditEngine *self = TAGORE_EDIT_ENGINE(object);

  g_clear_object(&self->original_texture);
  g_clear_object(&self->preview_texture);

  G_OBJECT_CLASS(tagore_edit_engine_parent_class)->dispose(object);
}

static void
tagore_edit_engine_class_init(TagoreEditEngineClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->dispose = tagore_edit_engine_dispose;

  /**
   * TagoreEditEngine::preview-changed:
   * @self: a #TagoreEditEngine.
   *
   * Emitted after the rendered preview texture changes.
   */
  signals[PREVIEW_CHANGED] = g_signal_new(
    "preview-changed",
    G_TYPE_FROM_CLASS(klass),
    G_SIGNAL_RUN_LAST,
    0,
    NULL,
    NULL,
    NULL,
    G_TYPE_NONE,
    0);
}

static void
tagore_edit_engine_init(TagoreEditEngine *self)
{
  tagore_edit_state_init(&self->state);
}

static gboolean
tagore_edit_engine_has_original(TagoreEditEngine *self)
{
  return self->original_texture != NULL &&
         gdk_texture_get_width(self->original_texture) > 0 &&
         gdk_texture_get_height(self->original_texture) > 0;
}

static void
tagore_edit_engine_get_preview_size(const TagoreEditEngine *self, int *out_width, int *out_height)
{
  const int source_width = gdk_texture_get_width(self->original_texture);
  const int source_height = gdk_texture_get_height(self->original_texture);

  switch (self->state.rotation) {
  case TAGORE_ROTATION_90:
  case TAGORE_ROTATION_270:
    *out_width = source_height;
    *out_height = source_width;
    break;
  case TAGORE_ROTATION_0:
  case TAGORE_ROTATION_180:
    *out_width = source_width;
    *out_height = source_height;
    break;
  default:
    g_assert_not_reached();
  }
}

static void
tagore_edit_engine_map_preview_to_source(const TagoreEditEngine *self,
                                         int preview_x,
                                         int preview_y,
                                         int preview_width,
                                         int preview_height,
                                         int *source_x,
                                         int *source_y)
{
  const int source_width = gdk_texture_get_width(self->original_texture);
  const int source_height = gdk_texture_get_height(self->original_texture);
  int unflipped_x = self->state.flip_horizontal ? preview_width - 1 - preview_x : preview_x;
  int unflipped_y = self->state.flip_vertical ? preview_height - 1 - preview_y : preview_y;

  switch (self->state.rotation) {
  case TAGORE_ROTATION_0:
    *source_x = unflipped_x;
    *source_y = unflipped_y;
    break;
  case TAGORE_ROTATION_90:
    *source_x = unflipped_y;
    *source_y = source_height - 1 - unflipped_x;
    break;
  case TAGORE_ROTATION_180:
    *source_x = source_width - 1 - unflipped_x;
    *source_y = source_height - 1 - unflipped_y;
    break;
  case TAGORE_ROTATION_270:
    *source_x = source_width - 1 - unflipped_y;
    *source_y = unflipped_x;
    break;
  default:
    g_assert_not_reached();
  }
}

static GdkTexture *
tagore_edit_engine_render_transform_preview(TagoreEditEngine *self)
{
  const int source_width = gdk_texture_get_width(self->original_texture);
  const gsize source_stride = (gsize)source_width * 4;
  g_autoptr(GdkTextureDownloader) downloader = NULL;
  g_autoptr(GBytes) source_bytes = NULL;
  g_autoptr(GBytes) preview_bytes = NULL;
  guchar *preview_data = NULL;
  const guchar *source_data = NULL;
  gsize downloaded_stride = 0;
  int preview_width = 0;
  int preview_height = 0;
  gsize preview_stride = 0;

  tagore_edit_engine_get_preview_size(self, &preview_width, &preview_height);
  preview_stride = (gsize)preview_width * 4;

  downloader = gdk_texture_downloader_new(self->original_texture);
  gdk_texture_downloader_set_format(downloader, GDK_MEMORY_R8G8B8A8);
  source_bytes = gdk_texture_downloader_download_bytes(downloader, &downloaded_stride);
  source_data = g_bytes_get_data(source_bytes, NULL);

  if (source_data == NULL || downloaded_stride < source_stride)
    return g_object_ref(self->original_texture);

  preview_data = g_malloc(preview_stride * (gsize)preview_height);

  for (int y = 0; y < preview_height; y++) {
    for (int x = 0; x < preview_width; x++) {
      int source_x = 0;
      int source_y = 0;
      guchar *destination_pixel = preview_data + ((gsize)y * preview_stride) + ((gsize)x * 4);
      const guchar *source_pixel = NULL;

      tagore_edit_engine_map_preview_to_source(
        self,
        x,
        y,
        preview_width,
        preview_height,
        &source_x,
        &source_y);

      source_pixel = source_data + ((gsize)source_y * downloaded_stride) + ((gsize)source_x * 4);
      destination_pixel[0] = source_pixel[0];
      destination_pixel[1] = source_pixel[1];
      destination_pixel[2] = source_pixel[2];
      destination_pixel[3] = source_pixel[3];
    }
  }

  preview_bytes = g_bytes_new_take(preview_data, preview_stride * (gsize)preview_height);
  preview_data = NULL;

  return gdk_memory_texture_new(
    preview_width,
    preview_height,
    GDK_MEMORY_R8G8B8A8,
    preview_bytes,
    preview_stride);
}

static void
tagore_edit_engine_render_preview(TagoreEditEngine *self)
{
  g_autoptr(GdkTexture) preview = NULL;

  if (!tagore_edit_engine_has_original(self)) {
    g_clear_object(&self->preview_texture);
    g_signal_emit(self, signals[PREVIEW_CHANGED], 0);
    return;
  }

  if (tagore_edit_state_is_identity(&self->state))
    preview = g_object_ref(self->original_texture);
  else
    preview = tagore_edit_engine_render_transform_preview(self);

  if (g_set_object(&self->preview_texture, preview))
    g_signal_emit(self, signals[PREVIEW_CHANGED], 0);
}

TagoreEditEngine *
tagore_edit_engine_new(void)
{
  return g_object_new(TAGORE_TYPE_EDIT_ENGINE, NULL);
}

void
tagore_edit_engine_set_original_texture(TagoreEditEngine *self, GdkTexture *texture)
{
  g_return_if_fail(TAGORE_IS_EDIT_ENGINE(self));
  g_return_if_fail(texture == NULL || GDK_IS_TEXTURE(texture));

  g_set_object(&self->original_texture, texture);
  tagore_edit_state_init(&self->state);
  tagore_edit_engine_render_preview(self);
}

GdkTexture *
tagore_edit_engine_get_preview_texture(TagoreEditEngine *self)
{
  g_return_val_if_fail(TAGORE_IS_EDIT_ENGINE(self), NULL);

  return self->preview_texture;
}

const TagoreEditState *
tagore_edit_engine_get_state(TagoreEditEngine *self)
{
  g_return_val_if_fail(TAGORE_IS_EDIT_ENGINE(self), NULL);

  return &self->state;
}

void
tagore_edit_engine_rotate_left(TagoreEditEngine *self)
{
  g_return_if_fail(TAGORE_IS_EDIT_ENGINE(self));

  tagore_edit_state_rotate_left(&self->state);
  tagore_edit_engine_render_preview(self);
}

void
tagore_edit_engine_rotate_right(TagoreEditEngine *self)
{
  g_return_if_fail(TAGORE_IS_EDIT_ENGINE(self));

  tagore_edit_state_rotate_right(&self->state);
  tagore_edit_engine_render_preview(self);
}

void
tagore_edit_engine_flip_horizontal(TagoreEditEngine *self)
{
  g_return_if_fail(TAGORE_IS_EDIT_ENGINE(self));

  tagore_edit_state_flip_horizontal(&self->state);
  tagore_edit_engine_render_preview(self);
}

void
tagore_edit_engine_flip_vertical(TagoreEditEngine *self)
{
  g_return_if_fail(TAGORE_IS_EDIT_ENGINE(self));

  tagore_edit_state_flip_vertical(&self->state);
  tagore_edit_engine_render_preview(self);
}
