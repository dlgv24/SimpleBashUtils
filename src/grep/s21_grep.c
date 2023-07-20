#include "s21_grep.h"

void free_regexps() {
  for (int i = 0; i < options.regexps_size; i++) {
    regfree(options.regexps + i);
  }
  free(options.regexps);
  options.regexps_size = 0;
}

char* get_string(FILE* stream) {
  int string_length = 0;
  char ch = 0, *string = NULL, *tmp;
  while ((ch = getc(stream)) != EOF && ch != '\n') {
    if (string_length % 10 == 0) {
      tmp = (char*)realloc(string, (string_length + 10) * sizeof(char));
      string = tmp;
    }
    string[string_length++] = ch;
  }
  tmp = (char*)realloc(string, (string_length + 1) * sizeof(char));
  string = tmp;
  string[string_length] = 0;
  return string;
}

char set_options(int argc, char* argv[]) {
  const struct option long_options[] = {{"regexp", 1, 0, 'e'},
                                        {"ignore-case", 0, 0, 'i'},
                                        {"invert-match", 0, 0, 'v'},
                                        {"count", 0, 0, 'c'},
                                        {"files-with-matches", 0, 0, 'l'},
                                        {"line-number", 0, 0, 'n'},
                                        {"no-filename", 0, 0, 'h'},
                                        {"no-messages", 0, 0, 's'},
                                        {"file", 1, 0, 'f'},
                                        {"only-matching", 0, 0, 'o'},
                                        {0, 0, 0, 0}};
  options.ignore_case = REG_EXTENDED;
  char error = 0, **tmp;
  int long_option = -1, option = 0;
  while ((option = getopt_long(argc, argv, "e:ivclnhsf:o", long_options,
                               &long_option)) != -1 &&
         !error) {
    if (option == 'e') {
      tmp = (char**)realloc(patterns, (patterns_size + 1) * sizeof(char*));
      patterns = tmp;
      patterns[patterns_size++] = optarg;
    } else if (option == 'i')
      options.ignore_case = REG_ICASE;
    else if (option == 'v')
      options.invert_match = 1;
    else if (option == 'c')
      options.count = 1;
    else if (option == 'l')
      options.files_with_matches = 1;
    else if (option == 'n')
      options.line_number = 1;
    else if (option == 'h')
      options.no_filename = 1;
    else if (option == 's')
      options.no_messages = 1;
    else if (option == 'f') {
      tmp = (char**)realloc(files, (files_size + 1) * sizeof(char*));
      files = tmp;
      files[files_size++] = optarg;
    } else if (option == 'o')
      options.only_matching = 1;
    else if (option == '?')
      error = 1;
  }
  if (patterns == NULL && files == NULL) {
    patterns = (char**)realloc(patterns, (patterns_size + 1) * sizeof(char*));
    patterns[patterns_size++] = argv[optind++];
  }
  if (!patterns && !files) error = 1;
  if (error) fprintf(stderr, "Usage: grep [OPTION]... PATTERNS [FILE]...\n");
  return error;
}

char set_regexps(char* command) {
  char error = 0;
  for (int i = 0; i < patterns_size && !error; i++) {
    options.regexps_size++;
    options.regexps = (regex_t*)realloc(
        options.regexps, (options.regexps_size) * sizeof(regex_t));
    int errcode = regcomp(options.regexps + options.regexps_size - 1,
                          patterns[i], options.ignore_case);
    if (errcode) {
      int errlen = regerror(errcode, options.regexps + options.regexps_size - 1,
                            NULL, 0);
      char errmsg[errlen];
      regerror(errcode, options.regexps + options.regexps_size - 1, errmsg,
               errlen);
      fprintf(stderr, "%s: %s", command, errmsg);
      error = 1;
    }
  }
  free(patterns);
  for (int i = 0; i < files_size && !error; i++) {
    FILE* file = fopen(files[i], "r");
    if (file == NULL) {
      fprintf(stderr, "%s: %s: No such file or directory\n", command, files[i]);
      error = 1;
    } else {
      int line_number = 1;
      while (!feof(file) && !error) {
        char* string = get_string(file);
        options.regexps_size++;
        options.regexps = (regex_t*)realloc(
            options.regexps, (options.regexps_size) * sizeof(regex_t));
        int errcode = regcomp(options.regexps + options.regexps_size - 1,
                              string, options.ignore_case);
        if (errcode) {
          int errlen = regerror(
              errcode, options.regexps + options.regexps_size - 1, NULL, 0);
          char errmsg[errlen];
          regerror(errcode, options.regexps + options.regexps_size - 1, errmsg,
                   errlen);
          fprintf(stderr, "%s:%d: %s", command, line_number++, errmsg);
          error = 1;
        }
        free(string);
      }
      fclose(file);
    }
  }
  free(files);
  return error;
}

regmatch_t* get_matches(int* nmatch, regex_t* regex, char* string) {
  *nmatch = 0;
  regmatch_t *pmatch = NULL, *tmp;
  int result = 0;
  while (!result) {
    result = regexec(regex, string, 0, NULL, 0);
    if (!result) {
      tmp = (regmatch_t*)realloc(pmatch, sizeof(regmatch_t) * (*nmatch + 1));
      pmatch = tmp;
      regexec(regex, string, 1, pmatch + *nmatch, 0);
      string += pmatch[*nmatch].rm_eo;
      (*nmatch)++;
    }
    if (!options.only_matching && !result) {
      result = 1;
    }
  }
  for (int i = 1; i < *nmatch; i++) {
    pmatch[i].rm_so += pmatch[i - 1].rm_eo;
    pmatch[i].rm_eo += pmatch[i - 1].rm_eo;
  }
  return pmatch;
}

void grep(int argc, char* argv[]) {
  int streams_size = argc - optind;
  extended_file_t streams[streams_size ? streams_size : 1];
  if (streams_size) {
    for (int i = 0; i < streams_size; i++) {
      streams[i].file = fopen(argv[i + optind], "r");
      streams[i].filename = argv[i + optind];
    }
  } else {
    streams[0].file = stdin;
    char standard_input[] = "(standard input)";
    streams[0].filename = standard_input;
    streams_size = 1;
  }

  if (streams_size == 1) {
    options.no_filename = 1;
  }

  for (int i = 0; i < streams_size; i++) {
    if (streams[i].file != NULL) {
      int count = 0, line_number = 1;
      char flag = 1;
      while (!feof(streams[i].file) && flag) {
        char* string = get_string(streams[i].file);
        if (strcmp(string, "") == 0 && feof(streams[i].file)) {
          free(string);
          break;
        }
        int nmatch = 0;
        regmatch_t* pmatch = NULL;
        for (int j = 0; j < options.regexps_size && !nmatch; j++) {
          pmatch = get_matches(&nmatch, options.regexps + j, string);
        }

        if (nmatch) {
          if (!options.invert_match) {
            count++;
          }
        } else if (options.invert_match) {
          count++;
        }

        if (options.files_with_matches && nmatch) {
          flag = 0;
        }
        if (!options.files_with_matches && !options.count) {
          if (!nmatch && options.invert_match) {
            if (!options.no_filename) {
              printf("%s:", streams[i].filename);
            }
            if (options.line_number) {
              printf("%d:", line_number);
            }
            printf("%s\n", string);
          }
          if (nmatch && !options.invert_match) {
            if (options.only_matching) {
              if (!options.no_filename) {
                printf("%s:", streams[i].filename);
              }
              if (options.line_number) {
                printf("%d:", line_number);
              }
              for (int j = 0; j < nmatch; j++) {
                printf("%.*s\n", (int)(pmatch[j].rm_eo - pmatch[j].rm_so),
                       string + pmatch[j].rm_so);
              }
            } else {
              if (!options.no_filename) {
                printf("%s:", streams[i].filename);
              }
              if (options.line_number) {
                printf("%d:", line_number);
              }
              printf("%s\n", string);
            }
          }
        }
        line_number++;
        if (pmatch != NULL) free(pmatch);
        free(string);
      }
      if (options.count) {
        if (!options.no_filename) {
          printf("%s:", streams[i].filename);
        }
        printf("%d\n", count);
      }
      if (options.files_with_matches) {
        if (count) {
          printf("%s\n", streams[i].filename);
        }
      }
      fclose(streams[i].file);
    } else {
      if (!options.no_messages)
        fprintf(stderr, "%s: %s: No such file or directory\n", argv[0],
                streams[i].filename);
    }
  }
}

int main(int argc, char* argv[]) {
  if (!set_options(argc, argv)) {
    if (!set_regexps(argv[0])) {
      grep(argc, argv);
    }
  }

  free_regexps();
  return 0;
}
