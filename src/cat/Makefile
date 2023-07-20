CC=gcc
CFLAGS=-Wall -Werror -Wextra -std=c11 -pedantic

all: s21_cat

s21_cat:
	$(CC) $(CFLAGS) s21_cat.c -o s21_cat

clean:
	rm -rf s21_cat

check:
	cppcheck --enable=all --suppress=missingIncludeSystem *.[ch]
	clang-format -n *.[ch]