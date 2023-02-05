#ifndef __PUNK_H_INCLUDED__
#define __PUNK_H_INCLUDED__

#include "stdlib.h"
#include "stdint.h"

#ifdef WIN32
#define DLLEXPORT __declspec(dllexport)
#else
// TODO
#define DLLEXPORT
#endif

#define PUNK_FILL -1

// Forward declare the SDL types we depend on.
struct SDL_Renderer;
struct SDL_Rect;
union SDL_Event;

struct punk_context;
extern struct punk_context* g_punk_ctx;

DLLEXPORT int punk_init(struct SDL_Renderer*, int, int);
DLLEXPORT void punk_quit();

DLLEXPORT void punk_handle_event(union SDL_Event*);

// These functions are used to indicate when the UI definition starts and ends.
DLLEXPORT void punk_begin();
DLLEXPORT void punk_end();

DLLEXPORT void punk_render();

/*
 Layout definitions contain a `split` string which indicates how the layout area is
 divided between the contained widgets.
 Elements in the split string can have a couple of different forms. The split
 might be entirely ratio-based e.g. "1:2:1" indicates a layout containing 3 widgets
 with the middle one being twice as large as the outer two. Alternatively, some
 of the widgets could have fixed sizes e.g. "1:1:1:e50" indicates a layout containing
 4 widgets where the last one is exactly 50 pixels in size and the first 3 are all
 the same size (filling the area of the layout).
*/
DLLEXPORT void punk_begin_horizontal_layout(const char* split, int width, int height);
DLLEXPORT void punk_begin_vertical_layout(const char* split, int width, int height);
DLLEXPORT void punk_end_layout();

// Use this function to leave an empty area inside a layout rather than populating
// every slot with a widget.
DLLEXPORT void punk_skip_layout_widget();

// Get the rect for the current widget.
// This is useful for debugging, but could also be used to decorate an existing
// widget.
// The return value is non-zero in the case of an error (perhaps there is no layout
// defined?).
DLLEXPORT int punk_current_rect(struct SDL_Rect*);

/*
 Style can be customised on a per-widget basis.
 It's recommended that you use the below function to get the default style and
 then modify this and pass it into your widget function calls.
 If the style argument to a widget call is NULL then the default style is used.
*/

struct punk_style
{
  int font_size;
  uint32_t text_colour_rgba;
};

DLLEXPORT void punk_default_style(struct punk_style*);

// Functions to display widgets.
enum punk_click_type
{
  PUNK_CLICK_NONE,
  PUNK_CLICK_LEFT,
  PUNK_CLICK_RIGHT
};

DLLEXPORT enum punk_click_type punk_button(const char* text, struct punk_style*);
DLLEXPORT void punk_label(const char* text, struct punk_style*);
DLLEXPORT void punk_checkbox(const char* text, int* checked, struct punk_style*);

#endif // __PUNK_H_INCLUDED__
