# Tetris on C

<p align="center">
  Консольная версия «Тетриса» на C с разделением игровой логики и интерфейса.
</p>

<p align="center">
  <img alt="C" src="https://img.shields.io/badge/C-00599C?logo=c&logoColor=white">
  <img alt="ncurses" src="https://img.shields.io/badge/UI-ncurses-2E8B57">
  <img alt="Tests" src="https://img.shields.io/badge/Tests-Unit%20tests-2E8B57">
</p>

Игровая логика расположена в **brick_game/tetris/**, консольный интерфейс — в **gui/cli/**. В репозитории также есть тесты backend-логики и изображение **tetris.png**.

## Сборка и запуск

Нужны GCC, GNU Make и библиотека ncurses.

~~~bash
make
make run
~~~

## Тесты

~~~bash
make test
~~~

Дополнительные цели Makefile включают очистку сборки, установку, документацию Doxygen и отчёт покрытия.