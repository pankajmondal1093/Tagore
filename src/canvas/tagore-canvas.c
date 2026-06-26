#include "tagore/tagore-canvas.h"

struct _TagoreCanvas
{
  GtkWidget parent_instance;

  GdkTexture *texture;
};

G_DEFINE_TYPE(TagoreCanvas, tagore_canvas, GTK_TYPE_WIDGET)

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

  const double scale_x = (double)widget_width / (double)texture_width;
  const double scale_y = (double)widget_height / (double)texture_height;
  const double scale = MIN(scale_x, scale_y);
  const double draw_width = (double)texture_width * scale;
  const double draw_height = (double)texture_height * scale;
  const double x = ((double)widget_width - draw_width) / 2.0;
  const double y = ((double)widget_height - draw_height) / 2.0;
  graphene_rect_t image_rect;

  graphene_rect_init(
    &image_rect,
    (float)x,
    (float)y,
    (float)draw_width,
    (float)draw_height);

  gtk_snapshot_append_texture(snapshot, self->texture, &image_rect);
}

static void
tagore_canvas_class_init(TagoreCanvasClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

  object_class->dispose = tagore_canvas_dispose;
  widget_class->snapshot = tagore_canvas_snapshot;
}

static void
tagore_canvas_init(TagoreCanvas *self)
{
  gtk_widget_set_hexpand(GTK_WIDGET(self), TRUE);
  gtk_widget_set_vexpand(GTK_WIDGET(self), TRUE);
  gtk_widget_set_focusable(GTK_WIDGET(self), TRUE);
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

  if (g_set_object(&self->texture, texture))
    gtk_widget_queue_draw(GTK_WIDGET(self));
}

void
tagore_canvas_clear(TagoreCanvas *self)
{
  tagore_canvas_set_texture(self, NULL);
}
