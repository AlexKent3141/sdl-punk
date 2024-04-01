#ifndef __PUNK_INTERNAL_H_INCLUDED__
#define __PUNK_INTERNAL_H_INCLUDED__

#include "SDL.h"
#include "SDL_ttf.h"

#include "assert.h"
#include "stdlib.h"
#include "string.h"

#define MAX_WIDGETS 100
#define MAX_WIDGETS_PER_LAYOUT 20
#define MAX_NESTED_LAYOUTS 10
#define WIDGET_BORDER 1
#define MAX_IMAGES_RENDERED 1000
#define MAX_CAPTION_LENGTH 50
#define MAX_PATH_LENGTH 255
#define TEXT_SIZE_PIXELS 20
#define MAX_TEXT_SIZE_PIXELS 100

enum widget_type
{
  BUTTON,
  LABEL,
  CHECKBOX,
  IMAGE,
  IMAGE_BUTTON,
  TEXTBOX
};

struct button_state;
struct label_state;
struct checkbox_state;
struct image_state;
struct image_button_state;
struct textbox_state;

struct widget_state
{
  // These fields are used to uniquely identify the widget between render passes.
  enum widget_type type;
  SDL_Rect loc;
  struct punk_style style;

  // Flags used to check whether we need to render now.
  int currently_active;
  int needs_to_be_active;
  int currently_rendered;
  int needs_to_be_rendered;

  // Optionally store the currently rendered txt.
  SDL_Surface* text;

  // Widget specific data.
  union
  {
    struct button_state* button;
    struct label_state* label;
    struct checkbox_state* checkbox;
    struct image_state* image;
    struct image_button_state* image_button;
    struct textbox_state* textbox;
    void* data;
  } state;

  void (*draw)(const struct widget_state*);
};

enum layout_type
{
  HORIZONTAL,
  VERTICAL
};

struct layout_state
{
  enum layout_type type;
  int current_child_index;
  SDL_Rect current_child;
  float current_x;
  float current_y;
  int width;
  int height;
  float widget_sizes[MAX_WIDGETS_PER_LAYOUT];
  int num_widgets;
};

struct text_and_surface
{
  char text[MAX_CAPTION_LENGTH];
  SDL_Surface* surf;
  struct punk_style style;
};

struct image_and_surface
{
  char img_path[MAX_PATH_LENGTH];
  SDL_Surface* surf;
};

struct punk_context
{
  // Window dimensions.
  int width;
  int height;

  // Renderer for the window we're targeting.
  SDL_Renderer* renderer;

  // Fonts.
  TTF_Font* fonts[MAX_TEXT_SIZE_PIXELS];

  // The texture we incrementally update as the UI changes state.
  // This is owned by punk.
  SDL_Texture* tex;
  uint32_t back_colour; // Background where there's no widget.

  // Maintain a cache of textures for each image and piece of text we've rendered.
  struct image_and_surface image_surfaces[MAX_IMAGES_RENDERED];
  int num_images_rendered;
  int next_image_index;

  // Keep track of all widgets we've encountered so far.
  struct widget_state widgets[MAX_WIDGETS];
  int num_widgets;
  int next_widget_index;

  // Event status.
  SDL_MouseMotionEvent motion;
  SDL_MouseButtonEvent click;
  SDL_KeyboardEvent key;
  SDL_TextInputEvent text;

  // Keep track of where we are in each nested layout.
  struct layout_state layouts[MAX_NESTED_LAYOUTS];
  int num_layouts;
};

void fill_rect(const SDL_Rect*, uint32_t);
void clear_rect(const SDL_Rect*);
void render_text(SDL_Surface*, const SDL_Rect*);
void render_image(SDL_Surface*, const SDL_Rect*);
void get_inner_rect(const SDL_Rect*, SDL_Rect*, int);
void text_size(const char*, const struct punk_style*, int*, int*);

SDL_Surface* create_text_surface(const char*, const struct punk_style*);
SDL_Surface* get_image_surface(const char*);

void layout_step(struct layout_state*);

int hit_test(const SDL_Rect*, int32_t, int32_t);

void init_widget(
  struct widget_state*, enum widget_type, const SDL_Rect*, const struct punk_style*);
struct widget_state* find_widget(enum widget_type, const SDL_Rect*);

struct widget_state* next_widget_slot();

#endif // __PUNK_INTERNAL_H_INCLUDED__
