#include "punk.h"
#include "punk_internal.h"

struct image_state
{
  char img_path[MAX_PATH_LENGTH];
};

void draw_image(const struct widget_state* w)
{
  clear_rect(&w->loc);

  fill_rect(&w->loc, w->style.back_colour_rgba);

  struct image_state* state = w->state.image;

  // Render the image.
  SDL_Surface* image_surface = get_image_surface(state->img_path);
  render_image(image_surface, &w->loc);
}

void punk_image(const char* img_path, const struct punk_style* style)
{
  assert(g_punk_ctx->num_layouts > 0);
  struct layout_state* layout = &g_punk_ctx->layouts[g_punk_ctx->num_layouts - 1];

  // Check whether we've already got this widget cached.
  struct widget_state* w = find_widget(IMAGE, &layout->current_child);
  if (w)
  {
    w->needs_to_be_rendered = 1;

    // If the caption has changed then force a re-draw.
    struct image_state* current_state = w->state.image;
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
    init_widget(w, IMAGE, &layout->current_child, style);

    struct image_state* state =
      (struct image_state*)malloc(sizeof(struct image_state));
    strcpy(state->img_path, img_path);
    w->state.image = state;
    w->draw = &draw_image;
  }

  layout_step(layout);
}

