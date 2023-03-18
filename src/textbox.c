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

  // Border around the textbox.
  fill_rect(&w->loc, 0x000000FF);

  SDL_Rect text_rect;
  get_inner_rect(&w->loc, &text_rect, WIDGET_BORDER);
  fill_rect(&text_rect, g_punk_ctx->back_colour);

  const struct textbox_state* state = w->state.textbox;
  if (strlen(state->text) != 0)
  {
    // Render the text.
    render_text(w->text, &text_rect);
  }
}

// The text argument backs the actual text the widget shows.
int punk_textbox(char* text, struct punk_style* style)
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
      // Style has changed so need to render.
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

  // Check whether the textbox has been selected.
  SDL_MouseButtonEvent* click = &g_punk_ctx->click;
  if (click->type == 0) goto no_click;

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

no_click: ;

  SDL_TextInputEvent* text_input = &g_punk_ctx->text;
  if (text_input->type != 0 && current_state->selected)
  {
    int len = strlen(text);
    memcpy(&text[len], text_input->text, strlen(text_input->text));
    text[len + strlen(text_input->text) + 1] = '\0';

    // Check whether the new text will fit.
    int width, height;
    text_size(text, &w->style, &width, &height);
    if (width >= w->loc.w)
    {
      text[len + 1] = '\0';
    }
    else
    {
      if (w->text) SDL_FreeSurface(w->text);
      w->text = create_text_surface(text, &w->style);
      w->currently_rendered = 0;
      return 1;
    }
  }

  SDL_KeyboardEvent* key = &g_punk_ctx->key;
  if (key->type == SDL_KEYDOWN && current_state->selected)
  {
    if (key->keysym.sym == SDLK_BACKSPACE)
    {
      int len = strlen(text);
      if (len > 0)
      {
        text[len - 1] = '\0';
        if (w->text) SDL_FreeSurface(w->text);
        w->text = create_text_surface(text, &w->style);
        w->currently_rendered = 0;
        return 1;
      }
    }
  }

  return 0;
}
