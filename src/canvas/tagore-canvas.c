#include "tagore/tagore-canvas.h"

struct _TagoreCanvas
{
  GtkWidget parent_instance;

  GdkTexture *texture;
  TagoreCanvasZoomMode zoom_mode;
  double custom_zoom;
  double pan_x;
  double pan_y;
  double drag_start_pan_x;
  double drag_start_pan_y;
  gboolean space_pressed;
  gboolean drag_panning;
};

G_DEFINE_TYPE(TagoreCanvas, tagore_canvas, GTK_TYPE_WIDGET)

enum
{
  ZOOM_CHANGED,
  N_SIGNALS,
};

static guint signals[N_SIGNALS];

static double
tagore_canvas_clamp_zoom(double zoom)
{
  return CLAMP(zoom, 0.05, 32.0);
}

static gboolean
tagore_canvas_has_texture(TagoreCanvas *self)
{
  return self->texture != NULL &&
         gdk_texture_get_width(self->texture) > 0 &&
         gdk_texture_get_height(self->texture) > 0;
}

static double
tagore_canvas_get_fit_zoom_for_size(TagoreCanvas *self, int widget_width, int widget_height)
{
  const int texture_width = gdk_texture_get_width(self->texture);
  const int texture_height = gdk_texture_get_height(self->texture);
  const double scale_x = (double)widget_width / (double)texture_width;
  const double scale_y = (double)widget_height / (double)texture_height;

  if (widget_width <= 0 || widget_height <= 0 || texture_width <= 0 || texture_height <= 0)
    return 1.0;

  return tagore_canvas_clamp_zoom(MIN(scale_x, scale_y));
}

static double
tagore_canvas_get_effective_zoom_for_size(TagoreCanvas *self, int widget_width, int widget_height)
{
  if (!tagore_canvas_has_texture(self))
    return 1.0;

  switch (self->zoom_mode) {
  case TAGORE_CANVAS_ZOOM_MODE_FIT:
    return tagore_canvas_get_fit_zoom_for_size(self, widget_width, widget_height);
  case TAGORE_CANVAS_ZOOM_MODE_ACTUAL_SIZE:
    return 1.0;
  case TAGORE_CANVAS_ZOOM_MODE_CUSTOM:
    return tagore_canvas_clamp_zoom(self->custom_zoom);
  default:
    g_assert_not_reached();
  }
}

static double
tagore_canvas_get_effective_zoom(TagoreCanvas *self)
{
  return tagore_canvas_get_effective_zoom_for_size(
    self,
    gtk_widget_get_width(GTK_WIDGET(self)),
    gtk_widget_get_height(GTK_WIDGET(self)));
}

static void
tagore_canvas_emit_zoom_changed(TagoreCanvas *self)
{
  g_signal_emit(self, signals[ZOOM_CHANGED], 0, tagore_canvas_get_effective_zoom(self));
}

static void
tagore_canvas_reset_pan(TagoreCanvas *self)
{
  self->pan_x = 0.0;
  self->pan_y = 0.0;
}

static void
tagore_canvas_set_cursor_for_pan(TagoreCanvas *self)
{
  GtkWidget *widget = GTK_WIDGET(self);

  if (self->space_pressed || self->drag_panning)
    gtk_widget_set_cursor_from_name(widget, self->drag_panning ? "grabbing" : "grab");
  else
    gtk_widget_set_cursor(widget, NULL);
}

static void
tagore_canvas_dispose(GObject *object)
{
  TagoreCanvas *self = TAGORE_CANVAS(object);

  g_clear_object(&self->texture);

  G_OBJECT_CLASS(tagore_canvas_parent_class)->dispose(object);
}

static void
tagore_canvas_snapshot(GtkWidget *widget, GtkSnapshot *snapshot)
{
  TagoreCanvas *self = TAGORE_CANVAS(widget);
  const int widget_width = gtk_widget_get_width(widget);
  const int widget_height = gtk_widget_get_height(widget);
  graphene_rect_t bounds;

  graphene_rect_init(&bounds, 0.0f, 0.0f, (float)widget_width, (float)widget_height);
  gtk_snapshot_append_color(snapshot, &(GdkRGBA){0.12, 0.12, 0.12, 1.0}, &bounds);

  if (self->texture == NULL || widget_width <= 0 || widget_height <= 0)
    return;

  const int texture_width = gdk_texture_get_width(self->texture);
  const int texture_height = gdk_texture_get_height(self->texture);

  if (texture_width <= 0 || texture_height <= 0)
    return;

  const double scale = tagore_canvas_get_effective_zoom_for_size(self, widget_width, widget_height);
  const double draw_width = (double)texture_width * scale;
  const double draw_height = (double)texture_height * scale;
  const double x = (((double)widget_width - draw_width) / 2.0) + self->pan_x;
  const double y = (((double)widget_height - draw_height) / 2.0) + self->pan_y;
  graphene_rect_t image_rect;

  graphene_rect_init(
    &image_rect,
    (float)x,
    (float)y,
    (float)draw_width,
    (float)draw_height);

  gtk_snapshot_append_texture(snapshot, self->texture, &image_rect);
}

static gboolean
tagore_canvas_key_pressed(GtkEventControllerKey *controller,
                          guint keyval,
                          guint keycode,
                          GdkModifierType state,
                          gpointer user_data)
{
  TagoreCanvas *self = TAGORE_CANVAS(user_data);

  (void)controller;
  (void)keycode;
  (void)state;

  if (keyval == GDK_KEY_space) {
    self->space_pressed = TRUE;
    tagore_canvas_set_cursor_for_pan(self);
    return TRUE;
  }

  return FALSE;
}

static void
tagore_canvas_key_released(GtkEventControllerKey *controller,
                           guint keyval,
                           guint keycode,
                           GdkModifierType state,
                           gpointer user_data)
{
  TagoreCanvas *self = TAGORE_CANVAS(user_data);

  (void)controller;
  (void)keycode;
  (void)state;

  if (keyval == GDK_KEY_space) {
    self->space_pressed = FALSE;
    tagore_canvas_set_cursor_for_pan(self);
  }
}

static gboolean
tagore_canvas_scroll(GtkEventControllerScroll *controller,
                     double dx,
                     double dy,
                     gpointer user_data)
{
  TagoreCanvas *self = TAGORE_CANVAS(user_data);

  (void)controller;
  (void)dx;

  if (!tagore_canvas_has_texture(self))
    return FALSE;

  if (dy < 0.0)
    tagore_canvas_zoom_in(self);
  else if (dy > 0.0)
    tagore_canvas_zoom_out(self);

  return TRUE;
}

static void
tagore_canvas_drag_begin(GtkGestureDrag *gesture, double start_x, double start_y, gpointer user_data)
{
  TagoreCanvas *self = TAGORE_CANVAS(user_data);
  GdkModifierType state = gtk_event_controller_get_current_event_state(GTK_EVENT_CONTROLLER(gesture));

  (void)start_x;
  (void)start_y;

  self->drag_panning = self->space_pressed || (state & GDK_BUTTON2_MASK) != 0;
  self->drag_start_pan_x = self->pan_x;
  self->drag_start_pan_y = self->pan_y;
  tagore_canvas_set_cursor_for_pan(self);
}

static void
tagore_canvas_drag_update(GtkGestureDrag *gesture, double offset_x, double offset_y, gpointer user_data)
{
  TagoreCanvas *self = TAGORE_CANVAS(user_data);

  (void)gesture;

  if (!self->drag_panning)
    return;

  self->pan_x = self->drag_start_pan_x + offset_x;
  self->pan_y = self->drag_start_pan_y + offset_y;
  gtk_widget_queue_draw(GTK_WIDGET(self));
}

static void
tagore_canvas_drag_end(GtkGestureDrag *gesture, double offset_x, double offset_y, gpointer user_data)
{
  TagoreCanvas *self = TAGORE_CANVAS(user_data);

  (void)gesture;
  (void)offset_x;
  (void)offset_y;

  self->drag_panning = FALSE;
  tagore_canvas_set_cursor_for_pan(self);
}

static void
tagore_canvas_class_init(TagoreCanvasClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

  object_class->dispose = tagore_canvas_dispose;
  widget_class->snapshot = tagore_canvas_snapshot;

  /**
   * TagoreCanvas::zoom-changed:
   * @self: a #TagoreCanvas.
   * @zoom: current effective zoom factor.
   *
   * Emitted when the viewer zoom changes.
   */
  signals[ZOOM_CHANGED] = g_signal_new(
    "zoom-changed",
    G_TYPE_FROM_CLASS(klass),
    G_SIGNAL_RUN_LAST,
    0,
    NULL,
    NULL,
    NULL,
    G_TYPE_NONE,
    1,
    G_TYPE_DOUBLE);
}

static void
tagore_canvas_init(TagoreCanvas *self)
{
  GtkEventController *key_controller = gtk_event_controller_key_new();
  GtkEventController *scroll_controller = gtk_event_controller_scroll_new(GTK_EVENT_CONTROLLER_SCROLL_VERTICAL);
  GtkGesture *drag_gesture = gtk_gesture_drag_new();

  self->zoom_mode = TAGORE_CANVAS_ZOOM_MODE_FIT;
  self->custom_zoom = 1.0;

  gtk_widget_set_hexpand(GTK_WIDGET(self), TRUE);
  gtk_widget_set_vexpand(GTK_WIDGET(self), TRUE);
  gtk_widget_set_focusable(GTK_WIDGET(self), TRUE);

  g_signal_connect(key_controller, "key-pressed", G_CALLBACK(tagore_canvas_key_pressed), self);
  g_signal_connect(key_controller, "key-released", G_CALLBACK(tagore_canvas_key_released), self);
  gtk_widget_add_controller(GTK_WIDGET(self), key_controller);

  g_signal_connect(scroll_controller, "scroll", G_CALLBACK(tagore_canvas_scroll), self);
  gtk_widget_add_controller(GTK_WIDGET(self), scroll_controller);

  gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(drag_gesture), 0);
  g_signal_connect(drag_gesture, "drag-begin", G_CALLBACK(tagore_canvas_drag_begin), self);
  g_signal_connect(drag_gesture, "drag-update", G_CALLBACK(tagore_canvas_drag_update), self);
  g_signal_connect(drag_gesture, "drag-end", G_CALLBACK(tagore_canvas_drag_end), self);
  gtk_widget_add_controller(GTK_WIDGET(self), GTK_EVENT_CONTROLLER(drag_gesture));
}

GtkWidget *
tagore_canvas_new(void)
{
  return g_object_new(TAGORE_TYPE_CANVAS, NULL);
}

void
tagore_canvas_set_texture(TagoreCanvas *self, GdkTexture *texture)
{
  g_return_if_fail(TAGORE_IS_CANVAS(self));
  g_return_if_fail(texture == NULL || GDK_IS_TEXTURE(texture));

  if (g_set_object(&self->texture, texture)) {
    self->zoom_mode = TAGORE_CANVAS_ZOOM_MODE_FIT;
    self->custom_zoom = 1.0;
    tagore_canvas_reset_pan(self);
    gtk_widget_queue_draw(GTK_WIDGET(self));
    tagore_canvas_emit_zoom_changed(self);
  }
}

void
tagore_canvas_clear(TagoreCanvas *self)
{
  tagore_canvas_set_texture(self, NULL);
}

void
tagore_canvas_zoom_in(TagoreCanvas *self)
{
  g_return_if_fail(TAGORE_IS_CANVAS(self));

  self->custom_zoom = tagore_canvas_clamp_zoom(tagore_canvas_get_effective_zoom(self) * 1.25);
  self->zoom_mode = TAGORE_CANVAS_ZOOM_MODE_CUSTOM;
  gtk_widget_queue_draw(GTK_WIDGET(self));
  tagore_canvas_emit_zoom_changed(self);
}

void
tagore_canvas_zoom_out(TagoreCanvas *self)
{
  g_return_if_fail(TAGORE_IS_CANVAS(self));

  self->custom_zoom = tagore_canvas_clamp_zoom(tagore_canvas_get_effective_zoom(self) / 1.25);
  self->zoom_mode = TAGORE_CANVAS_ZOOM_MODE_CUSTOM;
  gtk_widget_queue_draw(GTK_WIDGET(self));
  tagore_canvas_emit_zoom_changed(self);
}

void
tagore_canvas_fit_to_window(TagoreCanvas *self)
{
  g_return_if_fail(TAGORE_IS_CANVAS(self));

  self->zoom_mode = TAGORE_CANVAS_ZOOM_MODE_FIT;
  tagore_canvas_reset_pan(self);
  gtk_widget_queue_draw(GTK_WIDGET(self));
  tagore_canvas_emit_zoom_changed(self);
}

void
tagore_canvas_actual_size(TagoreCanvas *self)
{
  g_return_if_fail(TAGORE_IS_CANVAS(self));

  self->zoom_mode = TAGORE_CANVAS_ZOOM_MODE_ACTUAL_SIZE;
  tagore_canvas_reset_pan(self);
  gtk_widget_queue_draw(GTK_WIDGET(self));
  tagore_canvas_emit_zoom_changed(self);
}

double
tagore_canvas_get_zoom(TagoreCanvas *self)
{
  g_return_val_if_fail(TAGORE_IS_CANVAS(self), 1.0);

  return tagore_canvas_get_effective_zoom(self);
}
