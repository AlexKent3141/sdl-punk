#include "punk.h"
#include "punk_internal.h"

struct image_button_state
{
  char img_path[MAX_PATH_LENGTH];
};

void draw_image_button(const struct widget_state* w)
{
  SDL_Rect inner_rect;
  get_inner_rect(&w->loc, &inner_rect, WIDGET_BORDER);

  clear_rect(&w->loc);

  struct image_button_state* state = w->state.image_button;

  const struct punk_style* style = &w->style;

  uint32_t col = w->needs_to_be_active
    ? style->active_colour_rgba : style->control_colour_rgba;
  fill_rect(&inner_rect, col);

  // Render the image.
  SDL_Surface* image_surface = get_image_surface(state->img_path);
  render_image(image_surface, &w->loc);
}

enum punk_click_type punk_image_button(const char* img_path, const struct punk_style* style)
{
  assert(g_punk_ctx->num_layouts > 0);
  struct layout_state* layout = &g_punk_ctx->layouts[g_punk_ctx->num_layouts - 1];

  // Check whether we've already got this widget cached.
  struct widget_state* w = find_widget(IMAGE_BUTTON, &layout->current_child);
  if (w)
  {
    w->needs_to_be_rendered = 1;

    // If the caption has changed then force a re-draw.
    struct image_button_state* current_state = w->state.image_button;
    if (strcmp(current_state->img_path, img_path) != 0)
    {
      strcpy(current_state->img_path, img_path);
      w->currently_rendered = 0;
    }

    // Check whether the style has changed.
    if (style != NULL && memcmp(style, &w->style, sizeof(struct punk_style)) != 0)
    {
      memcpy(&w->style, style, sizeof(struct punk_style));
      w->currently_rendered = 0;
    }
  }
  else
  {
    w = next_widget_slot();
    init_widget(w, IMAGE_BUTTON, &layout->current_child, style);

    struct image_button_state* state =
      (struct image_button_state*)malloc(sizeof(struct image_button_state));
    strcpy(state->img_path, img_path);
    w->state.image_button = state;
    w->draw = &draw_image_button;
  }

  layout_step(layout);

  // Check the next active state of the widget.
  SDL_MouseMotionEvent* motion = &g_punk_ctx->motion;
  w->needs_to_be_active = motion->type != 0 && hit_test(&w->loc, motion->x, motion->y);

  // Check whether the button has been clicked.
  SDL_MouseButtonEvent* click = &g_punk_ctx->click;
  if (click->type == 0) return 0;

  if (hit_test(&w->loc, click->x, click->y))
  {
    switch (click->button)
    {
      case SDL_BUTTON_LEFT:
        return PUNK_CLICK_LEFT;
      case SDL_BUTTON_RIGHT:
        return PUNK_CLICK_RIGHT;
      default:
        return PUNK_CLICK_NONE;
    }
  }

  return PUNK_CLICK_NONE;
}
