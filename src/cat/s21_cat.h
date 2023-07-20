#ifndef S21_CAT_H
#define S21_CAT_H

#include <getopt.h>
#include <stdio.h>

typedef struct options_s {
  unsigned char number : 1;
  unsigned char nonblank : 1;
  unsigned char squeeze_blank : 1;
  unsigned char show_ends : 1;
  unsigned char show_tabs : 1;
  unsigned char show_nonprinting : 1;
} options_t;

options_t options;

char set_options(int argc, char* argv[]);
void cat(int argc, char* argv[]);
#endif