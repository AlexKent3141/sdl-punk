#ifndef __PUNK_H_INCLUDED__
#define __PUNK_H_INCLUDED__

#include "stdlib.h"

#ifdef WIN32
#define DLLEXPORT __declspec(dllexport)
#else
// TODO
#define DLLEXPORT
#endif

#define PUNK_FILL -1

// Forward declare the SDL types we depend on.
struct SDL_Renderer;
union SDL_Event;

struct punk_context;
extern struct punk_context* g_punk_ctx = NULL;

DLLEXPORT void punk_init(struct SDL_Renderer*, int, int);
DLLEXPORT void punk_quit();

DLLEXPORT void punk_handle_event(union SDL_Event*);

// These functions are used to indicate when the UI definition starts and ends.
DLLEXPORT void punk_begin();
DLLEXPORT void punk_end();

DLLEXPORT void punk_render();

DLLEXPORT void punk_begin_horizontal_layout(int n, int width, int height);
DLLEXPORT void punk_end_horizontal_layout();

DLLEXPORT int punk_button(const char* text);

#endif // __PUNK_H_INCLUDED__
