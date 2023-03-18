#include "punk.h"
#include "punk_internal.h"

struct textbox_state
{
  char* text;
  int selected;
};

void draw_textbox(const struct widget_state* w)
{
  SDL_Rect inner_rect;
  get_inner_rect(&w->loc, &inner_rect, WIDGET_BORDER);

  clear_rect(&w->loc);

  struct textbox_state* state = w->state.textbox;

  // Border around the textbox.
  fill_rect(&w->loc, 0x000000FF);

  SDL_Rect text_rect;
  get_inner_rect(&w->loc, &text_rect, WIDGET_BORDER);
  fill_rect(&text_rect, g_punk_ctx->back_colour);

  if (strlen(state->text) != 0)
  {
    // Render the text.
    SDL_Surface* text_surface = get_text_surface(state->text, &w->style);
    render_text(text_surface, &text_rect);
  }
}

// The text argument backs the actual text the widget shows.
void punk_textbox(char* text, struct punk_style* style)
{
  assert(g_punk_ctx->num_layouts > 0);
  struct layout_state* layout = &g_punk_ctx->layouts[g_punk_ctx->num_layouts - 1];

  // Check whether we've already got this widget cached.
  struct widget_state* w = find_widget(TEXTBOX, &layout->current_child);
  struct textbox_state* current_state;
  if (w)
  {
    w->needs_to_be_rendered = 1;
    w->currently_rendered = 1;

    current_state = w->state.textbox;

    // Check whether the style has changed.
    if (style != NULL && memcmp(style, &w->style, sizeof(struct punk_style)) != 0)
    {
      // Style has change so need to render.
      memcpy(&w->style, style, sizeof(struct punk_style));
      w->currently_rendered = 0;
    }
  }
  else
  {
    w = &g_punk_ctx->widgets[g_punk_ctx->num_widgets++];
    init_widget(w, TEXTBOX, &layout->current_child, style);

    current_state = (struct textbox_state*)malloc(sizeof(struct textbox_state));
    current_state->text = text;
    current_state->selected = 0;
    w->state.textbox = current_state;
    w->draw = &draw_textbox;

    w->currently_rendered = 0;
  }

  layout_step(layout);

  // TODO: If the text has changed then re-render.

  // Check whether the button has been clicked.
  SDL_MouseButtonEvent* click = &g_punk_ctx->click;
  if (click->type == 0) return;

  if (hit_test(&w->loc, click->x, click->y))
  {
    switch (click->button)
    {
      case SDL_BUTTON_LEFT:
        if (!current_state->selected)
        {
          w->currently_rendered = 0;
          current_state->selected = 1;
        }
        break;
      default:
        break;
    }
  }
  else
  {
    // Clicked off the textbox.
    if (current_state->selected)
    {
      w->currently_rendered = 0;
      current_state->selected = 0;
    }
  }
}
