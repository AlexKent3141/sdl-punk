#define SDL_MAIN_HANDLED
#include "SDL.h"

#include "punk.h"

#include "stdio.h"

#define WIDTH 640
#define HEIGHT 480

int checked = 0;
char buf[255] = {0};
struct punk_style img_style, btn_style, lbl_style;
int dialog_visible = 0;
int key_button_visible = 0;

void show_gui()
{
  punk_begin_horizontal_layout("1:1:1:1", PUNK_FILL, PUNK_FILL);

  punk_begin_vertical_layout("1:e20:e30:1:1:1", PUNK_FILL, PUNK_FILL);

  // Tweak the style for the image.
  punk_image("res/punk.png", &img_style);
  punk_checkbox("Checkmate", &checked, NULL);

  if (punk_textbox(buf, NULL))
  {
    printf("Text changed: %s\n", buf);
  }

  punk_label("It's over", &lbl_style);
  punk_label("9000!", &lbl_style);
  if (punk_image_button("res/punk.png", &img_style))
  {
    printf("Punk!\n");
    dialog_visible = 1;
  }

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
}

struct CheckValue
{
  const char* field_name;
  int checked;
};

struct CheckValue options[3] =
{
  {"Brachiosaurus", 1},
  {"Velociraptor" , 0},
  {"Aerodactyl"   , 0}
};

void show_dialog()
{
  punk_begin_vertical_layout("1:1:1:1:1", PUNK_FILL, PUNK_FILL);

  punk_label("What is your favourite dinosaur?", NULL);
  for (int i = 0; i < 3; i++)
  {
    const int initially_checked = options[i].checked;

    punk_checkbox(options[i].field_name, &options[i].checked, NULL);

    // I want these options to be mutually exclusive: if this one is checked
    // then uncheck the other options.
    if (options[i].checked)
    {
      options[(i + 1) % 3].checked = 0;
      options[(i + 2) % 3].checked = 0;
    }

    // Additionally, do not allow the user to uncheck the only checkbox that
    // is checked.
    if (initially_checked && !options[i].checked)
    {
      options[i].checked = 1;
    }
  }

  punk_begin_horizontal_layout("1:1", PUNK_FILL, PUNK_FILL);
  if (punk_button("OK", NULL)) dialog_visible = 0;
  if (punk_button("Cancel", NULL)) dialog_visible = 0;
  punk_end_layout();

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

  punk_default_style(&img_style);
  img_style.back_colour_rgba = 0x0000FFFF;

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

    if (dialog_visible)
    {
      show_dialog();
    }
    else
    {
      show_gui();
    }

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
