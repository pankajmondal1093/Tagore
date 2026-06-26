#include "tagore/tagore-recent-files.h"

#include <glib.h>
#include <glib/gstdio.h>

static void
reset_recent_files_storage(void)
{
  g_autofree char *data_dir = g_build_filename(g_get_user_data_dir(), "tagore", NULL);
  g_autofree char *storage_path = g_build_filename(data_dir, "recent-files.ini", NULL);

  g_remove(storage_path);
}

static void
test_recent_files_adds_most_recent_first(void)
{
  reset_recent_files_storage();

  g_autoptr(TagoreRecentFiles) recent_files = tagore_recent_files_new();
  g_autoptr(GFile) first = g_file_new_for_uri("file:///tmp/tagore-first.png");
  g_autoptr(GFile) second = g_file_new_for_uri("file:///tmp/tagore-second.jpg");
  g_autolist(GFile) files = NULL;

  tagore_recent_files_add_file(recent_files, first);
  tagore_recent_files_add_file(recent_files, second);

  files = tagore_recent_files_get_files(recent_files);

  g_assert_cmpuint(g_list_length(files), ==, 2);
  g_assert_true(g_file_equal(G_FILE(files->data), second));
  g_assert_true(g_file_equal(G_FILE(files->next->data), first));
}

static void
test_recent_files_deduplicates(void)
{
  reset_recent_files_storage();

  g_autoptr(TagoreRecentFiles) recent_files = tagore_recent_files_new();
  g_autoptr(GFile) first = g_file_new_for_uri("file:///tmp/tagore-duplicate.png");
  g_autoptr(GFile) second = g_file_new_for_uri("file:///tmp/tagore-other.webp");
  g_autolist(GFile) files = NULL;

  tagore_recent_files_add_file(recent_files, first);
  tagore_recent_files_add_file(recent_files, second);
  tagore_recent_files_add_file(recent_files, first);

  files = tagore_recent_files_get_files(recent_files);

  g_assert_cmpuint(g_list_length(files), ==, 2);
  g_assert_true(g_file_equal(G_FILE(files->data), first));
  g_assert_true(g_file_equal(G_FILE(files->next->data), second));
}

int
main(int argc, char *argv[])
{
  g_autofree char *data_home = g_dir_make_tmp("tagore-recent-files-test-XXXXXX", NULL);

  if (data_home != NULL)
    g_setenv("XDG_DATA_HOME", data_home, TRUE);

  g_test_init(&argc, &argv, NULL);

  g_test_add_func("/tagore/recent-files/adds-most-recent-first", test_recent_files_adds_most_recent_first);
  g_test_add_func("/tagore/recent-files/deduplicates", test_recent_files_deduplicates);

  return g_test_run();
}
