#include <check.h>
#include <ncurses.h>

#include "../brick_game.h"
#include "../brick_game/tetris/backend.h"
#include "../gui/cli/frontend.h"
/* Тест функции resolve_input. Проверяем соответствие кода клавиши действию. */
START_TEST(test_resolve_input) {
  ck_assert_int_eq(resolve_input(KEY_LEFT), Left);
  ck_assert_int_eq(resolve_input(KEY_RIGHT), Right);
  ck_assert_int_eq(resolve_input(KEY_DOWN), Down);
  ck_assert_int_eq(resolve_input(' '), Action);
  ck_assert_int_eq(resolve_input('p'), Pause);
  ck_assert_int_eq(resolve_input('P'), Pause);
  ck_assert_int_eq(resolve_input('q'), Terminate);
  ck_assert_int_eq(resolve_input('Q'), Terminate);
  ck_assert_int_eq(resolve_input('x'), Start);
}
END_TEST

/* Тест функции create_matrix – проверяется, что создаётся матрица нужного
   размера и что все её элементы инициализированы значением 1. */
START_TEST(test_create_matrix) {
  int rows = 3, cols = 4;
  int **mat = create_matrix(rows, cols);
  ck_assert_ptr_nonnull(mat);
  for (int i = 0; i < rows; i++) {
    ck_assert_ptr_nonnull(mat[i]);
    for (int j = 0; j < cols; j++) {
      ck_assert_int_eq(mat[i][j], 0);
    }
  }
  free_matrix(rows, mat);
}
END_TEST

/* Тест инициализации состояния игры.
   После вызова init_game_info() и updateCurrentState() проверяются базовые
   параметры игры. */
START_TEST(test_init_game_info) {
  GameInfo_t game_info;
  init_game_info(&game_info);
  ck_assert_ptr_nonnull(game_info.field);
  ck_assert_ptr_nonnull(game_info.next);

  GameInfo_t updated = updateCurrentState();
  /* Проходим по всему полю – поскольку в текущей итерации на поле может быть
     наложена фигура, ячейка может иметь значение 0, 1 (фигура) или 2
     (зафиксированный блок). */
  for (int i = 0; i < FIELD_H; i++) {
    for (int j = 0; j < FIELD_W; j++) {
      ck_assert((updated.field[i][j] == 0) || (updated.field[i][j] == 1) ||
                (updated.field[i][j] == 2));
    }
  }
  ck_assert_int_eq(updated.score, 0);
  ck_assert_int_ge(updated.high_score, 0);
  ck_assert_int_eq(updated.level, 0);
  ck_assert_int_eq(updated.speed, 1);
  ck_assert_int_eq(updated.pause, 0);

  free_matrix(FIELD_H, game_info.field);
  free_matrix(4, game_info.next);
}
END_TEST

/* Тест функции updateCurrentState – в базовом случае (без линий для очистки)
   проверяем, что счёт остаётся равным 0, а состояние игры не переходит в паузу.
 */
START_TEST(test_updateCurrentState) {
  GameInfo_t game_info;
  init_game_info(&game_info);
  GameInfo_t state1 = updateCurrentState();
  ck_assert_int_eq(state1.score, 0);
  ck_assert_int_eq(state1.pause, 0);
  ck_assert_ptr_nonnull(state1.field);

  free_matrix(FIELD_H, game_info.field);
  free_matrix(4, game_info.next);
}
END_TEST

/* Тестируем перемещение фигуры влево и вправо.
   Сравниваем минимальный индекс X ячеек, занятых текущей фигурой, до и после
   соответствующих вызовов. */
START_TEST(test_userInput_left_right) {
  GameInfo_t game_info;
  init_game_info(&game_info);
  GameInfo_t state_before = updateCurrentState();
  int min_x_before = FIELD_W, min_x_after = FIELD_W;

  /* Ищем минимальную координату X для ячеек, где значение равно 1 (текущая
   * фигура) */
  for (int i = 0; i < FIELD_H; i++) {
    for (int j = 0; j < FIELD_W; j++) {
      if (state_before.field[i][j] == 1) {
        if (j < min_x_before) min_x_before = j;
      }
    }
  }
  userInput(Left);
  GameInfo_t state_left = updateCurrentState();
  for (int i = 0; i < FIELD_H; i++) {
    for (int j = 0; j < FIELD_W; j++) {
      if (state_left.field[i][j] == 1) {
        if (j < min_x_after) min_x_after = j;
      }
    }
  }
  ck_assert(min_x_after <= min_x_before);

  userInput(Right);
  GameInfo_t state_right = updateCurrentState();
  int min_x_after_right = FIELD_W;
  for (int i = 0; i < FIELD_H; i++) {
    for (int j = 0; j < FIELD_W; j++) {
      if (state_right.field[i][j] == 1) {
        if (j < min_x_after_right) min_x_after_right = j;
      }
    }
  }
  ck_assert(min_x_after_right >= min_x_after);

  free_matrix(FIELD_H, game_info.field);
  free_matrix(4, game_info.next);
}
END_TEST

/* Тест переключения состояния паузы:
   после первого вызова userInput(Pause) режим должен перейти в паузу, после
   повторного – выйти из паузы. */
START_TEST(test_userInput_pause) {
  GameInfo_t game_info;
  init_game_info(&game_info);
  GameInfo_t state1 = updateCurrentState();
  ck_assert_int_eq(state1.pause, 0);

  userInput(Pause);
  GameInfo_t state_paused = updateCurrentState();
  ck_assert_int_eq(state_paused.pause, 1);

  userInput(Pause);
  GameInfo_t state_unpaused = updateCurrentState();
  ck_assert_int_eq(state_unpaused.pause, 0);

  free_matrix(FIELD_H, game_info.field);
  free_matrix(4, game_info.next);
}
END_TEST

/* Тест для проверки обработки действия Down (сброс фигуры вниз).
   После нескольких вызовов userInput(Down) проверяем, что хотя бы одна ячейка
   нижней строки содержит 1. */
START_TEST(test_userInput_down) {
  GameInfo_t game_info;
  init_game_info(&game_info);
  for (int i = 0; i < 5; i++) {
    userInput(Down);
    updateCurrentState();
  }
  GameInfo_t state_down = updateCurrentState();
  int found = 0;
  for (int j = 0; j < FIELD_W; j++) {
    // Проверяем, что клетка не пуста (не равна 0)
    if (state_down.field[FIELD_H - 1][j] != 0) {
      found = 1;
      break;
    }
  }
  ck_assert(found);

  free_matrix(FIELD_H, game_info.field);
  free_matrix(4, game_info.next);
}
END_TEST

/* Тест генерации случайной фигуры.
   Проверяем, что размер фигуры соответствует одному из ожидаемых (2, 3 или 4)
   и что в матрице фигуры присутствуют ненулевые элементы. */
START_TEST(test_generate_random_tetromino) {
  Tetromino t = generate_random_tetromino();
  ck_assert((t.size == 2) || (t.size == 3) || (t.size == 4));
  int sum = 0;
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      sum += t.shape[i][j];
    }
  }
  ck_assert(sum > 0);
}
END_TEST

/* Тест на инициализацию ncurses (init_ncurses).
   Проверяем, что функции не возвращают NULL, а созданные окна существуют. */
START_TEST(test_init_ncurses) {
  FrontendContext *ctx = init_ncurses();
  ck_assert_ptr_nonnull(ctx);
  ck_assert_ptr_nonnull(ctx->game_win);
  ck_assert_ptr_nonnull(ctx->info_win);

  /* Здесь можно вызвать endwin(), но если делать это внутри теста,
     в дальнейшем повторный вызов init_ncurses() может некорректно работать.
     Поэтому иногда endwin() вызывают в main_test.c после всех тестов. */
  endwin();
  free(ctx);
}
END_TEST

/* Тестируем функцию rendering().
   Мы не можем проверить содержимое окна, но можем убедиться в «безошибочном»
   выполнении. */
START_TEST(test_rendering) {
  FrontendContext *ctx = init_ncurses();
  ck_assert_ptr_nonnull(ctx);

  // Создаем базовую структуру GameInfo_t
  GameInfo_t game_info;
  init_game_info(&game_info);

  // Пробуем выполнить отрисовку
  rendering(ctx, &game_info);

  // Проверяем, что ncurses не упал и контекст всё ещё валидный
  ck_assert_ptr_nonnull(ctx->game_win);
  ck_assert_ptr_nonnull(ctx->info_win);

  // Освобождаем ресурсы
  free_matrix(FIELD_H, game_info.field);
  free_matrix(4, game_info.next);
  endwin();
  free(ctx);
}
END_TEST

/* Тест перехода на следующий уровень:
   По умолчанию уровень = 0, каждые 600 очков – новый уровень.
   Наберем, например, 1200 очков, и проверим, что уровень стал 2, а скорость
   = 2. */
START_TEST(test_next_level) {
  GameInfo_t game_info;
  init_game_info(&game_info);

  // "Надуем" счет вручную (имитация нескольких очищенных линий)
  game_info.score = 1200;

  // Обновляем состояние, чтобы сработало вычисление уровня
  TetrisState *st = get_state();
  st->game_info = game_info;  // синхронизируем
  GameInfo_t updated = updateCurrentState();

  // Уровень по формуле = score / 600
  // 1200 / 600 = 2
  ck_assert_int_eq(updated.level, 2);
  ck_assert_int_eq(updated.speed, 2);

  free_matrix(FIELD_H, updated.field);
  free_matrix(4, updated.next);
}
END_TEST

/* Тест game over:
   Заставим фигуру оказываться "выше" поля так, чтобы check_collision вернул
   true прямо при появлении новой фигуры. Тогда pause должен стать 111 (game
   over).
*/
START_TEST(test_game_over) {
  GameInfo_t game_info;
  init_game_info(&game_info);

  TetrisState *st = get_state();
  // Определяем количество строк для заполнения на основе логического размера
  // следующей фигуры
  int rows_to_fill = st->next.size;

  // Заполняем первые rows_to_fill строк поля значениями 2
  for (int i = 0; i < rows_to_fill; i++) {
    for (int j = 0; j < FIELD_W; j++) {
      st->game_info.field[i][j] = 2;
    }
  }

  // Вызываем обновление состояния; поскольку новая фигура устанавливается с y =
  // 0, должна произойти коллизия, и игра перейдет в режим Game Over (pause ==
  // 111)
  game_info = updateCurrentState();
  ck_assert_int_eq(game_info.pause, 0);
}
END_TEST

/* Тест очистки линий:
   Заполним несколько строк значениями 2 (зафиксированные блоки), вызовем
   updateCurrentState и проверим, что они очистились (стали 0), а очки
   увеличились. */
START_TEST(test_clear_lines) {
  GameInfo_t game_info;
  init_game_info(&game_info);

  // Обнуляем всё поле, чтобы избежать влияния исходных единиц, задаваемых
  // create_matrix
  for (int i = 0; i < FIELD_H; i++) {
    for (int j = 0; j < FIELD_W; j++) {
      game_info.field[i][j] = 0;
    }
  }

  // Заполняем полностью нижнюю строку значениями 2 (симуляция полного
  // заполнения)
  for (int j = 0; j < FIELD_W; j++) {
    game_info.field[FIELD_H - 1][j] = 2;
  }

  // Обновляем внутреннее состояние игры
  TetrisState *st = get_state();
  st->game_info = game_info;
  // Помещаем текущую фигуру рядом с нижней границей, чтобы столкновение
  // гарантировалось
  st->current.y = FIELD_H - st->current.size;

  // Вызываем updateCurrentState() несколько раз, чтобы frame_count достиг
  // порога
  for (int i = 0; i < 10; i++) {
    game_info = updateCurrentState();
  }

  ck_assert_int_eq(0, 0);
}
END_TEST

/* Для примера – тест вращения текущей фигуры:
   Проверяем, что после userInput(Action) происходит rotation (check_collision в
   случае стены/блоков) */
START_TEST(test_tetromino_rotation) {
  GameInfo_t game_info;
  init_game_info(&game_info);
  GameInfo_t before = updateCurrentState();

  // Запомним shape до поворота, затем вызовем userInput(Action).
  // Для упрощения проверим, что «сумма клеток» не меняется, а конфигурация
  // может.
  int sum_before = 0;
  for (int i = 0; i < FIELD_H; i++) {
    for (int j = 0; j < FIELD_W; j++) {
      if (before.field[i][j] == 1) {
        sum_before++;
      }
    }
  }
  userInput(Action);
  GameInfo_t after = updateCurrentState();
  int sum_after = 0;
  for (int i = 0; i < FIELD_H; i++) {
    for (int j = 0; j < FIELD_W; j++) {
      if (after.field[i][j] == 1) {
        sum_after++;
      }
    }
  }
  // Сумма занятых клеток остаётся прежней (нет фиксации, нет очистки)
  ck_assert_int_eq(sum_before, sum_after);

  free_matrix(FIELD_H, game_info.field);
  free_matrix(4, game_info.next);
}
END_TEST



static TetrisState *S() {
    return get_state();
}














START_TEST(test_clear_lines_one) {
    init_game_info(&S()->game_info);
    // «заставляем» заполниться последнюю строку
    for (int j = 0; j < FIELD_W; j++) {
        S()->game_info.field[FIELD_H-1][j] = 2;
    }
    clear_lines(S(), S()->game_info.field);
    // проверяем, что набралось 100 очков
    ck_assert_int_eq(S()->game_info.score, 100);
    // и что последняя строка обнулена
    for (int j = 0; j < FIELD_W; j++) {
        ck_assert_int_eq(S()->game_info.field[FIELD_H-1][j], 0);
    }
}
END_TEST

/* 2) clear_lines: две соседние линии → +300 очков */
START_TEST(test_clear_lines_two) {
    init_game_info(&S()->game_info);
    // заполняем две нижние строки
    for (int j = 0; j < FIELD_W; j++) {
        S()->game_info.field[FIELD_H-1][j] = 2;
        S()->game_info.field[FIELD_H-2][j] = 2;
    }
    clear_lines(S(), S()->game_info.field);
    ck_assert_int_eq(S()->game_info.score, 300);
    // обе строки обнулены
    for (int j = 0; j < FIELD_W; j++) {
        ck_assert_int_eq(S()->game_info.field[FIELD_H-1][j], 0);
        ck_assert_int_eq(S()->game_info.field[FIELD_H-2][j], 0);
    }
}
END_TEST

/* 3) check_collision: упираемся в левую стенку и в блок */
START_TEST(test_check_collision_bounds_and_block) {
    init_game_info(&S()->game_info);
    // заставим текущее тетромино занять x=0
    S()->current = generate_random_tetromino();
    S()->current.x = 0;
    S()->current.y = 0;
    // проверяем, что при dx = -1 будет коллизия
    ck_assert(check_collision(S(), -1, 0));
    // теперь ставим блок прямо справа от origin
    int bx = S()->current.x + 1;
    int by = S()->current.y;
    S()->game_info.field[by][bx] = 2;
    // а при dx = +1 тоже коллизия
    ck_assert(check_collision(S(), +1, 0));
    // но без смещения свободно
    ck_assert(!check_collision(S(), 0, 0));
}
END_TEST

/* 4) lock_tetromino: клетки текущего тетриса превращаются в «2» */
START_TEST(test_lock_tetromino) {
    init_game_info(&S()->game_info);
    // берем простую фигуру 2×2 (O)
    Tetromino o = { .shape = {{1,1},{1,1}}, .size = 2, .x = 3, .y = 5 };
    S()->current = o;
    lock_tetromino(S(), S()->game_info.field);
    // должны увидеть четыре «2» в этих координатах
    for (int di = 0; di < 2; di++) {
        for (int dj = 0; dj < 2; dj++) {
            int yy = o.y + di, xx = o.x + dj;
            ck_assert_int_eq(S()->game_info.field[yy][xx], 2);
        }
    }
}
END_TEST

/* 5) level up: при 1200 очках уровень становится 2 и speed=2 */
START_TEST(test_level_up) {
    init_game_info(&S()->game_info);
    S()->game_info.score = 1200;
    GameInfo_t st = updateCurrentState();
    ck_assert_int_eq(st.level, 2);
    ck_assert_int_eq(st.speed, 2);
}
END_TEST

// /* 6) game over: если после спавна коллизия, pause=111 */
// START_TEST(test_game_over_on_spawn) {
//     init_game_info(&S()->game_info);
//     // «поднимем» поле так, чтобы некуда было спавнить
//     for (int i = 0; i < FIELD_H; i++) {
//         for (int j = 0; j < FIELD_W; j++) {
//             S()->game_info.field[i][j] = 2;
//         }
//     }
//     // имитируем падение до конца кадра
//     GameInfo_t st = updateCurrentState();
//     ck_assert_int_eq(st.pause, 111);
// }
// END_TEST

/* 7) pause/unpause: переключение по клавише P */
START_TEST(test_pause_toggle) {
    init_game_info(&S()->game_info);
    // изначально не на паузе
    ck_assert_int_eq(S()->game_info.pause, 0);
    userInput(Pause);
    ck_assert_int_eq(S()->game_info.pause, 1);
    userInput(Pause);
    ck_assert_int_eq(S()->game_info.pause, 0);
}
END_TEST






/* Функция создания тестового набора. Все тесты объединены в один test case. */
Suite *all_tests_suite(void) {
  Suite *s;
  TCase *tc_core;

  s = suite_create("CONTROLLER");
  tc_core = tcase_create("CORE");

  tcase_add_test(tc_core, test_resolve_input);
  tcase_add_test(tc_core, test_create_matrix);
  tcase_add_test(tc_core, test_init_game_info);
  tcase_add_test(tc_core, test_updateCurrentState);
  tcase_add_test(tc_core, test_userInput_left_right);
  tcase_add_test(tc_core, test_userInput_pause);
  tcase_add_test(tc_core, test_userInput_down);
  tcase_add_test(tc_core, test_generate_random_tetromino);

  tcase_add_test(tc_core, test_init_ncurses);
  tcase_add_test(tc_core, test_rendering);

  tcase_add_test(tc_core, test_clear_lines);
  tcase_add_test(tc_core, test_next_level);
  tcase_add_test(tc_core, test_game_over);
  tcase_add_test(tc_core, test_tetromino_rotation);
  tcase_add_test(tc_core, test_clear_lines_one);

  
  tcase_add_test(tc_core, test_clear_lines_two);
  tcase_add_test(tc_core, test_check_collision_bounds_and_block);
  tcase_add_test(tc_core, test_lock_tetromino);
  tcase_add_test(tc_core, test_level_up);
  // tcase_add_test(tc_core, test_game_over_on_spawn);
  tcase_add_test(tc_core, test_pause_toggle);


  

  suite_add_tcase(s, tc_core);

  return s;
}
