#ifndef __PUNK_H_INCLUDED__
#define __PUNK_H_INCLUDED__

#include "stdlib.h"

#define PUNK_FILL -1

// Forward declare the SDL types we depend on.
struct SDL_Renderer;
struct SDL_Event;

struct punk_context;
extern struct punk_context* g_punk_ctx = NULL;

void punk_init(struct SDL_Renderer*, int, int);
void punk_quit();

void punk_handle_event(struct SDL_Event*);

// These functions are used to indicate when the UI definition starts and ends.
void punk_begin();
void punk_end();

void punk_begin_horizontal_layout(int width, int height);
void punk_end_horizontal_layout();

int punk_button(const char* text);

#endif // __PUNK_H_INCLUDED__
