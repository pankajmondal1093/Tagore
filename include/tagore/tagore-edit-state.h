#pragma once

#include <glib-object.h>

G_BEGIN_DECLS

typedef enum
{
  TAGORE_ROTATION_0 = 0,
  TAGORE_ROTATION_90 = 90,
  TAGORE_ROTATION_180 = 180,
  TAGORE_ROTATION_270 = 270,
} TagoreRotation;

typedef enum
{
  TAGORE_RESIZE_INTERPOLATION_NEAREST,
  TAGORE_RESIZE_INTERPOLATION_LINEAR,
  TAGORE_RESIZE_INTERPOLATION_CUBIC,
  TAGORE_RESIZE_INTERPOLATION_LANCZOS,
} TagoreResizeInterpolation;

typedef struct
{
  gboolean enabled;
  double x;
  double y;
  double width;
  double height;
  gboolean preserve_aspect_ratio;
} TagoreCropState;

typedef struct
{
  gboolean enabled;
  int width;
  int height;
  gboolean maintain_aspect_ratio;
  TagoreResizeInterpolation interpolation;
} TagoreResizeState;

typedef struct
{
  double brightness;
  double contrast;
  double saturation;
  double exposure;
  double gamma;
} TagoreAdjustmentState;

typedef struct
{
  TagoreRotation rotation;
  gboolean flip_horizontal;
  gboolean flip_vertical;
  TagoreCropState crop;
  TagoreResizeState resize;
  TagoreAdjustmentState adjustments;
} TagoreEditState;

/**
 * tagore_edit_state_init:
 * @state: state to initialize.
 *
 * Initializes an edit state with identity, non-destructive parameters.
 */
void tagore_edit_state_init(TagoreEditState *state);

/**
 * tagore_edit_state_is_identity:
 * @state: state to inspect.
 *
 * Returns whether @state leaves the original image unchanged.
 */
gboolean tagore_edit_state_is_identity(const TagoreEditState *state);

/**
 * tagore_edit_state_rotate_left:
 * @state: state to mutate.
 *
 * Adds a non-destructive 90-degree counter-clockwise rotation.
 */
void tagore_edit_state_rotate_left(TagoreEditState *state);

/**
 * tagore_edit_state_rotate_right:
 * @state: state to mutate.
 *
 * Adds a non-destructive 90-degree clockwise rotation.
 */
void tagore_edit_state_rotate_right(TagoreEditState *state);

/**
 * tagore_edit_state_flip_horizontal:
 * @state: state to mutate.
 *
 * Toggles horizontal flip in the non-destructive state.
 */
void tagore_edit_state_flip_horizontal(TagoreEditState *state);

/**
 * tagore_edit_state_flip_vertical:
 * @state: state to mutate.
 *
 * Toggles vertical flip in the non-destructive state.
 */
void tagore_edit_state_flip_vertical(TagoreEditState *state);

G_END_DECLS
