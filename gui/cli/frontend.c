#include "frontend.h"

#include <ncurses.h>
#include <stdlib.h>

#include "../../brick_game.h"

#define GAME_WIN_HEIGHT (FIELD_H + 2)
#define GAME_WIN_WIDTH (FIELD_W * 2 + 2)
#define INFO_WIN_WIDTH 30

FrontendContext *init_ncurses() {
  FrontendContext *ctx = NULL;

  initscr();
  raw();
  noecho();
  keypad(stdscr, TRUE);
  curs_set(0);
  nodelay(stdscr, TRUE);
  start_color();

  init_pair(1, COLOR_RED, COLOR_BLACK);
  init_pair(2, COLOR_WHITE, COLOR_BLACK);
  init_pair(3, COLOR_CYAN, COLOR_BLACK);

  ctx = malloc(sizeof(FrontendContext));
  if (ctx != NULL) {
    ctx->game_win = newwin(GAME_WIN_HEIGHT, GAME_WIN_WIDTH, 1, 1);
    ctx->info_win =
        newwin(GAME_WIN_HEIGHT, INFO_WIN_WIDTH, 1, GAME_WIN_WIDTH + 2);

    if (ctx->game_win == NULL || ctx->info_win == NULL) {
      delwin(ctx->game_win);
      delwin(ctx->info_win);
      free(ctx);
      ctx = NULL;
    }
  }
  if (ctx == NULL) {
    endwin();
  }
  return ctx;
}

static void render_game_field(FrontendContext *ctx, GameInfo_t *game_info) {
  werase(ctx->game_win);
  box(ctx->game_win, 0, 0);
  for (int i = 0; i < FIELD_H; i++) {
    for (int j = 0; j < FIELD_W; j++) {
      int val = game_info->field[i][j];
      if (val) {
        wattron(ctx->game_win, COLOR_PAIR(1));
        mvwprintw(ctx->game_win, i + 1, j * 2 + 1, "[]");
        wattroff(ctx->game_win, COLOR_PAIR(1));
      } else {
        mvwprintw(ctx->game_win, i + 1, j * 2 + 1, "  ");
      }
    }
  }
  wrefresh(ctx->game_win);
}

static void render_info_panel(FrontendContext *ctx, GameInfo_t *game_info) {
  werase(ctx->info_win);
  box(ctx->info_win, 0, 0);
  wattron(ctx->info_win, COLOR_PAIR(2));

  mvwprintw(ctx->info_win, 1, 12, "TETRIS");
  mvwprintw(ctx->info_win, 3, 2, "Score:      %d", game_info->score);
  mvwprintw(ctx->info_win, 4, 2, "High Score: %d", game_info->high_score);
  mvwprintw(ctx->info_win, 5, 2, "Level:      %d", game_info->level);
  mvwprintw(ctx->info_win, 7, 2, "Controls:");
  mvwprintw(ctx->info_win, 8, 4, "Move: <- ->");
  mvwprintw(ctx->info_win, 9, 4, "Drop: down");
  mvwprintw(ctx->info_win, 10, 4, "Rotate: Space");
  mvwprintw(ctx->info_win, 11, 4, "Pause: P");
  mvwprintw(ctx->info_win, 12, 4, "Quit: Q");

  wattroff(ctx->info_win, COLOR_PAIR(2));

  mvwprintw(ctx->info_win, 14, 2, "Next:");
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      if (game_info->next[i][j]) {
        wattron(ctx->info_win, COLOR_PAIR(4));
        mvwprintw(ctx->info_win, 15 + i, 2 + j * 2, "[]");
        wattroff(ctx->info_win, COLOR_PAIR(4));
      } else {
        mvwprintw(ctx->info_win, 15 + i, 2 + j * 2, "  ");
      }
    }
  }

  if (game_info->pause == 111) {
    wattron(ctx->info_win, COLOR_PAIR(5));
    mvwprintw(ctx->info_win, 20, 2, "GAME OVER!");
    wattroff(ctx->info_win, COLOR_PAIR(5));
  }

  wrefresh(ctx->info_win);
}

void rendering(FrontendContext *ctx, GameInfo_t *game_info) {
  render_game_field(ctx, game_info);
  render_info_panel(ctx, game_info);
}
