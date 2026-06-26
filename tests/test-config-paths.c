#include "tagore/tagore-paths.h"

#include <glib.h>

static void
assert_path_has_tagore_basename(const char *path)
{
  g_autofree char *basename = NULL;

  g_assert_nonnull(path);

  basename = g_path_get_basename(path);
  g_assert_cmpstr(basename, ==, "tagore");
}

static void
test_config_dir(void)
{
  g_autofree char *path = tagore_paths_get_config_dir();

  assert_path_has_tagore_basename(path);
}

static void
test_cache_dir(void)
{
  g_autofree char *path = tagore_paths_get_cache_dir();

  assert_path_has_tagore_basename(path);
}

static void
test_data_dir(void)
{
  g_autofree char *path = tagore_paths_get_data_dir();

  assert_path_has_tagore_basename(path);
}

int
main(int argc, char *argv[])
{
  g_test_init(&argc, &argv, NULL);

  g_test_add_func("/tagore/config-paths/config-dir", test_config_dir);
  g_test_add_func("/tagore/config-paths/cache-dir", test_cache_dir);
  g_test_add_func("/tagore/config-paths/data-dir", test_data_dir);

  return g_test_run();
}
