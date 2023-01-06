#include "punk.h"
#include "punk_internal.h"

#include "assert.h"

struct label_state
{
  char caption[MAX_CAPTION_LENGTH];
};

void draw_label(const struct widget_state* w)
{
  struct label_state* state = w->state.label;

  // Render the text.
  struct text_and_surface* text_surface =
    find_string_surface(state->caption);
  if (text_surface == NULL)
  {
    text_surface = render_and_insert_text(state->caption);
  }

  render_text(text_surface->surf, &w->loc);
}

void punk_label(const char* caption)
{
  assert(g_punk_ctx->num_layouts > 0);
  struct layout_state* layout = &g_punk_ctx->layouts[g_punk_ctx->num_layouts - 1];

  // Check whether we've already got this widget cached.
  struct widget_state* w = find_widget(LABEL, &layout->current_child);
  if (w)
  {
    w->needs_to_be_rendered = 1;
  }
  else
  {
    w = &g_punk_ctx->widgets[g_punk_ctx->num_widgets++];
    w->type = LABEL;
    memcpy(&w->loc, &layout->current_child, sizeof(SDL_Rect));
    w->currently_active = 0;
    w->currently_rendered = 0;
    w->needs_to_be_rendered = 1;

    struct label_state* state =
      (struct label_state*)malloc(sizeof(struct label_state));
    strcpy(state->caption, caption);
    w->state.label = state;
    w->draw = &draw_label;
  }

  layout_step(layout);
}

