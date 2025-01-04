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

int compare_string(char *string1, int len1, char *string2, int len2) {
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

void copy_string(char *dest, int dest_len, char *src, int src_len) {
  int len = dest_len > src_len ? src_len : dest_len;
  for (int i = 0; i < len; ++i) {
    dest[i] = src[i];
  }
}

#define GROUP_NAME_LENGTH 128

typedef struct Group {
  int id;
  int user_count;
  char name[GROUP_NAME_LENGTH];
} Group;

static Group groups[100];
static int group_empty_index;

int create_group(char name[GROUP_NAME_LENGTH]) {
  if (group_empty_index >= 100) {
    return 0;
  }
  groups[group_empty_index].id = group_empty_index;
  copy_string(groups[group_empty_index].name, GROUP_NAME_LENGTH, name,
              GROUP_NAME_LENGTH);
  ++group_empty_index;
  return 1;
}

typedef struct GroupUsers {
  int group_id;
  int user_id;
} GroupUsers;

typedef struct User {
  int id;
  int friend_count;
  int group_count;
  char user_name[USER_NAME_LENGTH];
  int user_name_len;
  char password[PASSWORD_LENGTH];
  int password_len;
} User;

typedef struct ClientSideUser {
  int id;
  char user_name[USER_NAME_LENGTH];
  int user_name_len;
} ClientSideUser;

ClientSideUser user_to_client_side(User *user) {
    ClientSideUser result;
    result.id = user->id;
    copy_string(result.user_name, USER_NAME_LENGTH,
                user->user_name, USER_NAME_LENGTH);
    result.user_name_len = user->user_name_len;
    return result;
}

enum CREATE_USER_STATUS {
  CREATE_USER_SUCCESS,
  CREATE_USER_ALREADY_EXISTS,
  CREATE_USER_USER_LIMIT,
  CREATUE_USER_NAME_TOO_LONG,
  CREATE_USER_PASSWORD_TOO_LONG
};

#define MAX_USERS 40

static struct User all_users[MAX_USERS];
static int users_empty_index;

/* int request_create_user(char user_name[USER_NAME_LENGTH], char
 * password[PASSWORD_LENGTH]) { */
/*   if (users_empty_index == MAX_USERS) */
/*     return CREATE_USER_USER_LIMIT; */
/*   for (int i = 0; i < users_empty_index; ++i) { */
/*     if (compare_string(user_name, USER_NAME_LENGTH, users[i].user_name,
 * USER_NAME_LENGTH) == 0) */
/*       return CREATE_USER_ALREADY_EXISTS; */
/*   } */
/**/
/*   users[users_empty_index].id = users_empty_index; */
/*   copy_string(users[users_empty_index].user_name, USER_NAME_LENGTH, */
/*               user_name, USER_NAME_LENGTH); */
/*   users[users_empty_index].user_name_len = USER_NAME_LENGTH; */
/*   copy_string(users[users_empty_index].password, PASSWORD_LENGTH, */
/*               password, PASSWORD_LENGTH); */
/*   users[users_empty_index].password_len = PASSWORD_LENGTH; */
/**/
/*   ++users_empty_index; */
/*   return CREATE_USER_SUCCESS; */
/* } */

int request_create_user(char user_name[], int user_name_len, char password[],
                        int password_len) {
  if (users_empty_index == MAX_USERS)
    return CREATE_USER_USER_LIMIT;
  if (user_name_len >= USER_NAME_LENGTH)
    return CREATUE_USER_NAME_TOO_LONG;
  if (password_len >= PASSWORD_LENGTH)
    return CREATE_USER_PASSWORD_TOO_LONG;
  for (int i = 0; i < users_empty_index; ++i) {
    if (compare_string(user_name, user_name_len, all_users[i].user_name,
                       all_users[i].user_name_len) == 0)
      return CREATE_USER_ALREADY_EXISTS;
  }

  all_users[users_empty_index].id = users_empty_index;
  copy_string(all_users[users_empty_index].user_name, USER_NAME_LENGTH, user_name,
              user_name_len);
  all_users[users_empty_index].user_name_len = user_name_len;
  copy_string(all_users[users_empty_index].password, PASSWORD_LENGTH, password,
              password_len);
  all_users[users_empty_index].password_len = password_len;

  ++users_empty_index;
  return CREATE_USER_SUCCESS;
}

int request_create_user_terminated(char user_name[], char password[]) {
  if (users_empty_index == MAX_USERS)
    return CREATE_USER_USER_LIMIT;
  int user_name_len = strlen(user_name);
  int password_len = strlen(password);
  if (user_name_len >= USER_NAME_LENGTH)
    return CREATUE_USER_NAME_TOO_LONG;
  if (password_len >= PASSWORD_LENGTH)
    return CREATE_USER_PASSWORD_TOO_LONG;
  for (int i = 0; i < users_empty_index; ++i) {
    if (compare_string(user_name, user_name_len, all_users[i].user_name,
                       all_users[i].user_name_len) == 0)
      return CREATE_USER_ALREADY_EXISTS;
  }

  all_users[users_empty_index].id = users_empty_index;
  copy_string(all_users[users_empty_index].user_name, USER_NAME_LENGTH, user_name,
              user_name_len);
  all_users[users_empty_index].user_name_len = user_name_len;
  copy_string(all_users[users_empty_index].password, PASSWORD_LENGTH, password,
              password_len);
  all_users[users_empty_index].password_len = password_len;

  ++users_empty_index;
  return CREATE_USER_SUCCESS;
}

/* void debug_create_users() */

typedef struct UserFriends {
  int user1_id;
  int user2_id;
} UserFriends;

#define MAX_FRIENDS_PER_USER 40
#define MAX_FRIENDS_SYSTEM MAX_FRIENDS_PER_USER *MAX_USERS

static UserFriends users_friends[MAX_FRIENDS_SYSTEM];
static int users_friends_empty_index;

int add_friend(int user1_id, int user2_id) {
  if (all_users[user1_id].friend_count >= MAX_FRIENDS_PER_USER ||
      all_users[user2_id].friend_count >= MAX_FRIENDS_PER_USER ||
      users_friends_empty_index >= (MAX_FRIENDS_PER_USER * MAX_USERS))
    return 0;

  users_friends[users_friends_empty_index].user1_id = user1_id;
  users_friends[users_friends_empty_index].user2_id = user2_id;

  ++users_friends_empty_index;

  users_friends[users_friends_empty_index].user1_id = user2_id;
  users_friends[users_friends_empty_index].user2_id = user1_id;

  ++users_friends_empty_index;

  ++all_users[user1_id].friend_count;
  ++all_users[user2_id].friend_count;

  return 1;
}

// friends needs to be able to hold all ClintSideUser friends in memory
int get_friends(int user_id, ClientSideUser *friends) {
  int current_friend_index = 0;
  for (int i = 0; i < MAX_FRIENDS_SYSTEM; ++i) {
    if (users_friends[i].user1_id == user_id) {
      friends[current_friend_index].id = users_friends[i].user2_id;
      copy_string(friends[current_friend_index].user_name, USER_NAME_LENGTH,
                  all_users[users_friends[i].user2_id].user_name, USER_NAME_LENGTH);
      friends[current_friend_index].user_name_len = all_users[users_friends[i].user2_id].user_name_len;
      ++current_friend_index;
    }
  }
  return 1;
}

void search_users(char name[USER_NAME_LENGTH], int name_length,
                  ClientSideUser users[], int max_num_results, int *results_num) {
  int found_index = 0;
  // NOTE: is having this in memory relevant?
  /* memset(users, 0, sizeof(ClientSideUser) * max_num_results); */
  for (int i = 0; i < users_empty_index && found_index < max_num_results; ++i) {
    if (compare_string(name, name_length, all_users[i].user_name, all_users[i].user_name_len) == 0) {
      users[found_index++] = user_to_client_side(&all_users[i]);
    }
  }
  *results_num = found_index;
}

struct Message {
  int id;
  int from_user_id;
  int to_user_id;
  char contents[MESSAGE_LENGTH];
};

enum MAIN_VIEW { MAIN_VIEW_LOGIN, MAIN_VIEW_SIGNUP, MAIN_VIEW_DASHBOARD };

typedef struct AuthResult {
  int was_successful;
  struct User data;
} AuthResult;

AuthResult authenticate(char user_name[USER_NAME_LENGTH], int user_name_len,
                        char password[PASSWORD_LENGTH], int password_len) {
  AuthResult result;
  result.was_successful = 0;
  for (int i = 0; i < users_empty_index; ++i) {
    if ((compare_string(user_name, user_name_len, all_users[i].user_name,
                        all_users[i].user_name_len) == 0) &&
        (compare_string(password, password_len, all_users[i].password,
                        all_users[i].password_len) == 0)) {
      result.was_successful = 1;
      result.data = all_users[i];
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
  win = SDL_CreateWindow("Skibidi Chat", SDL_WINDOWPOS_CENTERED,
                         SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT,
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
  // TODO: Better font handling
  // {
  struct nk_font_atlas *atlas;
  nk_sdl_font_stash_begin(&atlas);
  /*struct nk_font *droid = nk_font_atlas_add_from_file(atlas,
   * "../../../extra_font/DroidSans.ttf", 14, 0);*/
  struct nk_font *roboto18 =
      nk_font_atlas_add_from_file(atlas, "../fonts/Roboto-Regular.ttf", 18, 0);
  struct nk_font *roboto24 =
      nk_font_atlas_add_from_file(atlas, "../fonts/Roboto-Regular.ttf", 24, 0);
  /*struct nk_font *future = nk_font_atlas_add_from_file(atlas,
   * "../../../extra_font/kenvector_future_thin.ttf", 13, 0);*/
  /* struct nk_font *clean = nk_font_atlas_add_from_file(atlas, */
  /*  "../fonts/ProggyClean.ttf", 24, 0); */
  /*struct nk_font *tiny = nk_font_atlas_add_from_file(atlas,
   * "../../../extra_font/ProggyTiny.ttf", 10, 0);*/
  /*struct nk_font *cousine = nk_font_atlas_add_from_file(atlas,
   * "../../../extra_font/Cousine-Regular.ttf", 13, 0);*/
  nk_sdl_font_stash_end();
  /*nk_style_load_all_cursors(ctx, atlas->cursors);*/
  nk_style_set_font(ctx, &roboto18->handle);
  // }

  int current_user = 0;

  int first_scroll = 1;

  bg.r = 0.10f, bg.g = 0.18f, bg.b = 0.24f, bg.a = 1.0f;

  FILE *database = fopen("data.db", "rb");
  if (database) {
    fread(all_users, sizeof(User), MAX_USERS, database);
    fread(&users_empty_index, sizeof(int), 1, database);
    fread(users_friends, sizeof(UserFriends), MAX_FRIENDS_SYSTEM, database);
    fread(&users_friends_empty_index, sizeof(int), 1, database);
    fclose(database);
  } else {
    fprintf(stderr, "Couldn't open the database for reading.\n");
  }

  /* add_friend(9, 10); */

  char user_messages[10][MESSAGE_LENGTH];
  for (int i = 0; i < 10; ++i) {
    snprintf(user_messages[i], MESSAGE_LENGTH, "%s msg#%d", all_users[i].user_name,
             i + 1);
  }

  char current_message[MESSAGE_LENGTH];
  int actual_length = 0;

  char approved_message[MESSAGE_LENGTH];

  enum MAIN_VIEW current_view = MAIN_VIEW_LOGIN;

  char current_user_name[USER_NAME_LENGTH] = {0};
  int current_user_name_len = 0;
  char password[PASSWORD_LENGTH] = {0};
  int password_len = 0;
  char password_check[PASSWORD_LENGTH] = {0};
  int password_check_len = 0;
  int is_first_login_try = 1;
  int is_login_success = 0;
  int is_signup_error = 0;
  int are_password_different = 0;
  char searched_user_or_group[USER_NAME_LENGTH] = {0};
  int searched_user_or_group_len = 0;

  ClientSideUser suggested_users[10] = {0};

  struct User logged_in_user;
  ClientSideUser *friends = NULL;

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
      int login_height = 200;
      if (nk_begin(ctx, "skibidi login",
                   nk_rect(center_x - login_width / 2,
                           center_y - login_height / 2, login_width,
                           login_height),
                   NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR)) {
        nk_layout_row_dynamic(ctx, 30, 2);

        nk_label(ctx, "Login", NK_TEXT_LEFT);
        nk_edit_string(ctx, NK_EDIT_FIELD, current_user_name,
                       &current_user_name_len, USER_NAME_LENGTH,
                       nk_filter_default);

        nk_label(ctx, "Password", NK_TEXT_LEFT);
        nk_edit_string(ctx, NK_EDIT_FIELD, password, &password_len, 128,
                       nk_filter_default);
        if (nk_button_label(ctx, "Login")) {
          is_first_login_try = 0;
          AuthResult result = authenticate(
              current_user_name, current_user_name_len, password, password_len);
          is_login_success = result.was_successful;
          if (is_login_success) {
            logged_in_user = result.data;
            int is_data_retrieval_success = 1;
            if (logged_in_user.friend_count > 0) {
              //TODO: crash without this if statement in glClear, debug more
              friends = (ClientSideUser *)malloc(sizeof(ClientSideUser) *
                                                 logged_in_user.friend_count);
              is_data_retrieval_success =
                  get_friends(logged_in_user.id, friends);
            }
            if (is_data_retrieval_success)
              current_view = MAIN_VIEW_DASHBOARD;
          }
        }
        if (nk_button_label(ctx, "Sign Up")) {
          memset(current_user_name, 0, current_user_name_len);
          current_user_name_len = 0;
          memset(password, 0, password_len);
          password_len = 0;
          memset(password_check, 0, password_check_len);
          password_check_len = 0;
          current_view = MAIN_VIEW_SIGNUP;
        }
        if (!is_login_success && !is_first_login_try) {
          nk_layout_row_dynamic(ctx, 30, 1);
          nk_label_colored(ctx, "Wrong credentials, try again.", NK_TEXT_LEFT,
                           nk_rgb(255, 0, 0));
        }
      }
      nk_end(ctx);
    } else if (current_view == MAIN_VIEW_SIGNUP) {
      float center_x = WINDOW_WIDTH / 2.0;
      float center_y = WINDOW_HEIGHT / 2.0;
      float signup_width = 400;
      float signup_height = 400;
      if (nk_begin(ctx, "skibidi signup",
                   nk_rect(center_x - signup_width / 2,
                           center_y - signup_height / 2, signup_width,
                           signup_height),
                   NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR)) {
        nk_layout_row_dynamic(ctx, 30, 2);

        nk_label(ctx, "Login", NK_TEXT_LEFT);
        nk_edit_string(ctx, NK_EDIT_FIELD, current_user_name,
                       &current_user_name_len, USER_NAME_LENGTH,
                       nk_filter_default);

        nk_label(ctx, "Password", NK_TEXT_LEFT);
        nk_edit_string(ctx, NK_EDIT_FIELD, password, &password_len, PASSWORD_LENGTH,
                       nk_filter_default);
        nk_label(ctx, "Repeat Password", NK_TEXT_LEFT);
        nk_edit_string(ctx, NK_EDIT_FIELD, password_check, &password_check_len,
                       PASSWORD_LENGTH, nk_filter_default);
        if (nk_button_label(ctx, "Sign Up")) {
          is_first_login_try = 1;
          if (compare_string(password, PASSWORD_LENGTH, password_check,
                             PASSWORD_LENGTH) == 0) {
            int error_code =
                request_create_user(current_user_name, current_user_name_len,
                                    password, password_len);
            if (error_code != CREATE_USER_SUCCESS) {
              is_signup_error = 1;
            } else {
              memset(current_user_name, 0, current_user_name_len);
              current_user_name_len = 0;
              memset(password, 0, password_len);
              password_len = 0;
              memset(password_check, 0, password_check_len);
              password_check_len = 0;
              current_view = MAIN_VIEW_LOGIN;
            }
          } else {
            are_password_different = 1;
          }
        }
        if (nk_button_label(ctx, "Cancel")) {
          memset(current_user_name, 0, current_user_name_len);
          current_user_name_len = 0;
          memset(password, 0, password_len);
          password_len = 0;
          memset(password_check, 0, password_check_len);
          password_check_len = 0;
          current_view = MAIN_VIEW_LOGIN;
        }
        if (is_signup_error) {
          nk_layout_row_dynamic(ctx, 30, 1);
          // TODO: display different error messages
          nk_label_colored(ctx, "Signup error", NK_TEXT_LEFT,
                           nk_rgb(255, 0, 0));
        }
        if (are_password_different) {
          nk_layout_row_dynamic(ctx, 30, 1);
          nk_label_colored(ctx, "Passwords not matching", NK_TEXT_LEFT,
                           nk_rgb(255, 0, 0));
        }
      }
      nk_end(ctx);
    } else if (current_view == MAIN_VIEW_DASHBOARD) {
      if (nk_begin(ctx, "skibidi main window",
                   nk_rect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT),
                   NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR)) {
        static int display_find_popup = 0;
        if (display_find_popup) {
          int center_x = WINDOW_WIDTH / 2;
          int center_y = WINDOW_HEIGHT / 2;
          int popup_width = WINDOW_WIDTH / 2;
          int popup_height = WINDOW_HEIGHT / 2;
          struct nk_rect popup_rect = nk_rect(center_x - popup_width / 2,
                                     center_y - popup_height / 2,
                                     popup_width, popup_height);
          if (nk_popup_begin(ctx, NK_POPUP_STATIC, "Searching", 0, popup_rect)) {
            if (!nk_input_is_mouse_hovering_rect(&ctx->input, popup_rect) &&
                nk_input_is_mouse_pressed(&ctx->input, NK_BUTTON_LEFT)) {
              display_find_popup = 0;
              nk_popup_close(ctx);
            }
            nk_layout_row_dynamic(ctx, 30, 1);
            nk_edit_string(ctx, NK_EDIT_FIELD, searched_user_or_group, &searched_user_or_group_len,
                           USER_NAME_LENGTH, nk_filter_default);

            int results_num = 0;
            search_users(searched_user_or_group, searched_user_or_group_len,
                         suggested_users, 10, &results_num);
            for (int i = 0; i < results_num; ++i) {
              nk_layout_row_dynamic(ctx, 30, 2);
              nk_text(ctx, suggested_users[i].user_name, suggested_users[i].user_name_len, NK_TEXT_LEFT);
              nk_button_label(ctx, "Invite");
            }
            nk_popup_end(ctx);
          }
        }

        float ratio[] = {0.1f, 0.35f, 0.55f};
        nk_layout_row(ctx, NK_DYNAMIC, WINDOW_HEIGHT, 3, ratio);
        if (nk_group_begin(ctx, "spaces", 0)) {

          nk_group_end(ctx);
        }
        if (nk_group_begin(ctx, "chat rooms", 0)) {
          nk_layout_row_dynamic(ctx, 30, 1);
          if (nk_button_label(ctx, "Find")) {
            display_find_popup = 1;
          }
          if (nk_tree_push(ctx, NK_TREE_NODE, "Friends", NK_MAXIMIZED)) {
            nk_layout_row_dynamic(ctx, 30, 1);
            if (logged_in_user.friend_count == 0) {
              nk_label(ctx, "No friends added yet", NK_TEXT_LEFT);
            } else {
              for (int i = 0; i < logged_in_user.friend_count; ++i) {
                if (nk_button_label(ctx, friends[i].user_name)) {
                  current_user = i;
                  first_scroll = 1;
                }
              }
            }
            nk_tree_pop(ctx);
          }
          // TODO: Add "Groups" tree
          // DEBUG
          nk_text(ctx, current_message, actual_length, NK_TEXT_LEFT);
          nk_group_end(ctx);
        }
        if (nk_group_begin(ctx, "messages panel", NK_WINDOW_NO_SCROLLBAR)) {
          if (logged_in_user.friend_count > 0) {
            nk_layout_row_dynamic(ctx, 50, 1);
            if (nk_group_begin(ctx, "user header",
                               NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR)) {
              nk_style_push_font(ctx, &roboto24->handle);
              nk_layout_row_dynamic(ctx, 50, 1);
              nk_label(ctx, friends[current_user].user_name, NK_TEXT_LEFT);
              nk_style_pop_font(ctx);
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
                message_widget(ctx, all_users[current_user].user_name,
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
                copy_string(approved_message, MESSAGE_LENGTH, current_message,
                            MESSAGE_LENGTH);
                memset(current_message, 0, MESSAGE_LENGTH);
                actual_length = 0;
              }
              nk_group_end(ctx);
            }
          } else {
            nk_layout_row_dynamic(ctx, WINDOW_HEIGHT, 1);
            nk_label(ctx, "No chat room selected", NK_TEXT_CENTERED);
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
  database = fopen("data.db", "wb");
  if (database) {
    fwrite(all_users, sizeof(User), MAX_USERS, database);
    fwrite(&users_empty_index, sizeof(int), 1, database);
    fwrite(users_friends, sizeof(UserFriends), MAX_FRIENDS_SYSTEM, database);
    fwrite(&users_friends_empty_index, sizeof(int), 1, database);
    fclose(database);
  } else {
    fprintf(stderr, "Couldn't open the database for writing.\n");
  }

  if (friends)
    free(friends);
  nk_sdl_shutdown();
  SDL_GL_DeleteContext(glContext);
  SDL_DestroyWindow(win);
  SDL_Quit();
  return 0;
}
