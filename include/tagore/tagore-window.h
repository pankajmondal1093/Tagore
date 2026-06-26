#pragma once

#include <adwaita.h>

G_BEGIN_DECLS

#define TAGORE_TYPE_WINDOW (tagore_window_get_type())

G_DECLARE_FINAL_TYPE(TagoreWindow, tagore_window, TAGORE, WINDOW, AdwApplicationWindow)

/**
 * tagore_window_new:
 * @application: the owning application.
 *
 * Creates the main Tagore window.
 *
 * Returns: (transfer full): a new #TagoreWindow.
 */
TagoreWindow *tagore_window_new(AdwApplication *application);

G_END_DECLS
