/* nuklear - 1.32.0 - public domain */
#include <assert.h>
#include <limits.h>
#include <math.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_SDL_GL3_IMPLEMENTATION
#include "nuklear.h"
#include "nuklear_sdl_gl3.h"

#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 800

#define MAX_VERTEX_MEMORY 512 * 1024
#define MAX_ELEMENT_MEMORY 128 * 1024

/* ===============================================================
 *
 *                          EXAMPLE
 *
 * ===============================================================*/
/* This are some code examples to provide a small overview of what can be
 * done with this library. To try out an example uncomment the defines */
/*#define INCLUDE_ALL */
/*#define INCLUDE_STYLE */
/*#define INCLUDE_CALCULATOR */
/*#define INCLUDE_CANVAS */
/*#define INCLUDE_OVERVIEW */
/*#define INCLUDE_NODE_EDITOR */

#ifdef INCLUDE_ALL
#define INCLUDE_STYLE
#define INCLUDE_CALCULATOR
#define INCLUDE_CANVAS
#define INCLUDE_OVERVIEW
#define INCLUDE_NODE_EDITOR
#endif

#ifdef INCLUDE_STYLE
#include "../../demo/common/style.c"
#endif
#ifdef INCLUDE_CALCULATOR
#include "../../demo/common/calculator.c"
#endif
#ifdef INCLUDE_CANVAS
#include "../../demo/common/canvas.c"
#endif
#ifdef INCLUDE_OVERVIEW
#include "../../demo/common/overview.c"
#endif
#ifdef INCLUDE_NODE_EDITOR
#include "../../demo/common/node_editor.c"
#endif

void message_widget(struct nk_context *ctx, char *user_name, char *message) {
  nk_label(ctx, user_name, NK_TEXT_LEFT);
  nk_label(ctx, message, NK_TEXT_LEFT);
}

void copy_string(char *dest, int dest_len, char *src, int src_len) {
  int len = dest_len > src_len ? src_len : dest_len;
  for (int i = 0; i < len; ++i) {
    dest[i] = src[i];
  }
}

/* ===============================================================
 *
 *                          DEMO
 *
 * ===============================================================*/
int main(int argc, char *argv[]) {
  /* Platform */
  SDL_Window *win;
  SDL_GLContext glContext;
  int win_width, win_height;
  int running = 1;

  /* GUI */
  struct nk_context *ctx;
  struct nk_colorf bg;

  NK_UNUSED(argc);
  NK_UNUSED(argv);

  /* SDL setup */
  SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "0");
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS,
                      SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  win = SDL_CreateWindow("Skibidi Chat", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                         WINDOW_WIDTH, WINDOW_HEIGHT,
                         SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN |
                             SDL_WINDOW_ALLOW_HIGHDPI);
  glContext = SDL_GL_CreateContext(win);
  SDL_GetWindowSize(win, &win_width, &win_height);

  /* OpenGL setup */
  glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
  glewExperimental = 1;
  if (glewInit() != GLEW_OK) {
    fprintf(stderr, "Failed to setup GLEW\n");
    exit(1);
  }

  ctx = nk_sdl_init(win);
  /* Load Fonts: if none of these are loaded a default font will be used  */
  /* Load Cursor: if you uncomment cursor loading please hide the cursor */
  {
    struct nk_font_atlas *atlas;
    nk_sdl_font_stash_begin(&atlas);
    /*struct nk_font *droid = nk_font_atlas_add_from_file(atlas,
     * "../../../extra_font/DroidSans.ttf", 14, 0);*/
    /*struct nk_font *roboto = nk_font_atlas_add_from_file(atlas,
     * "../../../extra_font/Roboto-Regular.ttf", 16, 0);*/
    /*struct nk_font *future = nk_font_atlas_add_from_file(atlas,
     * "../../../extra_font/kenvector_future_thin.ttf", 13, 0);*/
    /*struct nk_font *clean = nk_font_atlas_add_from_file(atlas,
     * "../../../extra_font/ProggyClean.ttf", 12, 0);*/
    /*struct nk_font *tiny = nk_font_atlas_add_from_file(atlas,
     * "../../../extra_font/ProggyTiny.ttf", 10, 0);*/
    /*struct nk_font *cousine = nk_font_atlas_add_from_file(atlas,
     * "../../../extra_font/Cousine-Regular.ttf", 13, 0);*/
    nk_sdl_font_stash_end();
  /*nk_style_load_all_cursors(ctx, atlas->cursors);*/
    /*nk_style_set_font(ctx, &roboto->handle);*/}

    int current_user = 0;

    int first_scroll = 1;

    bg.r = 0.10f, bg.g = 0.18f, bg.b = 0.24f, bg.a = 1.0f;
    while (running) {
      /* Input */
      SDL_Event evt;
      nk_input_begin(ctx);
      while (SDL_PollEvent(&evt)) {
        if (evt.type == SDL_QUIT)
          goto cleanup;
        nk_sdl_handle_event(&evt);
      }
      nk_sdl_handle_grab(); /* optional grabbing behavior */
      nk_input_end(ctx);

      char users[10][20];
      for (int i = 0; i < 10; ++i) {
        snprintf(users[i], 20, "skbd_user#%d", i + 1);
      }

      char user_messages[10][40];
      for (int i = 0; i < 10; ++i) {
        snprintf(user_messages[i], 40, "%s msg#%d", users[i], i + 1);
      }

      char current_message[40];
      int actual_length;
      
      char approved_message[40];

      /* GUI */
      if (nk_begin(ctx, "skibidi main window",
                   nk_rect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT),
                   NK_WINDOW_BORDER)) {
        float ratio[] = {0.1f, 0.35, 0.55f};
        nk_layout_row(ctx, NK_DYNAMIC, 20 * 30, 3, ratio);
        if (nk_group_begin(ctx, "groups", 0)) {
           
          nk_group_end(ctx);
        }
        if (nk_group_begin(ctx, "users", 0)) {
          nk_layout_row_dynamic(ctx, 30, 1);
          for (int i = 0; i < 5; ++i) {
            if (nk_button_label(ctx, users[i])) {
              current_user = i;
              first_scroll = 1;
            }
          }
          nk_text(ctx, current_message, actual_length, NK_TEXT_LEFT);
          nk_group_end(ctx);
        }
        if (nk_group_begin(ctx, "messages panel", NK_WINDOW_NO_SCROLLBAR)) {
          nk_layout_row_dynamic(ctx, 50, 1);
          if (nk_group_begin(ctx, "user header", NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR)) {
            nk_layout_row_dynamic(ctx, 50, 1);
            nk_label(ctx, users[current_user], NK_TEXT_LEFT);
            nk_group_end(ctx);
          }

          nk_layout_row_dynamic(ctx, 13 * 30, 1);
          if (nk_group_begin(ctx, "messages", 0)) {
            if (first_scroll) {
              nk_group_set_scroll(ctx, "messages", 0, 13 * 30);
              first_scroll = 0;
            }
            nk_layout_row_dynamic(ctx, 30, 1);
            for (int i = 0; i < 10; ++i) {
              message_widget(ctx, users[current_user],
                             user_messages[current_user]);
            }
            nk_group_end(ctx);
          }

          if (nk_group_begin(ctx, "message_field", 0)) {
            float ratio[] = {0.9f, 0.1f};
            nk_layout_row(ctx, NK_DYNAMIC, 30, 2, ratio);
            nk_edit_string(ctx, NK_EDIT_FIELD, current_message,
                           &actual_length, 40, nk_filter_default);
            if (nk_button_label(ctx, "Send")) {
              copy_string(approved_message, 40, current_message, 40);
              memset(current_message, 0, 40);
              actual_length = 0;
            }
            nk_group_end(ctx);
          }
          nk_group_end(ctx);
        }

        enum { EASY, HARD };
        static int op = EASY;
        static int property = 20;

        nk_layout_row_static(ctx, 30, 80, 1);
        if (nk_button_label(ctx, "button"))
          printf("button pressed!\n");
        nk_layout_row_dynamic(ctx, 30, 2);
        if (nk_option_label(ctx, "easy", op == EASY))
          op = EASY;
        if (nk_option_label(ctx, "hard", op == HARD))
          op = HARD;
        nk_layout_row_dynamic(ctx, 22, 1);
        nk_property_int(ctx, "Compression:", 0, &property, 100, 10, 1);

        nk_layout_row_dynamic(ctx, 20, 1);
        nk_label(ctx, "background:", NK_TEXT_LEFT);
        nk_layout_row_dynamic(ctx, 25, 1);
        if (nk_combo_begin_color(ctx, nk_rgb_cf(bg),
                                 nk_vec2(nk_widget_width(ctx), 400))) {
          nk_layout_row_dynamic(ctx, 120, 1);
          bg = nk_color_picker(ctx, bg, NK_RGBA);
          nk_layout_row_dynamic(ctx, 25, 1);
          bg.r = nk_propertyf(ctx, "#R:", 0, bg.r, 1.0f, 0.01f, 0.005f);
          bg.g = nk_propertyf(ctx, "#G:", 0, bg.g, 1.0f, 0.01f, 0.005f);
          bg.b = nk_propertyf(ctx, "#B:", 0, bg.b, 1.0f, 0.01f, 0.005f);
          bg.a = nk_propertyf(ctx, "#A:", 0, bg.a, 1.0f, 0.01f, 0.005f);
          nk_combo_end(ctx);
        }
      }
      nk_end(ctx);

/* -------------- EXAMPLES ---------------- */
#ifdef INCLUDE_CALCULATOR
    calculator(ctx);
#endif
#ifdef INCLUDE_CANVAS
    canvas(ctx);
#endif
#ifdef INCLUDE_OVERVIEW
    overview(ctx);
#endif
#ifdef INCLUDE_NODE_EDITOR
    node_editor(ctx);
#endif
    /* ----------------------------------------- */

    /* Draw */
    SDL_GetWindowSize(win, &win_width, &win_height);
    glViewport(0, 0, win_width, win_height);
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(bg.r, bg.g, bg.b, bg.a);
    /* IMPORTANT: `nk_sdl_render` modifies some global OpenGL state
     * with blending, scissor, face culling, depth test and viewport and
     * defaults everything back into a default state.
     * Make sure to either a.) save and restore or b.) reset your own state
     * after rendering the UI. */
    nk_sdl_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_MEMORY, MAX_ELEMENT_MEMORY);
    SDL_GL_SwapWindow(win);
  }

cleanup:
  nk_sdl_shutdown();
  SDL_GL_DeleteContext(glContext);
  SDL_DestroyWindow(win);
  SDL_Quit();
  return 0;
}
