#include "tagore/tagore-recent-files.h"

#include "tagore/tagore-paths.h"

#include <glib/gstdio.h>

struct _TagoreRecentFiles
{
  GObject parent_instance;

  GPtrArray *uris;
  char *storage_path;
};

G_DEFINE_TYPE(TagoreRecentFiles, tagore_recent_files, G_TYPE_OBJECT)

static void tagore_recent_files_save(TagoreRecentFiles *self);

static char *
tagore_recent_files_get_storage_path(void)
{
  g_autofree char *data_dir = tagore_paths_get_data_dir();

  return g_build_filename(data_dir, "recent-files.ini", NULL);
}

static void
tagore_recent_files_dispose(GObject *object)
{
  TagoreRecentFiles *self = TAGORE_RECENT_FILES(object);

  g_clear_pointer(&self->uris, g_ptr_array_unref);
  g_clear_pointer(&self->storage_path, g_free);

  G_OBJECT_CLASS(tagore_recent_files_parent_class)->dispose(object);
}

static void
tagore_recent_files_class_init(TagoreRecentFilesClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->dispose = tagore_recent_files_dispose;
}

static void
tagore_recent_files_init(TagoreRecentFiles *self)
{
  self->uris = g_ptr_array_new_with_free_func(g_free);
  self->storage_path = tagore_recent_files_get_storage_path();
}

static gboolean
tagore_recent_files_contains_uri(TagoreRecentFiles *self, const char *uri, guint *position)
{
  for (guint i = 0; i < self->uris->len; i++) {
    const char *existing_uri = g_ptr_array_index(self->uris, i);

    if (g_strcmp0(existing_uri, uri) == 0) {
      if (position != NULL)
        *position = i;
      return TRUE;
    }
  }

  return FALSE;
}

static void
tagore_recent_files_load(TagoreRecentFiles *self)
{
  g_autoptr(GKeyFile) key_file = g_key_file_new();
  g_auto(GStrv) uris = NULL;
  gsize length = 0;

  if (!g_key_file_load_from_file(key_file, self->storage_path, G_KEY_FILE_NONE, NULL))
    return;

  uris = g_key_file_get_string_list(key_file, "Recent Files", "uris", &length, NULL);

  for (gsize i = 0; uris != NULL && i < length && self->uris->len < TAGORE_RECENT_FILES_MAX_ITEMS; i++) {
    if (uris[i] != NULL && *uris[i] != '\0' && !tagore_recent_files_contains_uri(self, uris[i], NULL))
      g_ptr_array_add(self->uris, g_strdup(uris[i]));
  }
}

static void
tagore_recent_files_save(TagoreRecentFiles *self)
{
  g_autoptr(GKeyFile) key_file = g_key_file_new();
  g_autofree char *data = NULL;
  g_autofree char *directory = g_path_get_dirname(self->storage_path);
  const char **uris = NULL;
  gsize data_length = 0;

  if (g_mkdir_with_parents(directory, 0700) != 0)
    return;

  uris = g_new0(const char *, self->uris->len + 1);
  for (guint i = 0; i < self->uris->len; i++)
    uris[i] = g_ptr_array_index(self->uris, i);

  g_key_file_set_string_list(key_file, "Recent Files", "uris", uris, self->uris->len);

  data = g_key_file_to_data(key_file, &data_length, NULL);
  if (data == NULL) {
    g_free(uris);
    return;
  }

  g_file_set_contents(self->storage_path, data, (gssize)data_length, NULL);
  g_free(uris);
}

TagoreRecentFiles *
tagore_recent_files_new(void)
{
  TagoreRecentFiles *recent_files = g_object_new(TAGORE_TYPE_RECENT_FILES, NULL);

  tagore_recent_files_load(recent_files);

  return recent_files;
}

void
tagore_recent_files_add_file(TagoreRecentFiles *self, GFile *file)
{
  g_autofree char *uri = NULL;
  guint old_position = 0;

  g_return_if_fail(TAGORE_IS_RECENT_FILES(self));
  g_return_if_fail(G_IS_FILE(file));

  uri = g_file_get_uri(file);

  if (tagore_recent_files_contains_uri(self, uri, &old_position))
    g_ptr_array_remove_index(self->uris, old_position);

  g_ptr_array_insert(self->uris, 0, g_strdup(uri));

  while (self->uris->len > TAGORE_RECENT_FILES_MAX_ITEMS)
    g_ptr_array_remove_index(self->uris, self->uris->len - 1);

  tagore_recent_files_save(self);
}

GList *
tagore_recent_files_get_files(TagoreRecentFiles *self)
{
  GList *files = NULL;

  g_return_val_if_fail(TAGORE_IS_RECENT_FILES(self), NULL);

  for (guint i = 0; i < self->uris->len; i++) {
    const char *uri = g_ptr_array_index(self->uris, i);

    files = g_list_append(files, g_file_new_for_uri(uri));
  }

  return files;
}
