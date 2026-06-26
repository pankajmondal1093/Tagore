#include "tagore/tagore-window.h"

#include "tagore/tagore-canvas.h"
#include "tagore/tagore-image-loader.h"
#include "tagore/tagore-recent-files.h"

struct _TagoreWindow
{
  AdwApplicationWindow parent_instance;

  TagoreCanvas *canvas;
  TagoreRecentFiles *recent_files;
  GtkWidget *recent_box;
  GtkWidget *recent_menu_button;
};

G_DEFINE_TYPE(TagoreWindow, tagore_window, ADW_TYPE_APPLICATION_WINDOW)

static void tagore_window_open_file(TagoreWindow *self, GFile *file);
static void tagore_window_recent_button_clicked(GtkButton *button, gpointer user_data);

static void
tagore_window_show_error(TagoreWindow *self, const char *heading, const char *body)
{
  GtkWidget *dialog = adw_message_dialog_new(GTK_WINDOW(self), heading, body);

  adw_message_dialog_add_response(ADW_MESSAGE_DIALOG(dialog), "ok", "OK");
  adw_message_dialog_set_default_response(ADW_MESSAGE_DIALOG(dialog), "ok");
  adw_message_dialog_set_close_response(ADW_MESSAGE_DIALOG(dialog), "ok");
  gtk_window_present(GTK_WINDOW(dialog));
}

static GtkFileFilter *
tagore_window_create_image_filter(void)
{
  GtkFileFilter *filter = gtk_file_filter_new();

  gtk_file_filter_set_name(filter, "Supported Images");
  gtk_file_filter_add_mime_type(filter, "image/png");
  gtk_file_filter_add_mime_type(filter, "image/jpeg");
  gtk_file_filter_add_mime_type(filter, "image/webp");
  gtk_file_filter_add_pattern(filter, "*.png");
  gtk_file_filter_add_pattern(filter, "*.jpg");
  gtk_file_filter_add_pattern(filter, "*.jpeg");
  gtk_file_filter_add_pattern(filter, "*.webp");

  return filter;
}

static void
tagore_window_refresh_recent_files(TagoreWindow *self)
{
  g_autolist(GFile) files = NULL;

  while (gtk_widget_get_first_child(self->recent_box) != NULL)
    gtk_box_remove(GTK_BOX(self->recent_box), gtk_widget_get_first_child(self->recent_box));

  files = tagore_recent_files_get_files(self->recent_files);

  if (files == NULL) {
    GtkWidget *label = gtk_label_new("No Recent Images");

    gtk_widget_set_sensitive(label, FALSE);
    gtk_widget_set_margin_top(label, 12);
    gtk_widget_set_margin_bottom(label, 12);
    gtk_widget_set_margin_start(label, 12);
    gtk_widget_set_margin_end(label, 12);
    gtk_box_append(GTK_BOX(self->recent_box), label);
    return;
  }

  for (GList *link = files; link != NULL; link = link->next) {
    GFile *file = G_FILE(link->data);
    g_autofree char *basename = g_file_get_basename(file);
    GtkWidget *button = gtk_button_new_with_label(basename != NULL ? basename : "Image");

    gtk_widget_set_halign(button, GTK_ALIGN_FILL);
    gtk_button_set_has_frame(GTK_BUTTON(button), FALSE);
    g_object_set_data_full(G_OBJECT(button), "tagore-file", g_object_ref(file), g_object_unref);
    g_signal_connect(button, "clicked", G_CALLBACK(tagore_window_recent_button_clicked), self);
    gtk_box_append(GTK_BOX(self->recent_box), button);
  }
}

static void
tagore_window_open_file(TagoreWindow *self, GFile *file)
{
  g_autoptr(TagoreLoadedImage) image = NULL;
  g_autoptr(GError) error = NULL;
  g_autofree char *basename = NULL;

  g_return_if_fail(TAGORE_IS_WINDOW(self));
  g_return_if_fail(G_IS_FILE(file));

  image = tagore_image_loader_load_file(file, &error);
  if (image == NULL) {
    tagore_window_show_error(
      self,
      "Unable to Open Image",
      error != NULL ? error->message : "The selected file could not be opened.");
    return;
  }

  tagore_canvas_set_texture(self->canvas, tagore_loaded_image_get_texture(image));
  tagore_recent_files_add_file(self->recent_files, tagore_loaded_image_get_file(image));
  tagore_window_refresh_recent_files(self);

  basename = g_file_get_basename(file);
  if (basename != NULL)
    gtk_window_set_title(GTK_WINDOW(self), basename);
}

static void
tagore_window_recent_button_clicked(GtkButton *button, gpointer user_data)
{
  TagoreWindow *self = TAGORE_WINDOW(user_data);
  GFile *file = g_object_get_data(G_OBJECT(button), "tagore-file");

  if (file != NULL)
    tagore_window_open_file(self, file);
}

static void
tagore_window_open_dialog_cb(GObject *source_object, GAsyncResult *result, gpointer user_data)
{
  GtkFileDialog *dialog = GTK_FILE_DIALOG(source_object);
  g_autoptr(TagoreWindow) self = TAGORE_WINDOW(user_data);
  g_autoptr(GFile) file = NULL;
  g_autoptr(GError) error = NULL;

  file = gtk_file_dialog_open_finish(dialog, result, &error);
  if (file == NULL) {
    if (error != NULL && !g_error_matches(error, GTK_DIALOG_ERROR, GTK_DIALOG_ERROR_DISMISSED))
      tagore_window_show_error(self, "Unable to Open Image", error->message);
    return;
  }

  tagore_window_open_file(self, file);
}

static void
tagore_window_open_button_clicked(GtkButton *button, gpointer user_data)
{
  TagoreWindow *self = TAGORE_WINDOW(user_data);
  g_autoptr(GtkFileDialog) dialog = NULL;
  g_autoptr(GtkFileFilter) image_filter = NULL;
  g_autoptr(GListStore) filters = NULL;

  (void)button;

  dialog = gtk_file_dialog_new();
  gtk_file_dialog_set_title(dialog, "Open Image");

  image_filter = tagore_window_create_image_filter();
  filters = g_list_store_new(GTK_TYPE_FILE_FILTER);
  g_list_store_append(filters, image_filter);

  gtk_file_dialog_set_filters(dialog, G_LIST_MODEL(filters));
  gtk_file_dialog_set_default_filter(dialog, image_filter);
  gtk_file_dialog_open(dialog, GTK_WINDOW(self), NULL, tagore_window_open_dialog_cb, g_object_ref(self));
}

static gboolean
tagore_window_drop_cb(GtkDropTarget *target, const GValue *value, double x, double y, gpointer user_data)
{
  TagoreWindow *self = TAGORE_WINDOW(user_data);
  GdkFileList *file_list = g_value_get_object(value);
  const GSList *files = NULL;

  (void)target;
  (void)x;
  (void)y;

  if (file_list == NULL)
    return FALSE;

  files = gdk_file_list_get_files(file_list);
  for (const GSList *link = files; link != NULL; link = link->next) {
    GFile *file = G_FILE(link->data);

    if (tagore_image_loader_file_is_supported(file)) {
      tagore_window_open_file(self, file);
      return TRUE;
    }
  }

  tagore_window_show_error(
    self,
    "Unsupported Image",
    "Drop a PNG, JPEG, or WebP image file.");
  return FALSE;
}

static GtkWidget *
tagore_window_create_open_button(TagoreWindow *self)
{
  GtkWidget *button = gtk_button_new();
  GtkWidget *content = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
  GtkWidget *icon = gtk_image_new_from_icon_name("document-open-symbolic");
  GtkWidget *label = gtk_label_new("Open Image");

  gtk_box_append(GTK_BOX(content), icon);
  gtk_box_append(GTK_BOX(content), label);
  gtk_button_set_child(GTK_BUTTON(button), content);
  gtk_widget_set_tooltip_text(button, "Open Image");
  g_signal_connect(button, "clicked", G_CALLBACK(tagore_window_open_button_clicked), self);

  return button;
}

static GtkWidget *
tagore_window_create_recent_menu(TagoreWindow *self)
{
  GtkWidget *menu_button = gtk_menu_button_new();
  GtkWidget *popover = gtk_popover_new();

  self->recent_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
  gtk_widget_set_margin_top(self->recent_box, 6);
  gtk_widget_set_margin_bottom(self->recent_box, 6);
  gtk_widget_set_margin_start(self->recent_box, 6);
  gtk_widget_set_margin_end(self->recent_box, 6);

  gtk_menu_button_set_icon_name(GTK_MENU_BUTTON(menu_button), "document-open-recent-symbolic");
  gtk_widget_set_tooltip_text(menu_button, "Recent Images");
  gtk_popover_set_child(GTK_POPOVER(popover), self->recent_box);
  gtk_menu_button_set_popover(GTK_MENU_BUTTON(menu_button), popover);

  return menu_button;
}

static void
tagore_window_dispose(GObject *object)
{
  TagoreWindow *self = TAGORE_WINDOW(object);

  g_clear_object(&self->recent_files);

  G_OBJECT_CLASS(tagore_window_parent_class)->dispose(object);
}

static void
tagore_window_init(TagoreWindow *self)
{
  GtkWidget *toolbar_view = adw_toolbar_view_new();
  GtkWidget *header_bar = adw_header_bar_new();
  GtkWidget *open_button = tagore_window_create_open_button(self);
  GtkDropTarget *drop_target = NULL;

  self->recent_files = tagore_recent_files_new();
  self->canvas = TAGORE_CANVAS(tagore_canvas_new());
  self->recent_menu_button = tagore_window_create_recent_menu(self);

  gtk_window_set_title(GTK_WINDOW(self), "Tagore");
  gtk_window_set_default_size(GTK_WINDOW(self), 1100, 760);

  adw_header_bar_pack_start(ADW_HEADER_BAR(header_bar), open_button);
  adw_header_bar_pack_end(ADW_HEADER_BAR(header_bar), self->recent_menu_button);

  adw_toolbar_view_add_top_bar(ADW_TOOLBAR_VIEW(toolbar_view), header_bar);
  adw_toolbar_view_set_content(ADW_TOOLBAR_VIEW(toolbar_view), GTK_WIDGET(self->canvas));

  adw_application_window_set_content(ADW_APPLICATION_WINDOW(self), toolbar_view);

  drop_target = gtk_drop_target_new(GDK_TYPE_FILE_LIST, GDK_ACTION_COPY);
  g_signal_connect(drop_target, "drop", G_CALLBACK(tagore_window_drop_cb), self);
  gtk_widget_add_controller(GTK_WIDGET(self->canvas), GTK_EVENT_CONTROLLER(drop_target));

  tagore_window_refresh_recent_files(self);
}

static void
tagore_window_class_init(TagoreWindowClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->dispose = tagore_window_dispose;
}

TagoreWindow *
tagore_window_new(AdwApplication *application)
{
  g_return_val_if_fail(ADW_IS_APPLICATION(application), NULL);

  return g_object_new(
    TAGORE_TYPE_WINDOW,
    "application", application,
    NULL);
}
