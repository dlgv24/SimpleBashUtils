#include "s21_cat.h"

char set_options(int argc, char* argv[]) {
  char error = 0;
  const struct option long_options[] = {{"number-nonblank", 0, 0, 'b'},
                                        {"show-ends", 0, 0, 'E'},
                                        {"number", 0, 0, 'n'},
                                        {"squeeze-blank", 0, 0, 's'},
                                        {"show-tabs", 0, 0, 'T'},
                                        {"show-nonprinting", 0, 0, 'v'},
                                        {0, 0, 0, 0}};
  opterr = 0;
  int long_option = -1;
  int option;
  while ((option = getopt_long(argc, argv, "beEnstTv", long_options,
                               &long_option)) != -1 &&
         !error) {
    if (option == 'b') {
      options.number = 1;
      options.nonblank = 1;
    } else if (option == 'e') {
      options.show_ends = 1;
      options.show_nonprinting = 1;
    } else if (option == 'E') {
      options.show_ends = 1;
    } else if (option == 'n') {
      options.number = 1;
    } else if (option == 's') {
      options.squeeze_blank = 1;
    } else if (option == 't') {
      options.show_tabs = 1;
      options.show_nonprinting = 1;
    } else if (option == 'T') {
      options.show_tabs = 1;
    } else if (option == 'v') {
      options.show_nonprinting = 1;
    } else if (option == '?') {
      if (optopt)
        fprintf(stderr, "%s: invalid option -- '%c'\n", argv[0], optopt);
      else
        fprintf(stderr, "%s: unrecognized option '%s'\n", argv[0],
                argv[optind - 1]);
      error = 1;
    }
  }
  return error;
}

void cat(int argc, char* argv[]) {
  int streams_count = argc - optind;
  FILE* streams[streams_count ? streams_count : 1];
  if (streams_count) {
    for (int i = 0; i < streams_count; i++) {
      streams[i] = fopen(argv[i + optind], "r");
    }
  } else {
    streams[0] = stdin;
    streams_count = 1;
  }
  unsigned char ch3 = 0, ch2 = 0, ch1, ch1_flag, ind_flag = 1;
  for (int i = 0; i < streams_count; i++) {
    if (streams[i] != NULL) {
      int ind = 1;
      while ((ch1 = fgetc(streams[i])) != (unsigned char)EOF) {
        ch1_flag = 1;
        if (options.squeeze_blank) {
          if ((ch3 == '\n' || ch3 == 0) && ch2 == '\n' && ch1 == '\n') {
            ch1_flag = 0;
            ind_flag = 0;
          }
        }
        if (options.number) {
          if (ind_flag) {
            if (!(options.nonblank && (ch2 == '\n' || ch2 == 0) && ch1 == '\n'))
              printf("%6d\t", ind++);
            ind_flag = 0;
          }
          if (ch1 == '\n') ind_flag = 1;
        }
        if (options.show_ends && ch1_flag && ch1 == '\n') {
          printf("$\n");
          ch1_flag = 0;
        }
        if (options.show_tabs && ch1_flag && ch1 == '\t') {
          printf("^I");
          ch1_flag = 0;
        }
        if (options.show_nonprinting && ch1_flag && ch1 != '\n' &&
            ch1 != '\t') {
          if (ch1 >= 32) {
            if (ch1 < 127)
              putchar(ch1);
            else if (ch1 == 127)
              printf("^?");
            else {
              printf("M-");
              if (ch1 >= 160) {
                if (ch1 < 255)
                  putchar(ch1 - 128);
                else
                  printf("^?");
              } else
                printf("^%c", ch1 - 64);
            }
          } else
            printf("^%c", ch1 + 64);
          ch1_flag = 0;
        }
        if (ch1_flag) putchar(ch1);
        ch3 = ch2;
        ch2 = ch1;
      }
      fclose(streams[i]);
    } else {
      fprintf(stderr, "%s: %s: No such file or directory\n", argv[0],
              argv[i + optind]);
    }
  }
}

int main(int argc, char* argv[]) {
  if (!set_options(argc, argv)) cat(argc, argv);
  return 0;
}