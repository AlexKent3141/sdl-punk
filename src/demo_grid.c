#define SDL_MAIN_HANDLED
#include "SDL.h"

#include "punk.h"

#include "stdio.h"

#define WIDTH 640
#define HEIGHT 480

void draw_box(SDL_Renderer* renderer, SDL_Rect* r)
{
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderDrawLine(renderer, r->x, r->y, r->x, r->y + r->h);
  SDL_RenderDrawLine(renderer, r->x, r->y, r->x + r->w, r->y);
  SDL_RenderDrawLine(renderer, r->x + r->w, r->y, r->x + r->w, r->y + r->h);
  SDL_RenderDrawLine(renderer, r->x, r->y + r->h, r->x + r->w, r->y + r->h);
}

void draw_grid(SDL_Renderer* renderer)
{
  SDL_Rect rect;
  punk_begin_vertical_layout("1:1:1:1:1:1:1:1:1", PUNK_FILL, PUNK_FILL);
  for (int r = 0; r < 9; r++)
  {
    punk_begin_horizontal_layout("1:1:1:1:1:1:1:1:1", PUNK_FILL, PUNK_FILL);
    for (int c = 0; c < 9; c++)
    {
      punk_current_rect(&rect);

      char buf[2];
      buf[0] = '0' + c;
      buf[1] = '\0';
      punk_label(buf, NULL);

      // Draw box outlining the widget.
      draw_box(renderer, &rect);
    }
    punk_end_layout();
  }

  punk_end_layout();
}

int main()
{
  int ret = SDL_Init(SDL_INIT_VIDEO);
  if (ret != 0)
  {
    printf("Failed to initialise SDL: %d\n", ret);
    return EXIT_FAILURE;
  }

  SDL_Window* window = SDL_CreateWindow(
    "Punk demo",
    SDL_WINDOWPOS_UNDEFINED,
    SDL_WINDOWPOS_UNDEFINED,
    WIDTH,
    HEIGHT,
    0);

  SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

  // Setup punk.
  punk_init(renderer, WIDTH, HEIGHT);

  SDL_bool stop = SDL_FALSE;
  SDL_Event event;
  while (!stop)
  {
    while (SDL_PollEvent(&event))
    {
      switch (event.type)
      {
        case SDL_QUIT:
          stop = SDL_TRUE;
          break;
        default:
          break;
      }

      punk_handle_event(&event);
    }

    // Clear the screen and re-draw.
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);

    // Define the GUI here.
    punk_begin();

    draw_grid(renderer);

    punk_end();

    fflush(stdout);

    // Draw the updated UI state.
    punk_render();

    SDL_RenderPresent(renderer);

    SDL_Delay(100);
  }

  punk_quit();

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
