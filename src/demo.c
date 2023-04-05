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

  int key_button_visible = 0;
  int checked = 0;
  struct punk_style pb_style, btn_style, lbl_style;

  punk_default_style(&pb_style);
  pb_style.back_colour_rgba = 0x0000FFFF;

  punk_default_style(&btn_style);
  btn_style.font_size = 40;
  btn_style.text_colour_rgba = 0xFF0000FF;

  punk_default_style(&lbl_style);
  lbl_style.back_colour_rgba = 0x90EE90FF;

  SDL_bool stop = SDL_FALSE;
  SDL_Event event;
  while (!stop)
  {
    while (SDL_PollEvent(&event))
    {
      switch (event.type)
      {
        case SDL_KEYDOWN:
          key_button_visible = 1 - key_button_visible;
          break;
        case SDL_QUIT:
          stop = SDL_TRUE;
          break;
        default:
          break;
      }

      punk_handle_event(&event);
    }

    // Define the GUI here.
    punk_begin();

    punk_begin_horizontal_layout("1:1:1:1", PUNK_FILL, PUNK_FILL);

    punk_begin_vertical_layout("1:e20:1:1:1", PUNK_FILL, PUNK_FILL);

    // Tweak the style for the the picture box.
    punk_picturebox("res/punk.png", &pb_style);
    punk_checkbox("Checkmate", &checked, NULL);

    punk_label("It's over", &lbl_style);
    punk_label("9000!", &lbl_style);
    if (punk_picture_button("res/punk.png", &pb_style)) printf("Punk!\n");

    punk_end_layout(); // Vertical layout

    if (punk_button("hello", NULL)) printf("Hello world!\n");

    if (key_button_visible)
    {
      if (punk_button("maybe", NULL)) printf("Maybe!\n");
    }
    else
    {
      punk_skip_layout_widget();
    }

    if (checked)
    {
      punk_begin_vertical_layout("1:2:1", PUNK_FILL, PUNK_FILL);

      if (punk_button("V1", &btn_style) == PUNK_CLICK_RIGHT) printf("V1!\n");
      if (punk_button("V2", &btn_style)) printf("V2!\n");
      if (punk_button("V3", &btn_style)) printf("Punk!\n");

      punk_end_layout(); // Vertical layout
    }

    punk_end_layout(); // Horizontal layout

    punk_end();

    fflush(stdout);

    // Clear the screen and re-draw.
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);

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
