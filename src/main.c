#include "tagore/tagore-application.h"

int
main(int argc, char *argv[])
{
  g_autoptr(TagoreApplication) app = tagore_application_new();

  return g_application_run(G_APPLICATION(app), argc, argv);
}
