/* nuklear - 1.32.0 - public domain */
#include <assert.h>
#include <limits.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
#define possible_users_to_display WINDOW_HEIGHT / 30

#define MAX_VERTEX_MEMORY 512 * 1024
#define MAX_ELEMENT_MEMORY 128 * 1024

#define MESSAGE_LENGTH 256
#define USER_NAME_LENGTH 32
#define PASSWORD_LENGTH 128

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

void get_users(char user_names[possible_users_to_display][USER_NAME_LENGTH], int count) {
  static int user_id = 1;
  for (int i = 0; i < count; ++i) {
      snprintf(user_names[i], USER_NAME_LENGTH, "skbd_user#%d", user_id++);
  }
}

struct Group {
  int id;
  int user_count;
  int *user_ids;
};

struct User {
  int id;
  char user_name[USER_NAME_LENGTH];
  int friend_count;
  int *friend_ids;
  int group_count;
  int *group_ids;
  char password[PASSWORD_LENGTH];
};

struct Message {
  int id;
  int from_user_id;
  int to_user_id;
  char contents[MESSAGE_LENGTH];
};

enum MAIN_VIEW {
  MAIN_VIEW_LOGIN,
  MAIN_VIEW_DASHBOARD
};

static struct User test_user = { 0, "user", 0, NULL, 0, NULL, "password" };

static struct User users[40];

int string_compare(char *string1, int len1, char *string2, int len2) {
  int len = len1 < len2 ? len1 : len2;
  for (int i = 0; i < len; ++i) {
    if (string1[i] < string2[i]) {
      return 1;
    } else if (string1[i] > string2[i]) {
      return -1;
    }
  }
  if (len1 > len2) {
    return -1;
  } else if (len1 < len2) {
    return 1;
  }
  return 0;
}

typedef struct AuthResult {
  int was_successful;
  struct User data;
} AuthResult;

AuthResult authenticate(char user_name[USER_NAME_LENGTH], char password[PASSWORD_LENGTH]) {
  AuthResult result;
  result.was_successful = 0;
  for (int i = 0; i < 40; ++i) {
    if ((string_compare(user_name, USER_NAME_LENGTH, users[i].user_name, USER_NAME_LENGTH) == 0) &&
        (string_compare(password, PASSWORD_LENGTH, users[i].password, PASSWORD_LENGTH) == 0))
    {
      result.was_successful = 1;
      result.data = users[i];
      break;
    }
  }
  return result;
}

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



    for (int i = 0; i < 10; ++i) {
      snprintf(users[i].user_name, USER_NAME_LENGTH, "skbd_user#%d", i + 1);
    }

    for (int i = 0; i < 10; ++i) {
      snprintf(users[i].password, PASSWORD_LENGTH, "skbd_password#%d", i + 1);
    }

    users[10] = test_user;

    char user_messages[10][MESSAGE_LENGTH];
    for (int i = 0; i < 10; ++i) {
      snprintf(user_messages[i], MESSAGE_LENGTH, "%s msg#%d", users[i].user_name, i + 1);
    }

    char current_message[MESSAGE_LENGTH];
    int actual_length = 0;
    
    char approved_message[MESSAGE_LENGTH];

    enum MAIN_VIEW current_view = MAIN_VIEW_LOGIN;

    char current_user_name[USER_NAME_LENGTH] = {0};
    int current_user_name_len = 0;
    char password[PASSWORD_LENGTH] = {0};
    int password_len = 0;
    int is_first_try = 1;
    int is_login_success = 0;

    struct User logged_in_user;

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

      /* GUI */
      if (current_view == MAIN_VIEW_LOGIN) {
        int center_x = WINDOW_WIDTH / 2;
        int center_y = WINDOW_HEIGHT / 2;
        int login_width = 200;
        int login_height = 120;
        if (nk_begin(ctx, "skibidi login",
              nk_rect(center_x - login_width / 2, center_y - login_height / 2,
                login_width, login_height),
              NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
          nk_layout_row_dynamic(ctx, 30, 2);

          nk_label(ctx, "Login", NK_TEXT_LEFT);
          nk_edit_string(ctx, NK_EDIT_FIELD, current_user_name, &current_user_name_len,
                          USER_NAME_LENGTH, nk_filter_default);

          nk_label(ctx, "Password", NK_TEXT_LEFT);
          nk_edit_string(ctx, NK_EDIT_FIELD, password, &password_len,
                          128, nk_filter_default);
          if (nk_button_label(ctx, "Login")) {
            is_first_try = 0;
            AuthResult result = authenticate(current_user_name, password);;
            is_login_success = result.was_successful;
            if (is_login_success) {
              current_view = MAIN_VIEW_DASHBOARD;
              logged_in_user = result.data;
            }
          }
          if (nk_button_label(ctx, "Sign Up")) {
          }
          if (!is_login_success && !is_first_try) {
            nk_label_colored(ctx, "Wrong credentials, try again.", NK_TEXT_LEFT, nk_rgb(255, 0, 0));
          }
        }
        nk_end(ctx);
      }
      else if (current_view == MAIN_VIEW_DASHBOARD) {
        if (nk_begin(ctx, "skibidi main window",
                     nk_rect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT),
                     NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
          float ratio[] = {0.1f, 0.35f, 0.55f};
          nk_layout_row(ctx, NK_DYNAMIC, WINDOW_HEIGHT, 3, ratio);
          if (nk_group_begin(ctx, "groups", 0)) {
             
            nk_group_end(ctx);
          }
          if (nk_group_begin(ctx, "users", 0)) {
            nk_layout_row_dynamic(ctx, 30, 1);
            char user_names[possible_users_to_display][USER_NAME_LENGTH];
            get_users(user_names, possible_users_to_display);
            for (int i = 0; i < possible_users_to_display; ++i) {
              if (nk_button_label(ctx, user_names[i])) {
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
              nk_label(ctx, users[current_user].user_name, NK_TEXT_LEFT);
              nk_group_end(ctx);
            }

            nk_layout_row_dynamic(ctx, WINDOW_HEIGHT - 120, 1);
            if (nk_group_begin(ctx, "messages", 0)) {
              if (first_scroll) {
                nk_group_set_scroll(ctx, "messages", 0, 13 * 30);
                first_scroll = 0;
              }
              nk_layout_row_dynamic(ctx, 30, 1);
              for (int i = 0; i < 10; ++i) {
                message_widget(ctx, users[current_user].user_name,
                               user_messages[current_user]);
              }
              nk_group_end(ctx);
            }

            nk_layout_row_dynamic(ctx, 50, 1);
            if (nk_group_begin(ctx, "message_field", 0)) {
              float ratio[] = {0.9f, 0.1f};
              nk_layout_row(ctx, NK_DYNAMIC, 30, 2, ratio);
              nk_edit_string(ctx, NK_EDIT_FIELD, current_message,
                             &actual_length, MESSAGE_LENGTH, nk_filter_default);
              if (nk_button_label(ctx, "Send")) {
                copy_string(approved_message, MESSAGE_LENGTH, current_message, MESSAGE_LENGTH);
                memset(current_message, 0, MESSAGE_LENGTH);
                actual_length = 0;
              }
              nk_group_end(ctx);
            }
            nk_group_end(ctx);
          }
        }

        nk_end(ctx);
      }

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
