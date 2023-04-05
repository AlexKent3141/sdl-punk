#include "punk.h"
#include "punk_internal.h"

struct label_state
{
  char caption[MAX_CAPTION_LENGTH];
};

void draw_label(const struct widget_state* w)
{
  SDL_Rect inner_rect;
  get_inner_rect(&w->loc, &inner_rect, WIDGET_BORDER);

  const struct punk_style* style = &w->style;
  fill_rect(&inner_rect, style->back_colour_rgba);

  struct label_state* state = w->state.label;

  // Render the text.
  SDL_Surface* text_surface = get_text_surface(state->caption, &w->style);
  render_text(text_surface, &w->loc);
}

void punk_label(const char* caption, const struct punk_style* style)
{
  assert(g_punk_ctx->num_layouts > 0);
  struct layout_state* layout = &g_punk_ctx->layouts[g_punk_ctx->num_layouts - 1];

  // Check whether we've already got this widget cached.
  struct widget_state* w = find_widget(LABEL, &layout->current_child);
  if (w)
  {
    w->needs_to_be_rendered = 1;

    // If the caption has changed then force a re-draw.
    struct label_state* current_state = w->state.label;
    if (strcmp(current_state->caption, caption) != 0)
    {
      strcpy(current_state->caption, caption);
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
    w = &g_punk_ctx->widgets[g_punk_ctx->num_widgets++];
    init_widget(w, LABEL, &layout->current_child, style);

    struct label_state* state =
      (struct label_state*)malloc(sizeof(struct label_state));
    strcpy(state->caption, caption);
    w->state.label = state;
    w->draw = &draw_label;
  }

  layout_step(layout);
}

