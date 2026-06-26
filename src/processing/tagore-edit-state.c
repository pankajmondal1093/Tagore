#include "tagore/tagore-edit-state.h"

static TagoreRotation
tagore_rotation_from_degrees(int degrees)
{
  int normalized = degrees % 360;

  if (normalized < 0)
    normalized += 360;

  switch (normalized) {
  case 0:
    return TAGORE_ROTATION_0;
  case 90:
    return TAGORE_ROTATION_90;
  case 180:
    return TAGORE_ROTATION_180;
  case 270:
    return TAGORE_ROTATION_270;
  default:
    g_assert_not_reached();
  }
}

static gboolean
tagore_double_is_near(double value, double expected)
{
  const double epsilon = 0.000001;
  const double difference = value - expected;

  return difference >= -epsilon && difference <= epsilon;
}

void
tagore_edit_state_init(TagoreEditState *state)
{
  g_return_if_fail(state != NULL);

  *state = (TagoreEditState){
    .rotation = TAGORE_ROTATION_0,
    .flip_horizontal = FALSE,
    .flip_vertical = FALSE,
    .crop = {
      .enabled = FALSE,
      .x = 0.0,
      .y = 0.0,
      .width = 0.0,
      .height = 0.0,
      .preserve_aspect_ratio = FALSE,
    },
    .resize = {
      .enabled = FALSE,
      .width = 0,
      .height = 0,
      .maintain_aspect_ratio = TRUE,
      .interpolation = TAGORE_RESIZE_INTERPOLATION_LANCZOS,
    },
    .adjustments = {
      .brightness = 0.0,
      .contrast = 1.0,
      .saturation = 1.0,
      .exposure = 0.0,
      .gamma = 1.0,
    },
  };
}

gboolean
tagore_edit_state_is_identity(const TagoreEditState *state)
{
  g_return_val_if_fail(state != NULL, TRUE);

  return state->rotation == TAGORE_ROTATION_0 &&
         !state->flip_horizontal &&
         !state->flip_vertical &&
         !state->crop.enabled &&
         !state->resize.enabled &&
         tagore_double_is_near(state->adjustments.brightness, 0.0) &&
         tagore_double_is_near(state->adjustments.contrast, 1.0) &&
         tagore_double_is_near(state->adjustments.saturation, 1.0) &&
         tagore_double_is_near(state->adjustments.exposure, 0.0) &&
         tagore_double_is_near(state->adjustments.gamma, 1.0);
}

void
tagore_edit_state_rotate_left(TagoreEditState *state)
{
  g_return_if_fail(state != NULL);

  state->rotation = tagore_rotation_from_degrees((int)state->rotation - 90);
}

void
tagore_edit_state_rotate_right(TagoreEditState *state)
{
  g_return_if_fail(state != NULL);

  state->rotation = tagore_rotation_from_degrees((int)state->rotation + 90);
}

void
tagore_edit_state_flip_horizontal(TagoreEditState *state)
{
  g_return_if_fail(state != NULL);

  state->flip_horizontal = !state->flip_horizontal;
}

void
tagore_edit_state_flip_vertical(TagoreEditState *state)
{
  g_return_if_fail(state != NULL);

  state->flip_vertical = !state->flip_vertical;
}
