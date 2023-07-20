#ifndef S21_GREP_H
#define S21_GREP_H

#include <getopt.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct options_s {
  regex_t* regexps;
  int regexps_size;
  unsigned char ignore_case;
  unsigned char invert_match : 1;
  unsigned char count : 1;
  unsigned char files_with_matches : 1;
  unsigned char line_number : 1;
  unsigned char no_filename : 1;
  unsigned char no_messages : 1;
  unsigned char only_matching : 1;
} options_t;

typedef struct extended_file_s {
  FILE* file;
  char* filename;
} extended_file_t;

options_t options;
char** patterns;
char** files;
int patterns_size;
int files_size;

void free_regexps();
char* get_string(FILE* stream);
char set_options(int argc, char* argv[]);
char set_regexps(char* command);
void grep(int argc, char* argv[]);
#endif