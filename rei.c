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

void append_string(char ***haybales, int *haybale_last_index, char *full_path) {
  *haybales = realloc(*haybales, (*haybale_last_index + 1) * sizeof(char *));

  (*haybales)[*haybale_last_index] = malloc(strlen(full_path) + 1);
  strcpy((*haybales)[*haybale_last_index], full_path);

  (*haybale_last_index)++;
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

int main() {
  initscr();

  /* getting current window width and height */
  int term_width, term_height;
  getmaxyx(stdscr, term_height, term_width);

  /* creating input and output windows */
  WINDOW *inputwin = newwin(3, term_width - 12, 1, 3);
  WINDOW *outputwin = newwin(term_height - 8, term_width - 12, 5, 3);
  scrollok(outputwin, TRUE);

  /* display all files when query empty  */

  char query[100], c;

  char **haybale = NULL;

  int haybale_last_index = 0;

  get_haybales(&haybale, &haybale_last_index, "./", inputwin, outputwin);

  for (int i = 0; i < haybale_last_index; i++) {
    wprintw(outputwin, "%s\n", haybale[i]);
  }
  wrefresh(outputwin);

  while ((c = wgetch(inputwin)) != '\n') {
    if (c == 127) {
      werase(inputwin);
      query[strlen(query) - 1] = '\0';
      wprintw(inputwin, "%s", query);
    } else {
      strncat(query, &c, 1);
    }
    wclear(outputwin);

    int found_needle = 0;

    for (int i = 0; i < haybale_last_index; i++) {
      if (fuzzy_find(query, haybale[i]) == 1) {
        found_needle = 1;
        wprintw(outputwin, "%s\n", haybale[i]);
      }
    }

    wrefresh(outputwin);

    if (found_needle == 0) {
      wprintw(outputwin, "no files match your query");
      wrefresh(outputwin);
    }

    wmove(inputwin, 0, strlen(query));
  }

  for (int i = 0; i < haybale_last_index; i++) {
    free(haybale[i]);
  }
  free(haybale);

  endwin();

  return 0;
}
