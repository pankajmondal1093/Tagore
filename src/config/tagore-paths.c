#include "tagore/tagore-paths.h"

static char *
tagore_paths_build_child(const char *base_dir)
{
  g_return_val_if_fail(base_dir != NULL, NULL);

  return g_build_filename(base_dir, "tagore", NULL);
}

char *
tagore_paths_get_config_dir(void)
{
  return tagore_paths_build_child(g_get_user_config_dir());
}

char *
tagore_paths_get_cache_dir(void)
{
  return tagore_paths_build_child(g_get_user_cache_dir());
}

char *
tagore_paths_get_data_dir(void)
{
  return tagore_paths_build_child(g_get_user_data_dir());
}
