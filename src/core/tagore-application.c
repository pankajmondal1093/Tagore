#include "tagore/tagore-application.h"

#include "tagore/tagore-window.h"

struct _TagoreApplication
{
  AdwApplication parent_instance;
};

G_DEFINE_TYPE(TagoreApplication, tagore_application, ADW_TYPE_APPLICATION)

static void
tagore_application_activate(GApplication *application)
{
  GtkWindow *window = gtk_application_get_active_window(GTK_APPLICATION(application));

  if (window == NULL)
    window = GTK_WINDOW(tagore_window_new(ADW_APPLICATION(application)));

  gtk_window_present(window);
}

static void
tagore_application_class_init(TagoreApplicationClass *klass)
{
  GApplicationClass *application_class = G_APPLICATION_CLASS(klass);

  application_class->activate = tagore_application_activate;
}

static void
tagore_application_init(TagoreApplication *self)
{
  (void)self;
}

TagoreApplication *
tagore_application_new(void)
{
  return g_object_new(
    TAGORE_TYPE_APPLICATION,
    "application-id", TAGORE_APP_ID,
    "flags", G_APPLICATION_DEFAULT_FLAGS,
    NULL);
}
