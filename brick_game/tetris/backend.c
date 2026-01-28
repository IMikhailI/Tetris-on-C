#include "backend.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static int load_high_score(void) {
  int high = 0;
  FILE *file = fopen("highscore.txt", "r");
  if (file) {
    if (fscanf(file, "%d", &high) != 1) {
      high = 0;
    }
    fclose(file);
  }
  return high;
}

static void save_high_score(int high) {
  FILE *file = fopen("highscore.txt", "w");
  if (file) {
    fprintf(file, "%d", high);
    fclose(file);
  }
}

static int **tetromino_to_matrix(const Tetromino *t) {
  int **matrix = create_matrix(4, 4);
  for (int i = 0; i < t->size; i++) {
    for (int j = 0; j < t->size; j++) {
      matrix[i][j] = t->shape[i][j];
    }
  }
  return matrix;
}

UserAction_t resolve_input(int ch) {
  UserAction_t action = Start;
  switch (ch) {
    case KEY_LEFT:
      action = Left;
      break;
    case KEY_RIGHT:
      action = Right;
      break;
    case KEY_DOWN:
      action = Down;
      break;
    case ' ':
      action = Action;
      break;
    case 'p':
    case 'P':
      action = Pause;
      break;
    case 'q':
    case 'Q':
      action = Terminate;
      break;
    default:
      action = Start;
      break;
  }
  return action;
}

TetrisState *get_state() {
  static TetrisState *state = NULL;
  if (state == NULL) {
    state = malloc(sizeof(TetrisState));
    memset(state, 0, sizeof(TetrisState));
  }
  return state;
}

Tetromino generate_random_tetromino() {
  static const int figures[7][4][4] = FIGURES_ARRAY;
  Tetromino t;
  int idx = rand() % 7;
  static const int sizes[7] = {4, 2, 3, 3, 3, 3, 3};
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      t.shape[i][j] = figures[idx][i][j];
    }
  }
  t.size = sizes[idx];
  t.x = (FIELD_W - t.size) / 2;
  t.y = 0;
  return t;
}

static void rotate_current_tetromino(TetrisState *state) {
  int temp[4][4] = {0};
  int size = state->current.size;
  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      temp[j][size - 1 - i] = state->current.shape[i][j];
    }
  }
  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      state->current.shape[i][j] = temp[i][j];
    }
  }
}

void init_game_info(GameInfo_t *game_info) {
  game_info->field = create_matrix(FIELD_H, FIELD_W);
  game_info->next = NULL;
  game_info->score = 0;
  game_info->high_score = load_high_score();
  game_info->level = 0;
  game_info->speed = 1;
  game_info->pause = 0;

  TetrisState *state = get_state();
  state->game_info = *game_info;
  state->current = generate_random_tetromino();
  state->next = generate_random_tetromino();
  state->game_info.next = game_info->next = tetromino_to_matrix(&state->next);
}

int **create_matrix(int N, int M) {
  int **result = calloc(N, sizeof(int *));
  bool error_occurred = false;

  if (result != NULL) {
    for (int i = 0; i < N; i++) {
      if (!error_occurred) {
        result[i] = calloc(M, sizeof(int));
        if (result[i] == NULL) {
          error_occurred = true;
        }
      } else {
        result[i] = NULL;
      }
    }

    if (error_occurred) {
      for (int i = 0; i < N; i++) {
        if (result[i] != NULL) free(result[i]);
      }
      free(result);
      result = NULL;
    } else {
      for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
          result[i][j] = 0;
        }
      }
    }
  }
  return result;
}

void free_matrix(int N, int **matrix) {
  if (matrix != NULL) {
    for (int i = 0; i < N; i++) {
      free(matrix[i]);
    }
    free(matrix);
  }
  matrix = NULL;
}

bool check_collision(TetrisState *state, int dx, int dy) {
  bool collision = false;
  int size = state->current.size;
  for (int i = 0; i < size && !collision; i++) {
    for (int j = 0; j < size && !collision; j++) {
      if (state->current.shape[i][j]) {
        int x = state->current.x + j + dx;
        int y = state->current.y + i + dy;
        if (x < 0 || x >= FIELD_W || y < 0 || y >= FIELD_H) {
          collision = true;
        } else if (state->game_info.field[y][x] == 2) {
          collision = true;
        }
      }
    }
  }
  return collision;
}

void lock_tetromino(TetrisState *state, int **field) {
  int size = state->current.size;
  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      if (state->current.shape[i][j]) {
        int x = state->current.x + j;
        int y = state->current.y + i;
        if (x >= 0 && x < FIELD_W && y >= 0 && y < FIELD_H) {
          field[y][x] = 2;
        }
      }
    }
  }
}

static void overlay_tetromino(TetrisState *state, int **field) {
  int size = state->current.size;
  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      if (state->current.shape[i][j]) {
        int x = state->current.x + j;
        int y = state->current.y + i;
        if (x >= 0 && x < FIELD_W && y >= 0 && y < FIELD_H) {
          if (field[y][x] != 2) field[y][x] = 1;
        }
      }
    }
  }
}

void clear_lines(TetrisState *state, int **field) {
  int cleared = 0;
  for (int i = 0; i < FIELD_H; i++) {
    bool full_line = true;
    for (int j = 0; j < FIELD_W && full_line == true; j++) {
      if (field[i][j] != 2) {
        full_line = false;
      }
    }
    if (full_line) {
      cleared++;
      for (int k = i; k > 0; k--) {
        for (int j = 0; j < FIELD_W; j++) {
          field[k][j] = field[k - 1][j];
        }
      }
      for (int j = 0; j < FIELD_W; j++) {
        field[0][j] = 0;
      }
      i--;
    }
  }

  switch (cleared) {
    case 1:
      state->game_info.score += 100;
      break;
    case 2:
      state->game_info.score += 300;
      break;
    case 3:
      state->game_info.score += 700;
      break;
    case 4:
      state->game_info.score += 1500;
      break;
    default:
      break;
  }
}

GameInfo_t updateCurrentState() {
  TetrisState *state = get_state();
  static int frame_count = 0;
  frame_count++;
  int new_level = state->game_info.score / 600;
  if (new_level > 10) {
    new_level = 10;
  }
  if (new_level > state->game_info.level) {
    state->game_info.level = new_level;
    state->game_info.speed = new_level;
  }
  int fall_threshold = 10 - state->game_info.speed;
  if (fall_threshold < 1) fall_threshold = 1;
  int **temp = create_matrix(FIELD_H, FIELD_W);
  for (int i = 0; i < FIELD_H; i++) {
    for (int j = 0; j < FIELD_W; j++) {
      temp[i][j] = (state->game_info.field[i][j] == 2) ? 2 : 0;
    }
  }
  if (!state->game_info.pause && frame_count >= fall_threshold) {
    frame_count = 0;
    if (!check_collision(state, 0, 1)) {
      state->current.y++;
    } else {
      lock_tetromino(state, temp);
      clear_lines(state, temp);
      state->current = state->next;
      state->current.x = (state->current.size == 2) ? 4 : 3;
      state->current.y = 0;
      state->next = generate_random_tetromino();
      if (check_collision(state, 0, 0)) {
        state->game_info.pause = 111;
      }
      free_matrix(4, state->game_info.next);
      state->game_info.next = tetromino_to_matrix(&state->next);
    }
  }
  overlay_tetromino(state, temp);
  for (int i = 0; i < FIELD_H; i++) {
    for (int j = 0; j < FIELD_W; j++) {
      state->game_info.field[i][j] = temp[i][j];
    }
  }
  free_matrix(FIELD_H, temp);
  if (state->game_info.score > state->game_info.high_score) {
    state->game_info.high_score = state->game_info.score;
    save_high_score(state->game_info.high_score);
  }
  return state->game_info;
}

static void handle_left(TetrisState *state) {
  if (!check_collision(state, -1, 0)) state->current.x--;
}

static void handle_right(TetrisState *state) {
  if (!check_collision(state, 1, 0)) state->current.x++;
}

static void handle_drop(TetrisState *state) {
  while (!check_collision(state, 0, 1)) {
    state->current.y++;
  }
  lock_tetromino(state, state->game_info.field);
  clear_lines(state, state->game_info.field);

  state->current = state->next;
  state->current.x = (state->current.size == 2) ? 4 : 3;
  state->current.y = 0;

  state->next = generate_random_tetromino();

  free_matrix(4, state->game_info.next);
  state->game_info.next = tetromino_to_matrix(&state->next);
}

static void handle_rotation(TetrisState *state) {
  Tetromino old = state->current;
  rotate_current_tetromino(state);
  if (check_collision(state, 0, 0)) {
    state->current.x--;
    if (check_collision(state, 0, 0)) {
      state->current = old;
      rotate_current_tetromino(state);
      state->current.x++;
      if (check_collision(state, 0, 0)) {
        state->current = old;
      }
    }
  }
}

static void handle_pause(TetrisState *state) {
  state->game_info.pause = !state->game_info.pause;
}

void userInput(UserAction_t action) {
  TetrisState *state = get_state();
  if (state->game_info.pause != 111 &&
      !(state->game_info.pause == 1 && action != Pause &&
        action != Terminate)) {
    switch (action) {
      case Left:
        handle_left(state);
        break;
      case Right:
        handle_right(state);
        break;
      case Down:
        handle_drop(state);
        break;
      case Action:
        handle_rotation(state);
        break;
      case Pause:
        handle_pause(state);
        break;
      case Terminate:
        break;
      default:
        break;
    }
  }
  return;
}
