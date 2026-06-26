#include "tagore/tagore-edit-state.h"

#include <glib.h>

static void
test_edit_state_defaults_to_identity(void)
{
  TagoreEditState state;

  tagore_edit_state_init(&state);

  g_assert_true(tagore_edit_state_is_identity(&state));
  g_assert_cmpint(state.rotation, ==, TAGORE_ROTATION_0);
  g_assert_false(state.flip_horizontal);
  g_assert_false(state.flip_vertical);
  g_assert_false(state.crop.enabled);
  g_assert_false(state.resize.enabled);
  g_assert_cmpfloat(state.adjustments.contrast, ==, 1.0);
  g_assert_cmpfloat(state.adjustments.saturation, ==, 1.0);
  g_assert_cmpfloat(state.adjustments.gamma, ==, 1.0);
}

static void
test_edit_state_rotates_without_touching_pixels(void)
{
  TagoreEditState state;

  tagore_edit_state_init(&state);

  tagore_edit_state_rotate_right(&state);
  g_assert_cmpint(state.rotation, ==, TAGORE_ROTATION_90);
  g_assert_false(tagore_edit_state_is_identity(&state));

  tagore_edit_state_rotate_right(&state);
  g_assert_cmpint(state.rotation, ==, TAGORE_ROTATION_180);

  tagore_edit_state_rotate_left(&state);
  g_assert_cmpint(state.rotation, ==, TAGORE_ROTATION_90);

  tagore_edit_state_rotate_left(&state);
  g_assert_cmpint(state.rotation, ==, TAGORE_ROTATION_0);
  g_assert_true(tagore_edit_state_is_identity(&state));
}

static void
test_edit_state_flips_toggle(void)
{
  TagoreEditState state;

  tagore_edit_state_init(&state);

  tagore_edit_state_flip_horizontal(&state);
  g_assert_true(state.flip_horizontal);
  g_assert_false(tagore_edit_state_is_identity(&state));

  tagore_edit_state_flip_horizontal(&state);
  g_assert_false(state.flip_horizontal);
  g_assert_true(tagore_edit_state_is_identity(&state));

  tagore_edit_state_flip_vertical(&state);
  g_assert_true(state.flip_vertical);
  g_assert_false(tagore_edit_state_is_identity(&state));
}

int
main(int argc, char *argv[])
{
  g_test_init(&argc, &argv, NULL);

  g_test_add_func("/tagore/edit-state/defaults-to-identity", test_edit_state_defaults_to_identity);
  g_test_add_func("/tagore/edit-state/rotates-without-touching-pixels", test_edit_state_rotates_without_touching_pixels);
  g_test_add_func("/tagore/edit-state/flips-toggle", test_edit_state_flips_toggle);

  return g_test_run();
}
