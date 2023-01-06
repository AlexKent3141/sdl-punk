#include "punk.h"
#include "punk_internal.h"

#include "assert.h"

struct checkbox_state
{
  char caption[MAX_CAPTION_LENGTH];
  SDL_Rect text_area;
  SDL_Rect box_area;
  int checked;
};

void draw_checkbox(const struct widget_state* w)
{
  assert(w->type == CHECKBOX);

  struct checkbox_state* state = w->state.checkbox;

  // Render the text.
  SDL_Surface* text_surface = get_text_surface(state->caption);
  render_text(text_surface, &state->text_area);

  // The check box needs to have a border and some indication of
  // when it's focused.
  fill_rect(&state->box_area, g_punk_ctx->fore_colour);

  if (w->needs_to_be_active)
  {
    SDL_Rect active_rect;
    get_inner_rect(&state->box_area, &active_rect, WIDGET_BORDER);

    fill_rect(&active_rect, g_punk_ctx->active_colour);
  }

  if (state->checked)
  {
    SDL_Rect checked_rect;
    get_inner_rect(&state->box_area, &checked_rect, 3*WIDGET_BORDER);

    fill_rect(&checked_rect, 0x000000FF);//g_punk_ctx->text_colour);
  }
}

void punk_checkbox(const char* caption, int* checked)
{
  assert(g_punk_ctx->num_layouts > 0);
  assert(checked != NULL);
  struct layout_state* layout = &g_punk_ctx->layouts[g_punk_ctx->num_layouts - 1];

  // Check whether we've already got this widget cached.
  struct widget_state* w = find_widget(CHECKBOX, &layout->current_child);
  if (w)
  {
    w->needs_to_be_rendered = 1;
  }
  else
  {
    w = &g_punk_ctx->widgets[g_punk_ctx->num_widgets++];
    w->type = CHECKBOX;
    memcpy(&w->loc, &layout->current_child, sizeof(SDL_Rect));
    w->currently_active = 0;
    w->currently_rendered = 0;
    w->needs_to_be_rendered = 1;

    struct checkbox_state* state =
      (struct checkbox_state*)malloc(sizeof(struct checkbox_state));
    strcpy(state->caption, caption);
    memcpy(&state->text_area, &layout->current_child, sizeof(SDL_Rect));
    state->text_area.w -= TEXT_SIZE_PIXELS;
    memcpy(&state->box_area, &layout->current_child, sizeof(SDL_Rect));
    state->box_area.w = TEXT_SIZE_PIXELS;
    state->box_area.x += state->text_area.w;
    state->box_area.h = TEXT_SIZE_PIXELS;
    state->box_area.y = 0.5 * (state->text_area.h - TEXT_SIZE_PIXELS);
    w->state.checkbox = state;
    w->draw = &draw_checkbox;
  }

  layout_step(layout);
  struct checkbox_state* state = w->state.checkbox;
  state->checked = *checked;

  // Check the next active state of the widget.
  SDL_MouseMotionEvent* motion = &g_punk_ctx->motion;
  w->needs_to_be_active =
    motion->type != 0 && hit_test(&state->box_area, motion->x, motion->y);

  // Check whether the button has been clicked.
  SDL_MouseButtonEvent* click = &g_punk_ctx->click;
  if (click->type == 0 || click->button != SDL_BUTTON_LEFT) return;

  if (hit_test(&state->box_area, click->x, click->y))
  {
    *checked = !*checked;
    state->checked = *checked;
    w->currently_active = 0;
  }
}
