#include "punk.h"
#include "punk_internal.h"

#include "assert.h"

struct button_state
{
  char caption[MAX_CAPTION_LENGTH];
};

void draw_button(const struct widget_state* w)
{
  SDL_Rect inner_rect;
  get_inner_rect(&w->loc, &inner_rect, WIDGET_BORDER);

  struct button_state* state = w->state.button;

  fill_rect(&w->loc, g_punk_ctx->back_colour);
  uint32_t col = w->needs_to_be_active
    ? g_punk_ctx->active_colour : g_punk_ctx->fore_colour;
  fill_rect(&inner_rect, col);

  // Render the text.
  SDL_Surface* text_surface = get_text_surface(state->caption);
  render_text(text_surface, &w->loc);
}

int punk_button(const char* caption)
{
  assert(g_punk_ctx->num_layouts > 0);
  struct layout_state* layout = &g_punk_ctx->layouts[g_punk_ctx->num_layouts - 1];

  // Check whether we've already got this widget cached.
  struct widget_state* w = find_widget(BUTTON, &layout->current_child);
  if (w)
  {
    w->needs_to_be_rendered = 1;
  }
  else
  {
    w = &g_punk_ctx->widgets[g_punk_ctx->num_widgets++];
    w->type = BUTTON;
    memcpy(&w->loc, &layout->current_child, sizeof(SDL_Rect));
    w->currently_active = 0;
    w->currently_rendered = 0;
    w->needs_to_be_rendered = 1;

    struct button_state* state =
      (struct button_state*)malloc(sizeof(struct button_state));
    strcpy(state->caption, caption);
    w->state.button = state;
    w->draw = &draw_button;
  }

  layout_step(layout);

  // Check the next active state of the widget.
  SDL_MouseMotionEvent* motion = &g_punk_ctx->motion;
  w->needs_to_be_active = motion->type != 0 && hit_test(&w->loc, motion->x, motion->y);

  // Check whether the button has been clicked.
  SDL_MouseButtonEvent* click = &g_punk_ctx->click;
  if (click->type == 0) return 0;

  return hit_test(&w->loc, click->x, click->y);
}
