#include "punk.h"
#include "punk_internal.h"

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

  clear_rect(&w->loc);

  struct checkbox_state* state = w->state.checkbox;

  // Render the text.
  render_text(w->text, &state->text_area);

  // The check box needs to have a border and some indication of
  // when it's focused.
  const struct punk_style* style = &w->style;
  fill_rect(&state->box_area, style->control_colour_rgba);

  if (w->needs_to_be_active)
  {
    SDL_Rect active_rect;
    get_inner_rect(&state->box_area, &active_rect, WIDGET_BORDER);

    fill_rect(&active_rect, style->active_colour_rgba);
  }

  if (state->checked)
  {
    SDL_Rect checked_rect;
    get_inner_rect(&state->box_area, &checked_rect, 3*WIDGET_BORDER);

    fill_rect(&checked_rect, 0x000000FF);
  }
}

void punk_checkbox(const char* caption, int* checked, const struct punk_style* style)
{
  assert(g_punk_ctx->num_layouts > 0);
  assert(checked != NULL);
  struct layout_state* layout = &g_punk_ctx->layouts[g_punk_ctx->num_layouts - 1];

  // Check whether we've already got this widget cached.
  struct widget_state* w = find_widget(CHECKBOX, &layout->current_child);
  if (w)
  {
    w->needs_to_be_rendered = 1;

    // If the state has changed then force a re-draw.
    struct checkbox_state* current_state = w->state.checkbox;
    if (strcmp(current_state->caption, caption) != 0 ||
        current_state->checked != *checked)
    {
      strcpy(current_state->caption, caption);
      current_state->checked = *checked;
      SDL_FreeSurface(w->text);
      w->text = create_text_surface(caption, &w->style);
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
    init_widget(w, CHECKBOX, &layout->current_child, style);

    struct checkbox_state* state =
      (struct checkbox_state*)malloc(sizeof(struct checkbox_state));
    strcpy(state->caption, caption);
    memcpy(&state->text_area, &layout->current_child, sizeof(SDL_Rect));
    state->text_area.w -= TEXT_SIZE_PIXELS;
    memcpy(&state->box_area, &layout->current_child, sizeof(SDL_Rect));
    state->box_area.w = TEXT_SIZE_PIXELS - 2;
    state->box_area.x += state->text_area.w + 1;
    state->box_area.h = TEXT_SIZE_PIXELS - 2;
    state->box_area.y += 1;
    w->state.checkbox = state;
    w->text = create_text_surface(caption, &w->style);
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
    w->currently_rendered = 0;
  }
}
