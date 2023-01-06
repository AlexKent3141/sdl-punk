#ifndef __PUNK_INTERNAL_H_INCLUDED__
#define __PUNK_INTERNAL_H_INCLUDED__

#include "SDL.h"
#include "SDL_ttf.h"

#define MAX_WIDGETS 100
#define MAX_NESTED_LAYOUTS 10
#define WIDGET_BORDER 1
#define MAX_STRINGS_RENDERED 100
#define MAX_CAPTION_LENGTH 50
#define TEXT_SIZE_PIXELS 20

enum widget_type
{
  BUTTON,
  LABEL,
  CHECKBOX
};

struct button_state;
struct label_state;
struct checkbox_state;

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
  union
  {
    struct button_state* button;
    struct label_state* label;
    struct checkbox_state* checkbox;
    void* data;
  } state;

  void (*draw)(const struct widget_state*);
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

void fill_rect(const SDL_Rect*, uint32_t);
void clear_rect(const SDL_Rect*);
void render_text(SDL_Surface*, const SDL_Rect*);
void get_inner_rect(const SDL_Rect*, SDL_Rect*, int);

SDL_Surface* get_text_surface(const char*);

int hit_test(const SDL_Rect*, int32_t, int32_t);

struct widget_state* find_widget(enum widget_type, const SDL_Rect*);

#endif // __PUNK_INTERNAL_H_INCLUDED__
