#define SDL_MAIN_HANDLED
#include "SDL.h"

#include "punk.h"

#include "stdio.h"

#define WIDTH 640
#define HEIGHT 480

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
    }

    // Define the GUI here.
    punk_begin();

    punk_begin_horizontal_layout(2, PUNK_FILL, 50);

    if (punk_button("hello"))
    {
      printf("Hello world!\n");
    }

    punk_end_horizontal_layout();

    punk_end();

    // Clear the screen and re-draw.
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);

    // Draw the updated UI state.
    punk_render();

    SDL_RenderPresent(renderer);

    SDL_Delay(100);
  }

  punk_quit();

  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
