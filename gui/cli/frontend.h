#ifndef FRONTEND_H
#define FRONTEND_H

#include <ncurses.h>

#include "../../brick_game.h"

/**
 * @file frontend.h
 * @brief Терминальный интерфейс для Tetris (ncurses)
 */

/**
 * @struct FrontendContext
 * @brief Хранит окна ncurses для отрисовки поля и панели информации
 */
typedef struct {
  WINDOW *game_win; /**< окно для отрисовки игрового поля */
  WINDOW *info_win; /**< окно для отрисовки информационной панели */
} FrontendContext;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Инициализация ncurses и создание окон (game_win, info_win)
 * @return указатель на созданный FrontendContext
 */
FrontendContext *init_ncurses();

/**
 * @brief Отрисовка игрового поля и информационной панели
 * @param ctx контекст фронтенда (окна)
 * @param game_info данные об игре (поле, счет, уровень, рекорд и т.д.)
 */
void rendering(FrontendContext *ctx, GameInfo_t *game_info);

#ifdef __cplusplus
}
#endif

#endif  // FRONTEND_H
