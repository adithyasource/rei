#include <ctype.h>
#include <dirent.h>
#include <ncurses.h>
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

void fuzzy_find_directory(char query[], char directory[], int *found_needle,
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
      fuzzy_find_directory(query, child_path, found_needle, inputwin,
                           outputwin);
    } else {
      if (fuzzy_find(query, de->d_name)) {
        *found_needle = 1;
        if (strcmp(directory, "./") != 0) {
          char *chopped_dir = directory + 3;
          wprintw(outputwin, "%s/%s\n", chopped_dir, de->d_name);
        } else {
          wprintw(outputwin, "%s\n", de->d_name);
        }
        wmove(inputwin, 0, strlen(query));
        wrefresh(outputwin);
      }
    }
  }

  closedir(dr);
}

void render_all_files(char directory[], WINDOW *inputwin, WINDOW *outputwin) {

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
      render_all_files(child_path, inputwin, outputwin);
    } else {
      if (strcmp(directory, "./") != 0) {
        char *chopped_dir = directory + 3;
        wprintw(outputwin, "%s/%s\n", chopped_dir, de->d_name);
      } else {
        wprintw(outputwin, "%s\n", de->d_name);
      }
      wrefresh(outputwin);
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
  render_all_files("./", inputwin, outputwin);

  char query[100], c;

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

    fuzzy_find_directory(query, "./", &found_needle, inputwin, outputwin);

    if (found_needle == 0) {
      wprintw(outputwin, "no files match your query");
      wmove(inputwin, 0, strlen(query));
      wrefresh(outputwin);
    }
  }

  endwin();

  return 0;
}
