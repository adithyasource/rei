#include <ctype.h>
#include <dirent.h>
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

int fuzzy_find(char needle[], char haystack[]) {
  for (int i = 0; i < strlen(needle); i++) {
    int found_character = 0;
    for (int j = 0; j < strlen(haystack); j++) {
      if (tolower(needle[i]) == tolower(haystack[j])) {
        found_character = 1;
        break;
      }
    }
    if (found_character == 0) {
      return 0;
    }
  }

  return 1;
}

void append_string(char ***array, int *last_index, char *new_string) {
  *array = realloc(*array, (*last_index + 1) * sizeof(char *));

  (*array)[*last_index] = malloc(strlen(new_string) + 1);
  strcpy((*array)[*last_index], new_string);

  (*last_index)++;
}

void get_haybales(char ***haybales, int *haybale_last_index, char directory[],
                  WINDOW *inputwin, WINDOW *outputwin) {
  struct dirent *de;
  DIR *dr = opendir(directory);

  while ((de = readdir(dr)) != NULL) {
    wrefresh(outputwin);
    if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) {
      continue;
    }
    if (de->d_type == 4) {
      char child_path[4096];
      snprintf(child_path, sizeof(child_path), "%s/%s", directory, de->d_name);
      get_haybales(haybales, haybale_last_index, child_path, inputwin,
                   outputwin);
    } else {
      if (strcmp(directory, "./") != 0) {
        char *chopped_dir = directory + 3;
        char full_path[4096];
        snprintf(full_path, sizeof(full_path), "%s/%s", chopped_dir,
                 de->d_name);
        append_string(haybales, haybale_last_index, full_path);
      } else {
        append_string(haybales, haybale_last_index, de->d_name);
      }
    }
  }

  closedir(dr);
}

void render_output_list(char query[], int haybale_last_index, char **haybale,
                        int highlight_index, WINDOW *inputwin,
                        WINDOW *outputwin) {
  int found_needle = 0;

  char **results = NULL;

  for (int i = 0; i < haybale_last_index; i++) {
    if (fuzzy_find(query, haybale[i]) == 1) {
      append_string(&results, &found_needle, haybale[i]);
      found_needle += 1;
    }
  }

  for (int i = 0; i < found_needle; i++) {
    if (results[i] == NULL)
      continue;

    if (highlight_index == i) {
      wattron(outputwin, COLOR_PAIR(1));
    }
    wprintw(outputwin, "%s\n", results[i]);
    if (highlight_index == i) {
      wattroff(outputwin, COLOR_PAIR(1));
    }
    free(results[i]);
  }
  free(results);

  wrefresh(outputwin);

  if (found_needle == 0) {
    wprintw(outputwin, "no files match your query");
    wrefresh(outputwin);
  }

  wmove(inputwin, 0, strlen(query));
}

void choose_result(char query[], int haybale_last_index, char **haybale,
                   int highlight_index, WINDOW *inputwin) {
  int found_needle = 0;

  char **results = NULL;

  for (int i = 0; i < haybale_last_index; i++) {
    if (fuzzy_find(query, haybale[i]) == 1) {
      append_string(&results, &found_needle, haybale[i]);
      found_needle += 1;
    }
  }

  for (int i = 0; i < found_needle; i++) {
    if (results[i] == NULL)
      continue;

    if (highlight_index == i) {
      endwin();

      printf("%s\n", results[i]);
      fflush(stdout);

      for (int i = 0; i < haybale_last_index; i++) {
        free(haybale[i]);
      }
      free(haybale);

      exit(0);
    }

    free(results[i]);
  }
  free(results);
}

int main() {
  initscr();
  start_color();

  init_pair(1, COLOR_BLACK, COLOR_WHITE);

  /* getting current window width and height */
  int term_width, term_height;
  getmaxyx(stdscr, term_height, term_width);

  /* creating input and output windows */
  WINDOW *inputwin = newwin(3, term_width - 12, 1, 3);
  WINDOW *outputwin = newwin(term_height - 8, term_width - 12, 5, 3);
  /*scrollok(outputwin, TRUE);*/
  intrflush(inputwin, FALSE);
  keypad(inputwin, TRUE);

  /* display all files when query empty  */
  char query[100];
  int c;

  char **haybale = NULL;

  int haybale_last_index = 0;

  get_haybales(&haybale, &haybale_last_index, "./", inputwin, outputwin);

  int highlight_index = 0;

  for (int i = 0; i < haybale_last_index; i++) {
    if (highlight_index == i) {
      wattron(outputwin, COLOR_PAIR(1));
    }
    wprintw(outputwin, "%s\n", haybale[i]);
    if (highlight_index == i) {
      wattroff(outputwin, COLOR_PAIR(1));
    }
  }
  wrefresh(outputwin);

  while ((c = wgetch(inputwin))) {
    if (c == 10) {
      /* enter key */
      choose_result(query, haybale_last_index, haybale, highlight_index,
                    inputwin);
    } else {

      switch (c) {
      case (127):
        /* backspace key */
        werase(inputwin);
        query[strlen(query) - 1] = '\0';
        wprintw(inputwin, "%s", query);
        highlight_index = 0;
        break;

      case (KEY_UP):
        if (highlight_index <= 0)
          break;
        highlight_index -= 2;
        break;

      case (KEY_DOWN):
        highlight_index += 2;
        break;

      default:
        strncat(query, (char *)&c, 1);
        highlight_index = 0;
        break;
      }

      wclear(outputwin);

      render_output_list(query, haybale_last_index, haybale, highlight_index,
                         inputwin, outputwin);
    }
  }

  for (int i = 0; i < haybale_last_index; i++) {
    free(haybale[i]);
  }
  free(haybale);

  endwin();

  return 0;
}
