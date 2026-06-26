#pragma once

#include <adwaita.h>

G_BEGIN_DECLS

#define TAGORE_TYPE_APPLICATION (tagore_application_get_type())

G_DECLARE_FINAL_TYPE(TagoreApplication, tagore_application, TAGORE, APPLICATION, AdwApplication)

/**
 * tagore_application_new:
 *
 * Creates the Tagore application object.
 *
 * Returns: (transfer full): a new #TagoreApplication.
 */
TagoreApplication *tagore_application_new(void);

G_END_DECLS
