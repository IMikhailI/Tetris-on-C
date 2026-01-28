#ifndef BACKEND_H
#define BACKEND_H

/**
 * @file backend.h
 * @brief Описание логики игры (TetrisState, функции updateCurrentState и т.д.)
 */

#include <ncurses.h>
#include <stdlib.h>

#include "../../brick_game.h"

/**
 * @def START_X
 * @brief Начальная координата X для появления фигур (не всегда используется)
 */
#define START_X 4

/**
 * @def START_Y
 * @brief Начальная координата Y (часто отрицательная для появления сверху)
 */
#define START_Y (-4)

/**
 * @def FIGURES_ARRAY
 * @brief Массив, задающий 7 фигур Tetris в формате 4x4
 */
#define FIGURES_ARRAY                                               \
  {                                                                 \
    {{0, 0, 1, 0}, {0, 0, 1, 0}, {0, 0, 1, 0}, {0, 0, 1, 0}},       \
        {{1, 1, 0, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},   \
        {{0, 1, 0, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},   \
        {{0, 1, 1, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},   \
        {{1, 1, 0, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},   \
        {{1, 0, 0, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}, { \
      {0, 0, 1, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}, { 0, 0, 0, 0 }      \
    }                                                               \
  }

/**
 * @enum UserAction_t
 * @brief Перечисление возможных действий пользователя в Tetris
 */
typedef enum {
  Start,     /**< Начало игры         */
  Pause,     /**< Пауза               */
  Terminate, /**< Завершение игры     */
  Left,      /**< Движение влево      */
  Right,     /**< Движение вправо     */
  Up,        /**< Вверх (не используется) */
  Down,      /**< Сброс фигуры вниз   */
  Action     /**< Вращение фигуры     */
} UserAction_t;

/**
 * @struct Tetromino
 * @brief Представление текущей (или следующей) фигуры
 */
typedef struct {
  int shape[4][4]; /**< Матрица фигуры (4x4) */
  int size; /**< Логический размер фигуры (2 для O, 3 для T,S,Z,J,L, 4 для I) */
  int x, y; /**< Координаты фигуры на поле */
} Tetromino;

/**
 * @struct TetrisState
 * @brief Внутреннее состояние игры Tetris, включающее GameInfo_t и
 * текущее/следующее тетромино
 */
typedef struct {
  GameInfo_t game_info; /**< Данные для отрисовки (поле, счет, рекорд и т.д.) */
  Tetromino current;    /**< Текущая фигура */
  Tetromino next;       /**< Следующая фигура */
} TetrisState;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Инициализация состояния игры
 * @param game_info [out] Структура с полем, счетом, рекордом и т.д.
 */
void init_game_info(GameInfo_t *game_info);

/**
 * @brief Преобразует код клавиши (ch) в действие (UserAction_t)
 * @param ch Нажатая клавиша
 * @return UserAction_t - действие пользователя (Left, Right, Down, Action и
 * т.д.)
 */
UserAction_t resolve_input(int ch);

/**
 * @brief Создаёт матрицу размером N x M и инициализирует её
 * @param N количество строк
 * @param M количество столбцов
 * @return Указатель на массив указателей (матрица int**)
 */
int **create_matrix(int N, int M);

/**
 * @brief Освобождает матрицу, созданную create_matrix
 * @param N число строк
 * @param matrix матрица
 */
void free_matrix(int N, int **matrix);

/**
 * @brief Обрабатывает пользовательский ввод (движение, сброс, поворот, пауза)
 * @param action действие пользователя (см. UserAction_t)
 */
void userInput(UserAction_t action);

/**
 * @brief Обновляет состояние игры (падение фигур, очистка линий, подсчет очков)
 * @return Актуальное состояние GameInfo_t
 */
GameInfo_t updateCurrentState();
TetrisState *get_state();
Tetromino generate_random_tetromino();
void lock_tetromino(TetrisState *state, int **field);
void clear_lines(TetrisState *state, int **field);
bool check_collision(TetrisState *state, int dx, int dy);

#ifdef __cplusplus
}
#endif

#endif  // BACKEND_H
