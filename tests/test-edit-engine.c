#include "tagore/tagore-edit-engine.h"

#include <glib.h>

typedef struct
{
  guint count;
} SignalCounter;

static GdkTexture *
create_two_pixel_texture(void)
{
  static const guchar pixels[] = {
    255, 0, 0, 255,
    0, 255, 0, 255,
  };
  g_autoptr(GBytes) bytes = g_bytes_new_static(pixels, sizeof(pixels));

  return gdk_memory_texture_new(2, 1, GDK_MEMORY_R8G8B8A8, bytes, 8);
}

static GBytes *
download_rgba(GdkTexture *texture, gsize *out_stride)
{
  g_autoptr(GdkTextureDownloader) downloader = gdk_texture_downloader_new(texture);

  gdk_texture_downloader_set_format(downloader, GDK_MEMORY_R8G8B8A8);
  return gdk_texture_downloader_download_bytes(downloader, out_stride);
}

static void
preview_changed_cb(TagoreEditEngine *engine, gpointer user_data)
{
  SignalCounter *counter = user_data;

  (void)engine;

  counter->count++;
}

static void
assert_pixel_is(const guchar *data, gsize stride, int x, int y, guchar r, guchar g, guchar b, guchar a)
{
  const guchar *pixel = data + ((gsize)y * stride) + ((gsize)x * 4);

  g_assert_cmpuint(pixel[0], ==, r);
  g_assert_cmpuint(pixel[1], ==, g);
  g_assert_cmpuint(pixel[2], ==, b);
  g_assert_cmpuint(pixel[3], ==, a);
}

static void
test_engine_keeps_original_and_updates_preview(void)
{
  g_autoptr(TagoreEditEngine) engine = tagore_edit_engine_new();
  g_autoptr(GdkTexture) original = create_two_pixel_texture();
  SignalCounter counter = {0};

  g_signal_connect(engine, "preview-changed", G_CALLBACK(preview_changed_cb), &counter);

  tagore_edit_engine_set_original_texture(engine, original);

  g_assert_true(tagore_edit_state_is_identity(tagore_edit_engine_get_state(engine)));
  g_assert_true(tagore_edit_engine_get_preview_texture(engine) == original);
  g_assert_cmpuint(counter.count, ==, 1);

  tagore_edit_engine_rotate_right(engine);

  g_assert_true(tagore_edit_engine_get_preview_texture(engine) != original);
  g_assert_cmpint(gdk_texture_get_width(tagore_edit_engine_get_preview_texture(engine)), ==, 1);
  g_assert_cmpint(gdk_texture_get_height(tagore_edit_engine_get_preview_texture(engine)), ==, 2);
  g_assert_cmpint(gdk_texture_get_width(original), ==, 2);
  g_assert_cmpint(gdk_texture_get_height(original), ==, 1);
  g_assert_cmpuint(counter.count, ==, 2);
}

static void
test_engine_flip_horizontal_preview_pixels(void)
{
  g_autoptr(TagoreEditEngine) engine = tagore_edit_engine_new();
  g_autoptr(GdkTexture) original = create_two_pixel_texture();
  GdkTexture *preview = NULL;
  g_autoptr(GBytes) bytes = NULL;
  gsize stride = 0;
  const guchar *data = NULL;

  tagore_edit_engine_set_original_texture(engine, original);
  tagore_edit_engine_flip_horizontal(engine);

  preview = tagore_edit_engine_get_preview_texture(engine);
  bytes = download_rgba(preview, &stride);
  data = g_bytes_get_data(bytes, NULL);

  g_assert_cmpint(gdk_texture_get_width(preview), ==, 2);
  g_assert_cmpint(gdk_texture_get_height(preview), ==, 1);
  assert_pixel_is(data, stride, 0, 0, 0, 255, 0, 255);
  assert_pixel_is(data, stride, 1, 0, 255, 0, 0, 255);
}

static void
test_engine_rotate_right_preview_pixels(void)
{
  g_autoptr(TagoreEditEngine) engine = tagore_edit_engine_new();
  g_autoptr(GdkTexture) original = create_two_pixel_texture();
  GdkTexture *preview = NULL;
  g_autoptr(GBytes) bytes = NULL;
  gsize stride = 0;
  const guchar *data = NULL;

  tagore_edit_engine_set_original_texture(engine, original);
  tagore_edit_engine_rotate_right(engine);

  preview = tagore_edit_engine_get_preview_texture(engine);
  bytes = download_rgba(preview, &stride);
  data = g_bytes_get_data(bytes, NULL);

  g_assert_cmpint(gdk_texture_get_width(preview), ==, 1);
  g_assert_cmpint(gdk_texture_get_height(preview), ==, 2);
  assert_pixel_is(data, stride, 0, 0, 255, 0, 0, 255);
  assert_pixel_is(data, stride, 0, 1, 0, 255, 0, 255);
}

int
main(int argc, char *argv[])
{
  g_test_init(&argc, &argv, NULL);

  g_test_add_func("/tagore/edit-engine/keeps-original-and-updates-preview", test_engine_keeps_original_and_updates_preview);
  g_test_add_func("/tagore/edit-engine/flip-horizontal-preview-pixels", test_engine_flip_horizontal_preview_pixels);
  g_test_add_func("/tagore/edit-engine/rotate-right-preview-pixels", test_engine_rotate_right_preview_pixels);

  return g_test_run();
}
