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
  GtkWidget *zoom_label;
  gboolean fullscreen;
};

G_DEFINE_TYPE(TagoreWindow, tagore_window, ADW_TYPE_APPLICATION_WINDOW)

static void tagore_window_open_file(TagoreWindow *self, GFile *file);
static void tagore_window_recent_button_clicked(GtkButton *button, gpointer user_data);
static void tagore_window_present_open_dialog(TagoreWindow *self);

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
tagore_window_present_open_dialog(TagoreWindow *self)
{
  g_autoptr(GtkFileDialog) dialog = NULL;
  g_autoptr(GtkFileFilter) image_filter = NULL;
  g_autoptr(GListStore) filters = NULL;

  g_return_if_fail(TAGORE_IS_WINDOW(self));

  dialog = gtk_file_dialog_new();
  gtk_file_dialog_set_title(dialog, "Open Image");

  image_filter = tagore_window_create_image_filter();
  filters = g_list_store_new(GTK_TYPE_FILE_FILTER);
  g_list_store_append(filters, image_filter);

  gtk_file_dialog_set_filters(dialog, G_LIST_MODEL(filters));
  gtk_file_dialog_set_default_filter(dialog, image_filter);
  gtk_file_dialog_open(dialog, GTK_WINDOW(self), NULL, tagore_window_open_dialog_cb, g_object_ref(self));
}

static void
tagore_window_open_button_clicked(GtkButton *button, gpointer user_data)
{
  (void)button;

  tagore_window_present_open_dialog(TAGORE_WINDOW(user_data));
}

static void
tagore_window_update_zoom_label(TagoreWindow *self, double zoom)
{
  g_autofree char *label = NULL;

  label = g_strdup_printf("%.0f%%", zoom * 100.0);
  gtk_label_set_text(GTK_LABEL(self->zoom_label), label);
}

static void
tagore_window_canvas_zoom_changed(TagoreCanvas *canvas, double zoom, gpointer user_data)
{
  (void)canvas;

  tagore_window_update_zoom_label(TAGORE_WINDOW(user_data), zoom);
}

static void
tagore_window_open_action(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  (void)action;
  (void)parameter;

  tagore_window_present_open_dialog(TAGORE_WINDOW(user_data));
}

static void
tagore_window_zoom_in_action(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  (void)action;
  (void)parameter;

  tagore_canvas_zoom_in(TAGORE_WINDOW(user_data)->canvas);
}

static void
tagore_window_zoom_out_action(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  (void)action;
  (void)parameter;

  tagore_canvas_zoom_out(TAGORE_WINDOW(user_data)->canvas);
}

static void
tagore_window_fit_action(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  (void)action;
  (void)parameter;

  tagore_canvas_fit_to_window(TAGORE_WINDOW(user_data)->canvas);
}

static void
tagore_window_actual_size_action(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  (void)action;
  (void)parameter;

  tagore_canvas_actual_size(TAGORE_WINDOW(user_data)->canvas);
}

static void
tagore_window_fullscreen_action(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  TagoreWindow *self = TAGORE_WINDOW(user_data);

  (void)action;
  (void)parameter;

  if (self->fullscreen)
    gtk_window_unfullscreen(GTK_WINDOW(self));
  else
    gtk_window_fullscreen(GTK_WINDOW(self));

  self->fullscreen = !self->fullscreen;
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
tagore_window_create_icon_button(const char *action_name, const char *icon_name, const char *tooltip)
{
  GtkWidget *button = gtk_button_new_from_icon_name(icon_name);

  gtk_actionable_set_action_name(GTK_ACTIONABLE(button), action_name);
  gtk_widget_set_tooltip_text(button, tooltip);

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

static GtkWidget *
tagore_window_create_status_bar(TagoreWindow *self)
{
  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);

  self->zoom_label = gtk_label_new("100%");
  gtk_widget_add_css_class(box, "toolbar");
  gtk_widget_set_margin_start(box, 12);
  gtk_widget_set_margin_end(box, 12);
  gtk_widget_set_margin_top(box, 6);
  gtk_widget_set_margin_bottom(box, 6);
  gtk_widget_set_halign(self->zoom_label, GTK_ALIGN_END);
  gtk_widget_set_hexpand(self->zoom_label, TRUE);
  gtk_box_append(GTK_BOX(box), self->zoom_label);

  return box;
}

static void
tagore_window_add_actions(TagoreWindow *self)
{
  static const GActionEntry actions[] = {
    { .name = "open", .activate = tagore_window_open_action },
    { .name = "zoom-in", .activate = tagore_window_zoom_in_action },
    { .name = "zoom-out", .activate = tagore_window_zoom_out_action },
    { .name = "fit", .activate = tagore_window_fit_action },
    { .name = "actual-size", .activate = tagore_window_actual_size_action },
    { .name = "fullscreen", .activate = tagore_window_fullscreen_action },
  };

  g_action_map_add_action_entries(G_ACTION_MAP(self), actions, G_N_ELEMENTS(actions), self);
}

static void
tagore_window_set_accelerators(TagoreWindow *self)
{
  GtkApplication *application = gtk_window_get_application(GTK_WINDOW(self));

  if (application == NULL)
    return;

  gtk_application_set_accels_for_action(application, "win.open", (const char *[]){"<Control>o", NULL});
  gtk_application_set_accels_for_action(application, "win.zoom-in", (const char *[]){"<Control>plus", "<Control>equal", NULL});
  gtk_application_set_accels_for_action(application, "win.zoom-out", (const char *[]){"<Control>minus", NULL});
  gtk_application_set_accels_for_action(application, "win.fit", (const char *[]){"<Control>0", NULL});
  gtk_application_set_accels_for_action(application, "win.actual-size", (const char *[]){"<Control>1", NULL});
  gtk_application_set_accels_for_action(application, "win.fullscreen", (const char *[]){"F11", NULL});
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
  GtkWidget *zoom_in_button = tagore_window_create_icon_button("win.zoom-in", "zoom-in-symbolic", "Zoom In");
  GtkWidget *zoom_out_button = tagore_window_create_icon_button("win.zoom-out", "zoom-out-symbolic", "Zoom Out");
  GtkWidget *fit_button = tagore_window_create_icon_button("win.fit", "zoom-fit-best-symbolic", "Fit to Window");
  GtkWidget *actual_size_button = tagore_window_create_icon_button("win.actual-size", "zoom-original-symbolic", "Actual Size");
  GtkWidget *fullscreen_button = tagore_window_create_icon_button("win.fullscreen", "view-fullscreen-symbolic", "Fullscreen");
  GtkWidget *status_bar = NULL;
  GtkDropTarget *drop_target = NULL;

  self->recent_files = tagore_recent_files_new();
  self->canvas = TAGORE_CANVAS(tagore_canvas_new());
  self->recent_menu_button = tagore_window_create_recent_menu(self);
  status_bar = tagore_window_create_status_bar(self);

  gtk_window_set_title(GTK_WINDOW(self), "Tagore");
  gtk_window_set_default_size(GTK_WINDOW(self), 1100, 760);

  tagore_window_add_actions(self);

  adw_header_bar_pack_start(ADW_HEADER_BAR(header_bar), open_button);
  adw_header_bar_pack_start(ADW_HEADER_BAR(header_bar), zoom_out_button);
  adw_header_bar_pack_start(ADW_HEADER_BAR(header_bar), zoom_in_button);
  adw_header_bar_pack_start(ADW_HEADER_BAR(header_bar), fit_button);
  adw_header_bar_pack_start(ADW_HEADER_BAR(header_bar), actual_size_button);
  adw_header_bar_pack_end(ADW_HEADER_BAR(header_bar), self->recent_menu_button);
  adw_header_bar_pack_end(ADW_HEADER_BAR(header_bar), fullscreen_button);

  adw_toolbar_view_add_top_bar(ADW_TOOLBAR_VIEW(toolbar_view), header_bar);
  adw_toolbar_view_add_bottom_bar(ADW_TOOLBAR_VIEW(toolbar_view), status_bar);
  adw_toolbar_view_set_content(ADW_TOOLBAR_VIEW(toolbar_view), GTK_WIDGET(self->canvas));

  adw_application_window_set_content(ADW_APPLICATION_WINDOW(self), toolbar_view);

  drop_target = gtk_drop_target_new(GDK_TYPE_FILE_LIST, GDK_ACTION_COPY);
  g_signal_connect(drop_target, "drop", G_CALLBACK(tagore_window_drop_cb), self);
  gtk_widget_add_controller(GTK_WIDGET(self->canvas), GTK_EVENT_CONTROLLER(drop_target));
  g_signal_connect(self->canvas, "zoom-changed", G_CALLBACK(tagore_window_canvas_zoom_changed), self);

  tagore_window_refresh_recent_files(self);
  tagore_window_update_zoom_label(self, tagore_canvas_get_zoom(self->canvas));
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
  TagoreWindow *window = NULL;

  g_return_val_if_fail(ADW_IS_APPLICATION(application), NULL);

  window = g_object_new(
    TAGORE_TYPE_WINDOW,
    "application", application,
    NULL);

  tagore_window_set_accelerators(window);

  return window;
}
