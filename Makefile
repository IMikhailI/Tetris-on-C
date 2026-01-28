# куда устанавливать — директория src рядом с Makefile
BINDIR ?= build

TARGET = game
VERSION = 1.0

# Имя итоговой библиотеки
LIBRARY = tetris.a

# Основные исходники проекта (адаптируйте пути, если нужно)
SRCS = brick_game.c brick_game/tetris/backend.c gui/cli/frontend.c
OBJS = $(SRCS:.c=.o)

# Тестовые объектные файлы, но без main (brick_game.o),
# чтобы избежать multiple definition of main
TEST_MODS = brick_game/tetris/backend.o gui/cli/frontend.o

TEST_SRCS = tests/main_test.c tests/all_tests.c
TEST_OBJS = $(TEST_SRCS:.c=.o)
TEST_EXEC = main_test

# Компилятор и флаги
CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2
LDFLAGS = -lncurses

# Собираем все заголовки/исходники для использования в clang-формате
ALL_SRC = $(wildcard */*.h */*/*.h */*/*/*.h */*.c */*/*.c */*/*/*.c)

.PHONY: all clean install uninstall dvi dist test gcov_report clang-n clang-i

# ------------------------- Основная сборка -------------------------
all: $(LIBRARY) $(TARGET)

# Рецепт для сборки библиотеки
$(LIBRARY): $(LIB_OBJS)
	@echo "Archiving static library $@ ..."
	ar rcs $@ $^
	ranlib $@

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# ---------------------------- Clean ---------------------------------
clean:
	rm -f $(OBJS) $(LIBRARY) $(TARGET) $(TEST_OBJS) $(TEST_EXEC)
	rm -rf coverage_report doxygen
	rm -f *.info *.gcda *.gcno */*.gcda */*.gcno */*/*.gcda */*/*.gcno

# ---------------------------- Install -------------------------------
install: all
	@echo "Устанавливаем $(TARGET) в $(BINDIR)..."
	# создаём каталог, если ещё нет
	install -d $(BINDIR)
	# копируем туда исполняемый файл
	install -m 0755 $(TARGET) $(BINDIR)/$(TARGET)

uninstall:
	@echo "Удаляем $(TARGET) из $(DESTDIR)$(BINDIR)..."
	rm -f $(DESTDIR)$(BINDIR)/$(TARGET)

run:
	./game

# -------------------------- Документация ----------------------------
dvi:
	doxygen
	xdg-open doxygen/html/index.html

# --------------------------- Dist -----------------------------------
dist:
	rm -f $(TARGET)-$(VERSION).tar.gz
	tar czf $(TARGET)-$(VERSION).tar.gz \
	    brick_game.c brick_game.h \
	    brick_game/tetris/backend.c brick_game/tetris/backend.h \
	    gui/cli/frontend.c gui/cli/frontend.h \
	    tests/main_test.c tests/controller_test.c tests/backend_test.c \
	    README_RUS.md Makefile

# --------------------------- Test -----------------------------------
test: $(TEST_EXEC)
	./$(TEST_EXEC)

$(TEST_EXEC): $(TEST_OBJS) $(TEST_MODS)
	$(CC) $(CFLAGS) -o $(TEST_EXEC) $(TEST_OBJS) $(TEST_MODS) -lcheck -lm -lsubunit -lncurses

# ----------------------- Gcov coverage -----------------------------
gcov_report: CFLAGS += -fprofile-arcs -ftest-coverage
gcov_report: CFLAGS += -fprofile-arcs -ftest-coverage
gcov_report: clean test
	@mkdir -p coverage_report

	lcov --capture \
	     --directory . \
	     --output-file coverage.info

	# убираем только свои тесты и всё из /usr
	lcov --remove coverage.info \
	     '*/tests/*' \
	     '/usr/*' \
	     --output-file coverage_filtered.info

	genhtml --no-branch-coverage \
	        --output-directory coverage_report \
	        coverage_filtered.info

	xdg-open coverage_report/index.html




# ---------------------- Clang-format -------------------------------
clang-n:
	clang-format -n --style=google $(ALL_SRC)
	rm -f .clang-format

clang-i:
	clang-format -i --style=google $(ALL_SRC)
	rm -f .clang-format

# ------------------------ Valgrind ---------------------------------
val:
	valgrind --tool=memcheck --track-origins=yes --leak-check=yes ./game