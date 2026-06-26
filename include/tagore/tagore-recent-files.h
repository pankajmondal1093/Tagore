#pragma once

#include <gio/gio.h>

G_BEGIN_DECLS

#define TAGORE_TYPE_RECENT_FILES (tagore_recent_files_get_type())
#define TAGORE_RECENT_FILES_MAX_ITEMS 10

G_DECLARE_FINAL_TYPE(TagoreRecentFiles, tagore_recent_files, TAGORE, RECENT_FILES, GObject)

/**
 * tagore_recent_files_new:
 *
 * Creates a recent-file service and loads persisted entries.
 *
 * Returns: (transfer full): a new #TagoreRecentFiles.
 */
TagoreRecentFiles *tagore_recent_files_new(void);

/**
 * tagore_recent_files_add_file:
 * @self: a #TagoreRecentFiles.
 * @file: file to add.
 *
 * Adds @file to the front of the recent-file list and persists the list.
 */
void tagore_recent_files_add_file(TagoreRecentFiles *self, GFile *file);

/**
 * tagore_recent_files_get_files:
 * @self: a #TagoreRecentFiles.
 *
 * Returns recent files in most-recent-first order.
 *
 * Returns: (transfer full) (element-type GFile): a list of #GFile objects.
 */
GList *tagore_recent_files_get_files(TagoreRecentFiles *self);

G_END_DECLS
