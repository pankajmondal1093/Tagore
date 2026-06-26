#pragma once

#include <glib.h>

G_BEGIN_DECLS

/**
 * tagore_paths_get_config_dir:
 *
 * Returns Tagore's configuration directory following the XDG base directory
 * specification, normally `~/.config/tagore`.
 *
 * Returns: (transfer full): a newly allocated path.
 */
char *tagore_paths_get_config_dir(void);

/**
 * tagore_paths_get_cache_dir:
 *
 * Returns Tagore's cache directory following the XDG base directory
 * specification, normally `~/.cache/tagore`.
 *
 * Returns: (transfer full): a newly allocated path.
 */
char *tagore_paths_get_cache_dir(void);

/**
 * tagore_paths_get_data_dir:
 *
 * Returns Tagore's user data directory following the XDG base directory
 * specification, normally `~/.local/share/tagore`.
 *
 * Returns: (transfer full): a newly allocated path.
 */
char *tagore_paths_get_data_dir(void);

G_END_DECLS
