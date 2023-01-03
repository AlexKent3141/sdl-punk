#include "punk.h"

#include "font.h"

#include "SDL.h"
#include "SDL_ttf.h"

#include "assert.h"
#include "stdint.h"
#include "stdlib.h"

#define MAX_WIDGETS 100
#define MAX_NESTED_LAYOUTS 10
#define WIDGET_BORDER 1
#define MAX_STRINGS_RENDERED 100
#define MAX_CAPTION_LENGTH 50

struct punk_context* g_punk_ctx = NULL;

enum layout_type
{
  HORIZONTAL,
  VERTICAL
};

enum widget_type
{
  BUTTON,
  LABEL
};

struct widget_state
{
  // These fields are used to uniquely identify the widget between render passes.
  enum widget_type type;
  SDL_Rect loc;

  // Flags used to check whether we need to render now.
  int currently_active;
  int needs_to_be_active;
  int currently_rendered;
  int needs_to_be_rendered;

  // Widget specific data.
  // TODO: we'll probably need to do something a bit cleverer here when more
  // widget types get implemented.
  char caption[MAX_CAPTION_LENGTH];
};

struct layout_state
{
  enum layout_type type;
  SDL_Rect current_child;
  int width;
  int height;
  int num_items;
};

struct text_and_surface
{
  const char* text;
  SDL_Surface* surf;
};

struct punk_context
{
  // Window dimensions.
  int width;
  int height;

  // Renderer for the window we're targeting.
  SDL_Renderer* renderer;

  // Font.
  TTF_Font* font;

  // The texture we incrementally update as the UI changes state.
  // This is owned by punk.
  SDL_Texture* tex;
  uint32_t back_colour;
  uint32_t fore_colour;
  uint32_t active_colour; // Selected widget
  SDL_Color text_colour;

  // Maintain a cache of textures for each piece of text we've rendered.
  struct text_and_surface text_surfaces[MAX_STRINGS_RENDERED];
  int num_strings_rendered;

  // Keep track of all widgets we've encountered so far.
  struct widget_state widgets[MAX_WIDGETS];
  int num_widgets;

  // Event status.
  SDL_MouseMotionEvent motion;
  SDL_MouseButtonEvent click;

  // Keep track of where we are in each nested layout.
  struct layout_state layouts[MAX_NESTED_LAYOUTS];
  int num_layouts;
};

void fill_rect(const SDL_Rect* rect, uint32_t col)
{
  SDL_Surface* surface;
  SDL_LockTextureToSurface(g_punk_ctx->tex, NULL, &surface);
  SDL_FillRect(surface, rect, col);
  SDL_UnlockTexture(g_punk_ctx->tex);
}

void clear_rect(const SDL_Rect* rect)
{
  fill_rect(rect, g_punk_ctx->back_colour);
}

void render_text(SDL_Surface* text, const SDL_Rect* rect)
{
  SDL_Surface* target;
  SDL_LockTextureToSurface(g_punk_ctx->tex, NULL, &target);

  // Calculate the target rect.
  // We aim to fill the middle third of the target.
  SDL_Rect temp;

  // Initially assume we're going to be limited by height.
  temp.y = rect->y + 0.1f * rect->h;
  temp.h = 0.8f * rect->h;

  // What width do we need based on the ratio?
  temp.w = temp.h * (float)(text->w) / text->h;
  temp.x = (rect->x + 0.5f * (rect->w - temp.w));

  if (temp.w > rect->w)
  {
    temp.x = rect->x + 0.1f * rect->w;
    temp.w = 0.8f * rect->w;

    // What height do we need based on the ratio?
    temp.h = temp.w * (float)(text->h) / text->w;
    temp.y = (rect->y + 0.5f * (rect->h - temp.h));
  }

  SDL_BlitScaled(text, NULL, target, &temp);

  SDL_UnlockTexture(g_punk_ctx->tex);
}

int punk_init(SDL_Renderer* renderer, int width, int height)
{
  if (g_punk_ctx != NULL) return -1;

  g_punk_ctx = (struct punk_context*)malloc(sizeof(struct punk_context));
  g_punk_ctx->width = width;
  g_punk_ctx->height = height;
  g_punk_ctx->renderer = renderer;

  // Initialise TTF and load the embedded font.
  if (TTF_Init() != 0) return -1;

  SDL_RWops* font_data = SDL_RWFromConstMem(Hack_Regular_ttf, Hack_Regular_ttf_len);
  g_punk_ctx->font = TTF_OpenFontRW(font_data, 0, 100);
  if (g_punk_ctx->font == NULL) return -1;

  // Create the texture on which the UI will be rendered.
  g_punk_ctx->tex = SDL_CreateTexture(
    renderer,
    SDL_PIXELFORMAT_RGBA8888,
    SDL_TEXTUREACCESS_STREAMING,
    width,
    height);

  SDL_SetTextureBlendMode(g_punk_ctx->tex, SDL_BLENDMODE_BLEND);

  g_punk_ctx->num_strings_rendered = 0;

  // Make the texture fully transparent and cache the transparent colour.
  SDL_Surface* surface;
  SDL_LockTextureToSurface(g_punk_ctx->tex, NULL, &surface);
  g_punk_ctx->back_colour = SDL_MapRGBA(surface->format, 255, 0, 0, 0);
  g_punk_ctx->fore_colour = SDL_MapRGBA(surface->format, 150, 150, 150, 255);
  g_punk_ctx->active_colour = SDL_MapRGBA(surface->format, 170, 170, 170, 255);
  SDL_FillRect(surface, NULL, g_punk_ctx->back_colour);
  SDL_UnlockTexture(g_punk_ctx->tex);

  g_punk_ctx->text_colour.r = 0;
  g_punk_ctx->text_colour.g = 0;
  g_punk_ctx->text_colour.b = 0;
  g_punk_ctx->text_colour.a = 255;

  memset(g_punk_ctx->widgets, 0, MAX_WIDGETS * sizeof(struct widget_state));
  g_punk_ctx->num_widgets = 0;

  memset(g_punk_ctx->layouts, 0, MAX_NESTED_LAYOUTS * sizeof(struct layout_state));
  g_punk_ctx->num_layouts = 0;

  // Invalidate the cached events.
  g_punk_ctx->motion.type = 0;
  g_punk_ctx->click.type = 0;

  return 0;
}

void punk_quit()
{
  if (g_punk_ctx == NULL) return;

  if (g_punk_ctx->font != NULL)
  {
    TTF_CloseFont(g_punk_ctx->font);
  }

  TTF_Quit();

  for (int i = 0; i < g_punk_ctx->num_strings_rendered; i++)
  {
    struct text_and_surface* tt = &g_punk_ctx->text_surfaces[i];
    SDL_FreeSurface(tt->surf);
  }

  SDL_DestroyTexture(g_punk_ctx->tex);
  free(g_punk_ctx);
  g_punk_ctx = NULL;
}

void punk_handle_event(SDL_Event* e)
{
  switch (e->type)
  {
    case SDL_MOUSEBUTTONDOWN:
    {
      if (e->button.button == SDL_BUTTON_LEFT)
      {
        memcpy(&g_punk_ctx->click, &e->button, sizeof(SDL_MouseButtonEvent));
      }
      break;
    }
    case SDL_MOUSEMOTION:
    {
      memcpy(&g_punk_ctx->motion, &e->motion, sizeof(SDL_MouseMotionEvent));
      break;
    }
  }
}

int hit_test(const SDL_Rect* r, int32_t x, int32_t y)
{
  return x > r->x && x < r->x + r->w && y > r->y && y < r->y + r->h;
}

// These functions are used to indicate when the UI definition starts and ends.
void punk_begin()
{
  // Assume nothing's going to change this time around.
  struct widget_state* w;
  for (int i = 0; i < g_punk_ctx->num_widgets; i++)
  {
    w = &g_punk_ctx->widgets[i];
    w->currently_active = w->needs_to_be_active;
    w->needs_to_be_active = 0;
    w->currently_rendered = w->needs_to_be_rendered;
    w->needs_to_be_rendered = 0;
  }
}

struct text_and_surface* find_string_surface(const char* text)
{
  for (int i = 0; i < g_punk_ctx->num_strings_rendered; i++)
  {
    struct text_and_surface* tt = &g_punk_ctx->text_surfaces[i];
    if (strcmp(tt->text, text) == 0)
    {
      return tt;
    }
  }

  return NULL;
}

struct text_and_surface* render_and_insert_text(const char* text)
{
  SDL_Surface* surf = TTF_RenderText_Blended(
    g_punk_ctx->font,
    text,
    g_punk_ctx->text_colour);

  int index = g_punk_ctx->num_strings_rendered++;
  struct text_and_surface* text_surface = &g_punk_ctx->text_surfaces[index];
  text_surface->text = text;
  text_surface->surf = surf;

  return text_surface;
}

void punk_end()
{
  // Resolve updates to the texture and render.
  // Done in two passes: clearing widgets and then drawing new widgets.
  const struct widget_state* w;
  for (int i = 0; i < g_punk_ctx->num_widgets; i++)
  {
    w = &g_punk_ctx->widgets[i];
    if (w->currently_rendered && !w->needs_to_be_rendered)
    {
      // Clear the relevant part of the texture.
      clear_rect(&w->loc);
    }
  }

  for (int i = 0; i < g_punk_ctx->num_widgets; i++)
  {
    w = &g_punk_ctx->widgets[i];
    int active_changed = w->currently_active != w->needs_to_be_active;
    int becomes_visible = !w->currently_rendered && w->needs_to_be_rendered;
    if (becomes_visible || (w->needs_to_be_rendered && active_changed))
    {
      // Draw the widget on the texture.
      SDL_Rect inner_rect;
      memcpy(&inner_rect, &w->loc, sizeof(SDL_Rect));
      inner_rect.x += WIDGET_BORDER;
      inner_rect.y += WIDGET_BORDER;
      inner_rect.w -= 2*WIDGET_BORDER;
      inner_rect.h -= 2*WIDGET_BORDER;
      switch (w->type)
      {
        case BUTTON:
        {
          fill_rect(&w->loc, g_punk_ctx->back_colour);
          uint32_t col = w->needs_to_be_active
            ? g_punk_ctx->active_colour : g_punk_ctx->fore_colour;
          fill_rect(&inner_rect, col);

          // Render the text.
          struct text_and_surface* text_surface = find_string_surface(w->caption);
          if (text_surface == NULL)
          {
            text_surface = render_and_insert_text(w->caption);
          }

          render_text(text_surface->surf, &w->loc);
          break;
        }
        case LABEL:
        {
          // Render the text.
          struct text_and_surface* text_surface = find_string_surface(w->caption);
          if (text_surface == NULL)
          {
            text_surface = render_and_insert_text(w->caption);
          }

          render_text(text_surface->surf, &w->loc);
          break;
        }
        default:
          assert(0);
          break;
      }
    }
  }

  // Invalidate cached click event.
  g_punk_ctx->click.type = 0;
}

void punk_render()
{
  SDL_RenderCopy(g_punk_ctx->renderer, g_punk_ctx->tex, NULL, NULL);
}

void punk_begin_horizontal_layout(int n, int width, int height)
{
  int layout_index = g_punk_ctx->num_layouts;

  struct layout_state* layout = &g_punk_ctx->layouts[layout_index];

  layout->type = HORIZONTAL;
  layout->num_items = n;

  // The current offset for this layout is inherited from the outer layout.
  if (layout_index > 0)
  {
    struct layout_state* prev_layout = &g_punk_ctx->layouts[layout_index - 1];
    layout->width = width == PUNK_FILL ? prev_layout->current_child.w : width;
    layout->height = height == PUNK_FILL ? prev_layout->current_child.h : height;

    layout->current_child.x = prev_layout->current_child.x;
    layout->current_child.y = prev_layout->current_child.y;
    layout->current_child.w = layout->width / n;
    layout->current_child.h = layout->height;
  }
  else
  {
    // The root layout fills the whole area (unless otherwise specified).
    layout->width = width == PUNK_FILL ? g_punk_ctx->width : width;
    layout->height = height == PUNK_FILL ? g_punk_ctx->height : height;

    layout->current_child.x = 0;
    layout->current_child.y = 0;
    layout->current_child.w = layout->width / n;
    layout->current_child.h = layout->height;
  }

  ++g_punk_ctx->num_layouts;
}

void punk_begin_vertical_layout(int n, int width, int height)
{
  int layout_index = g_punk_ctx->num_layouts;

  struct layout_state* layout = &g_punk_ctx->layouts[layout_index];

  layout->type = VERTICAL;
  layout->num_items = n;

  // The current offset for this layout is inherited from the outer layout.
  if (layout_index > 0)
  {
    struct layout_state* prev_layout = &g_punk_ctx->layouts[layout_index - 1];
    layout->width = width == PUNK_FILL ? prev_layout->current_child.w : width;
    layout->height = height == PUNK_FILL ? prev_layout->current_child.h : height;

    layout->current_child.x = prev_layout->current_child.x;
    layout->current_child.y = prev_layout->current_child.y;
    layout->current_child.w = layout->width;
    layout->current_child.h = layout->height / n;
  }
  else
  {
    // The root layout fills the whole area (unless otherwise specified).
    layout->width = width == PUNK_FILL ? g_punk_ctx->width : width;
    layout->height = height == PUNK_FILL ? g_punk_ctx->height : height;

    layout->current_child.x = 0;
    layout->current_child.y = 0;
    layout->current_child.w = layout->width;
    layout->current_child.h = layout->height / n;
  }

  ++g_punk_ctx->num_layouts;
}

void layout_step(struct layout_state* layout)
{
  switch (layout->type)
  {
    case HORIZONTAL:
      layout->current_child.x += layout->current_child.w;
      break;
    case VERTICAL:
      layout->current_child.y += layout->current_child.h;
      break;
    default:
      assert(0);
      break;
  }
}

void punk_end_layout()
{
  --g_punk_ctx->num_layouts;

  if (g_punk_ctx->num_layouts == 0) return;

  struct layout_state* layout = &g_punk_ctx->layouts[g_punk_ctx->num_layouts - 1];
  layout_step(layout);
}

struct widget_state* find_widget(enum widget_type type, const SDL_Rect* loc)
{
  for (int i = 0; i < g_punk_ctx->num_widgets; i++)
  {
    struct widget_state* w = &g_punk_ctx->widgets[i];
    if (w->type == type && memcmp(&w->loc, loc, sizeof(SDL_Rect)) == 0)
    {
      return w;
    }
  }

  return NULL;
}

int punk_button(const char* caption)
{
  // Where will this button be rendered? This is inherited from the current layout position.
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
    strcpy(w->caption, caption);
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

void punk_label(const char* caption)
{
  // Where will this button be rendered? This is inherited from the current layout position.
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
    strcpy(w->caption, caption);
  }

  layout_step(layout);
}
