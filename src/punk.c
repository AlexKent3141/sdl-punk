#include "punk.h"

#include "SDL.h"

#include "assert.h"
#include "stdint.h"
#include "stdlib.h"

#define MAX_WIDGETS 100
#define MAX_NESTED_LAYOUTS 10

enum layout_type
{
  NONE,
  HORIZONTAL
};

enum widget_type
{
  NONE,
  BUTTON
};

struct point
{
  int x;
  int y;
};

struct widget_state
{
  // These fields are used to uniquely identify the widget between render passes.
  enum widget_type type;
  struct point loc;

  // Flags used to check whether we need to render now.
  int currently_rendered;
  int needs_to_be_rendered;
};

struct layout_state
{
  enum layout_type type;
  struct point current_offset;
  int width;
  int height;
};

struct punk_context
{
  // Window dimensions.
  int width;
  int height;

  // Renderer for the window we're targeting.
  SDL_Renderer* renderer;

  // The texture we incrementally update as the UI changes state.
  // This is owned by punk.
  SDL_Texture* tex;
  uint32_t background_col;
  uint32_t foreground_col;

  // Keep track of all widgets we've encountered so far.
  struct widget_state widgets[MAX_WIDGETS];
  int num_widgets;

  // Keep track of where we are in each nested layout.
  struct layout_state layouts[MAX_NESTED_LAYOUTS];
  int num_layouts;
};

void fill_rect(SDL_Rect* rect, uint32_t col)
{
  SDL_Surface* surface;
  SDL_LockTextureToSurface(g_punk_ctx->tex, NULL, &surface);
  SDL_FillRect(surface, rect, col);
  SDL_UnlockSurface(g_punk_ctx->tex);
}

void clear_rect(SDL_Rect* rect)
{
  fill_rect(rect, g_punk_ctx->background_col);
}

void punk_init(SDL_Renderer* renderer, int width, int height)
{
  if (g_punk_ctx != NULL) return;

  g_punk_ctx = (struct punk_context*)malloc(sizeof(struct punk_context));
  g_punk_ctx->width = width;
  g_punk_ctx->height = height;
  g_punk_ctx->renderer = renderer;

  g_punk_ctx->tex = SDL_CreateTexture(
    renderer,
    SDL_PIXELFORMAT_RGBA8888,
    SDL_TEXTUREACCESS_STREAMING,
    width,
    height);

  SDL_SetTextureBlendMode(g_punk_ctx->tex, SDL_BLENDMODE_BLEND);

  // Make the texture fully transparent and cache the transparent colour.
  SDL_Surface* surface;
  SDL_LockTextureToSurface(g_punk_ctx->tex, NULL, &surface);
  g_punk_ctx->background_col = SDL_MapRGBA(surface->format, 255, 0, 0, 0);
  g_punk_ctx->background_col = SDL_MapRGBA(surface->format, 100, 100, 100, 255);
  SDL_FillRect(surface, NULL, g_punk_ctx->background_col);
  SDL_UnlockSurface(g_punk_ctx->tex);

  memset(g_punk_ctx->widgets, 0, MAX_WIDGETS * sizeof(struct widget_state));
  g_punk_ctk->num_widgets = 0;

  memset(g_punk_ctx->layouts, 0, MAX_NESTED_LAYOUTS * sizeof(struct layout_state));
  g_punk_ctx->num_layouts = 0;
}

void punk_quit()
{
  if (g_punk_ctx == NULL) return;

  SDL_DestroyTexture(g_punk_ctx->tex);
  free(g_punk_ctx);
  g_punk_ctx = NULL;
}

void punk_handle_event(SDL_Event* e)
{
  // TODO: need to watch out for relevant mouse clicks etc.
}

// These functions are used to indicate when the UI definition starts and ends.
void punk_begin()
{
  // Assume nothing's going to change this time around.
  struct widget_state* w;
  for (int i = 0; i < g_punk_ctx->num_widgets; i++)
  {
    w = &g_punk_ctx->widgets[i];
    w->currently_rendered = w->needs_to_be_rendered;
    w->needs_to_be_rendered = 0;
  }
}

void punk_end()
{
  // TODO: for now just draw all widgets with width 60 pixels.
  // TODO: only supporting horizontal layout, so height is inherited from that.
  struct layout_state* layout = &g_punk_ctx->layouts[0];

  // Resolve updates to the texture and render.
  const struct widget_state* w;
  for (int i = 0; i < g_punk_ctx->num_widgets; i++)
  {
    w = &g_punk_ctx->widgets[i];

    SDL_Rect rect;
    rect.x = w->loc.x;
    rect.y = w->loc.y;
    rect.w = 60;
    rect.h = layout->height;

    if (w->currently_rendered && !w->needs_to_be_rendered)
    {
      // Clear the relevant part of the texture.
      clear_rect(&rect);
    }
    else if (!w->currently_rendered && w->needs_to_be_rendered)
    {
      // Draw the widget on the texture.
      fill_rect(&rect, g_punk_ctx->foreground_col);
    }
  }
}

void punk_begin_horizontal_layout(int width, int height)
{
  int layout_index = g_punk_ctx->num_layouts;

  struct layout_state* layout = &g_punk_ctx->layouts[layout_index];

  layout->type = HORIZONTAL;
  layout->width = width;
  layout->height = height;

  // The current offset for this layout is inherited from the outer layout.
  if (layout_index > 0)
  {
    struct layout_state* prev_layout = &g_punk_ctx->layouts[layout_index - 1];
    memcpy(&layout->current_offset, &prev_layout->current_offset, sizeof(struct point));
  }
  else
  {
    layout->current_offset.x = 0;
    layout->current_offset.y = 0;
  }

  ++g_punk_ctx->num_layouts;
}

void punk_end_horizontal_layout()
{
  --g_punk_ctx->num_layouts;
}

struct widget_state* find_widget(enum widget_type type, const struct point* loc)
{
  for (int i = 0; i < g_punk_ctx->num_widgets; i++)
  {
    struct widget_state* w = &g_punk_ctx->widgets[i];
    if (w->type == type && memcmp(&w->loc, loc) == 0)
    {
      return w;
    }
  }

  return NULL;
}

int punk_button(const char* text)
{
  // Where will this button be rendered? This is inherited from the current layout position.
  assert(g_punk_ctx->num_layouts > 0);
  struct layout_state* layout = &g_punk_ctx->layouts[g_punk_ctx->num_layouts - 1];

  // Check whether we've already got this widget cached.
  struct widget_state* w = find_widget(BUTTON, &layout->current_offset);
  if (w)
  {
    w->needs_to_be_rendered = 1;
  }
  else
  {
    w = &g_punk_ctx->widgets[g_punk_ctx->num_widgets++];
    w->type = BUTTON;
    memcpy(&w->loc, &layout->current_offset, sizeof(struct point));
    w->currently_rendered = 0;
    w->needs_to_be_rendered = 1;
  }

  // TODO: Assuming all widgets have width 60 for now.

  // Increment the current offset in the (horizontal) layout.
  layout->current_offset.x += 65;

  // TODO: need to perform click detection.
  return 0;
}
