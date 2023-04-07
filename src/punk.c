#include "punk.h"
#include "punk_internal.h"
#include "SDL_image.h"

#include "font.h"

#include "stdint.h"

struct punk_context* g_punk_ctx = NULL;

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
  // Note: we've created the text with the right size - just need to position it.
  SDL_Rect temp;
  temp.h = text->h;
  temp.w = text->w;
  temp.x = rect->x + 0.5f * (rect->w - temp.w);
  temp.y = rect->y + 0.5f * (rect->h - temp.h);

  SDL_BlitSurface(text, NULL, target, &temp);

  SDL_UnlockTexture(g_punk_ctx->tex);
}

void render_image(SDL_Surface* image, const SDL_Rect* rect)
{
  SDL_Surface* target;
  SDL_LockTextureToSurface(g_punk_ctx->tex, NULL, &target);

  // Calculate the target rect.
  // We want the image to be centred in the target and we want to keep the aspect ratio.
  const double image_ratio = (double)image->w / image->h;
  const double target_ratio = (double)rect->w / rect->h;

  SDL_Rect temp;
  if (image_ratio < target_ratio)
  {
    temp.h = rect->h;
    temp.w = temp.h * image_ratio;
  }
  else
  {
    temp.w = rect->w;
    temp.h = temp.w / image_ratio;
  }

  temp.x = rect->x + 0.5f * (rect->w - temp.w);
  temp.y = rect->y + 0.5f * (rect->h - temp.h);

  SDL_BlitScaled(image, NULL, target, &temp);

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

  memset(&g_punk_ctx->fonts, 0, MAX_TEXT_SIZE_PIXELS*sizeof(TTF_Font*));

  // Create the texture on which the UI will be rendered.
  g_punk_ctx->tex = SDL_CreateTexture(
    renderer,
    SDL_PIXELFORMAT_RGBA8888,
    SDL_TEXTUREACCESS_STREAMING,
    width,
    height);

  SDL_SetTextureBlendMode(g_punk_ctx->tex, SDL_BLENDMODE_BLEND);

  g_punk_ctx->num_strings_rendered = 0;
  g_punk_ctx->num_images_rendered = 0;

  // Make the texture fully transparent and cache the transparent colour.
  SDL_Surface* surface;
  SDL_LockTextureToSurface(g_punk_ctx->tex, NULL, &surface);
  g_punk_ctx->back_colour = SDL_MapRGBA(surface->format, 0, 0, 0, 0);
  SDL_FillRect(surface, NULL, g_punk_ctx->back_colour);
  SDL_UnlockTexture(g_punk_ctx->tex);

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

  for (int i = 0; i < MAX_TEXT_SIZE_PIXELS; i++)
  {
    if (g_punk_ctx->fonts[i] != NULL)
    {
      TTF_CloseFont(g_punk_ctx->fonts[i]);
    }
  }

  TTF_Quit();

  // Clean up widget states.
  for (int i = 0; i < g_punk_ctx->num_widgets; i++)
  {
    struct widget_state* w = &g_punk_ctx->widgets[i];
    free(w->state.data);
  }

  // Clean up cached text surfaces.
  for (int i = 0; i < g_punk_ctx->num_strings_rendered; i++)
  {
    struct text_and_surface* tt = &g_punk_ctx->text_surfaces[i];
    SDL_FreeSurface(tt->surf);
  }

  // Clean up cached image surfaces.
  for (int i = 0; i < g_punk_ctx->num_images_rendered; i++)
  {
    struct image_and_surface* is = &g_punk_ctx->image_surfaces[i];
    SDL_FreeSurface(is->surf);
  }

  // Clean up main texture.
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
      memcpy(&g_punk_ctx->click, &e->button, sizeof(SDL_MouseButtonEvent));
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

SDL_Surface* get_text_surface(const char* text, const struct punk_style* style)
{
  // Check the cache.
  for (int i = 0; i < g_punk_ctx->num_strings_rendered; i++)
  {
    struct text_and_surface* tt = &g_punk_ctx->text_surfaces[i];
    if (strcmp(tt->text, text) == 0 &&
        memcmp(&tt->style, style, sizeof(struct punk_style)) == 0)
    {
      return tt->surf;
    }
  }

  // Potentially need to create the font object.
  TTF_Font* font = g_punk_ctx->fonts[style->font_size];
  if (font == NULL)
  {
    SDL_RWops* font_data =
      SDL_RWFromConstMem(Hack_Regular_ttf, Hack_Regular_ttf_len);
    g_punk_ctx->fonts[style->font_size] =
      TTF_OpenFontRW(font_data, 0, style->font_size);
    font = g_punk_ctx->fonts[style->font_size];
  }

  uint32_t rgba = style->text_colour_rgba;
  SDL_Color col =
  {
    rgba >> 24,
    (rgba >> 16) & 0xFF,
    (rgba >> 8) & 0xFF,
    rgba & 0xFF
  };

  SDL_Surface* surf = TTF_RenderText_Blended(font, text, col);

  int index = g_punk_ctx->num_strings_rendered++;
  struct text_and_surface* text_surface = &g_punk_ctx->text_surfaces[index];
  strcpy(text_surface->text, text);
  text_surface->surf = surf;
  memcpy(&text_surface->style, style, sizeof(struct punk_style));

  return surf;
}

SDL_Surface* get_image_surface(const char* img_path)
{
  // Check the cache.
  for (int i = 0; i < g_punk_ctx->num_images_rendered; i++)
  {
    struct image_and_surface* is = &g_punk_ctx->image_surfaces[i];
    if (strcmp(is->img_path, img_path) == 0)
    {
      return is->surf;
    }
  }

  // Need to load the image from scratch.
  SDL_Surface* surf = IMG_Load(img_path);

  int index = g_punk_ctx->num_images_rendered++;
  struct image_and_surface* image_surface = &g_punk_ctx->image_surfaces[index];
  strcpy(image_surface->img_path, img_path);
  image_surface->surf = surf;

  return surf;
}

void get_inner_rect(const SDL_Rect* initial, SDL_Rect* inner, int border)
{
  memcpy(inner, initial, sizeof(SDL_Rect));
  inner->x += border;
  inner->y += border;
  inner->w -= 2*border;
  inner->h -= 2*border;
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
      w->draw(w);
    }
  }

  // Invalidate cached click event.
  g_punk_ctx->click.type = 0;
}

void punk_render()
{
  SDL_RenderCopy(g_punk_ctx->renderer, g_punk_ctx->tex, NULL, NULL);
}

void calculate_sizes(const char* split, struct layout_state* layout, int max_size)
{
  // Parse the split string to calculate child widths.
  static char tokens[MAX_WIDGETS_PER_LAYOUT][10];
  static char split_mutable[255];
  int exact_pixels_used = 0;
  int ratio_units_used = 0;
  strcpy(split_mutable, split);
  char* token = strtok(split_mutable, ":");
  while (token != NULL)
  {
    strcpy(tokens[layout->num_widgets], token);
    if (token[0] == 'e') exact_pixels_used += atoi(&token[1]);
    else ratio_units_used += atoi(token);
    ++layout->num_widgets;
    token = strtok(NULL, ":");
  }

  float ratio_unit_size = 0;
  if (ratio_units_used > 0)
  {
    ratio_unit_size = (float)(max_size - exact_pixels_used) / ratio_units_used;
  }

  for (int i = 0; i < layout->num_widgets; i++)
  {
    if (tokens[i][0] == 'e')
      layout->widget_sizes[i] = atoi(&tokens[i][1]);
    else
      layout->widget_sizes[i] = atoi(tokens[i]) * ratio_unit_size;
  }
}

void punk_begin_horizontal_layout(const char* split, int width, int height)
{
  assert(split != NULL);
  assert(strlen(split) > 0);

  int layout_index = g_punk_ctx->num_layouts;

  struct layout_state* layout = &g_punk_ctx->layouts[layout_index];

  layout->type = HORIZONTAL;
  layout->current_child_index = 0;

  // The current offset for this layout is inherited from the outer layout.
  if (layout_index > 0)
  {
    struct layout_state* prev_layout = &g_punk_ctx->layouts[layout_index - 1];
    layout->width = width == PUNK_FILL ? prev_layout->current_child.w : width;
    layout->height = height == PUNK_FILL ? prev_layout->current_child.h : height;

    layout->current_child.x = prev_layout->current_child.x;
    layout->current_child.y = prev_layout->current_child.y;
    layout->current_child.h = layout->height;
  }
  else
  {
    // The root layout fills the whole area (unless otherwise specified).
    layout->width = width == PUNK_FILL ? g_punk_ctx->width : width;
    layout->height = height == PUNK_FILL ? g_punk_ctx->height : height;

    layout->current_child.x = 0;
    layout->current_child.y = 0;
    layout->current_child.h = layout->height;
  }

  layout->num_widgets = 0;

  calculate_sizes(split, layout, layout->width);

  layout->current_x = layout->current_child.x;
  layout->current_y = layout->current_child.y;
  layout->current_child.w = layout->widget_sizes[0];

  ++g_punk_ctx->num_layouts;
}

void punk_begin_vertical_layout(const char* split, int width, int height)
{
  int layout_index = g_punk_ctx->num_layouts;

  struct layout_state* layout = &g_punk_ctx->layouts[layout_index];

  layout->type = VERTICAL;
  layout->current_child_index = 0;

  // The current offset for this layout is inherited from the outer layout.
  if (layout_index > 0)
  {
    struct layout_state* prev_layout = &g_punk_ctx->layouts[layout_index - 1];
    layout->width = width == PUNK_FILL ? prev_layout->current_child.w : width;
    layout->height = height == PUNK_FILL ? prev_layout->current_child.h : height;

    layout->current_child.x = prev_layout->current_child.x;
    layout->current_child.y = prev_layout->current_child.y;
    layout->current_child.w = layout->width;
  }
  else
  {
    // The root layout fills the whole area (unless otherwise specified).
    layout->width = width == PUNK_FILL ? g_punk_ctx->width : width;
    layout->height = height == PUNK_FILL ? g_punk_ctx->height : height;

    layout->current_child.x = 0;
    layout->current_child.y = 0;
    layout->current_child.w = layout->width;
  }

  layout->num_widgets = 0;

  calculate_sizes(split, layout, layout->height);

  layout->current_x = layout->current_child.x;
  layout->current_y = layout->current_child.y;
  layout->current_child.h = layout->widget_sizes[0];

  ++g_punk_ctx->num_layouts;
}

void layout_step(struct layout_state* layout)
{
  switch (layout->type)
  {
    case HORIZONTAL:
      layout->current_x += layout->widget_sizes[layout->current_child_index++];
      layout->current_child.w = layout->widget_sizes[layout->current_child_index];
      break;
    case VERTICAL:
      layout->current_y += layout->widget_sizes[layout->current_child_index++];
      layout->current_child.h = layout->widget_sizes[layout->current_child_index];
      break;
    default:
      assert(0);
      break;
  }

  layout->current_child.x = layout->current_x;
  layout->current_child.y = layout->current_y;
}

void punk_skip_layout_widget()
{
  int layout_index = g_punk_ctx->num_layouts;
  assert(layout_index > 0);

  struct layout_state* top_layout = &g_punk_ctx->layouts[layout_index - 1];
  layout_step(top_layout);
}

void punk_end_layout()
{
  --g_punk_ctx->num_layouts;

  if (g_punk_ctx->num_layouts == 0) return;

  struct layout_state* layout = &g_punk_ctx->layouts[g_punk_ctx->num_layouts - 1];
  layout_step(layout);
}

int punk_current_rect(struct SDL_Rect* rect)
{
  if (rect == NULL)
  {
    return 1;
  }

  if (g_punk_ctx->num_layouts == 0)
  {
    // Cannot get a widget rect with no layout.
    return 2;
  }

  struct layout_state* layout = &g_punk_ctx->layouts[g_punk_ctx->num_layouts - 1];
  memcpy(rect, &layout->current_child, sizeof(struct SDL_Rect));
  return 0;
}

void punk_default_style(struct punk_style* style)
{
  style->font_size = TEXT_SIZE_PIXELS;
  style->text_colour_rgba = 0x000000FF;
  style->back_colour_rgba = 0xFFFFFFFF;
  style->control_colour_rgba = 0x969696FF;
  style->active_colour_rgba = 0xAAAAAAFF;
}

void init_widget(
  struct widget_state* w,
  enum widget_type type,
  const SDL_Rect* loc,
  const struct punk_style* style)
{
  w->type = type;
  memcpy(&w->loc, loc, sizeof(SDL_Rect));

  if (style)
  {
    memcpy(&w->style, style, sizeof(struct punk_style));
  }
  else
  {
    punk_default_style(&w->style);
  }

  w->currently_active = 0;
  w->currently_rendered = 0;
  w->needs_to_be_rendered = 1;
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
