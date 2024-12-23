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

int main() {

  initscr();

  /* getting current window width and height */
  int term_width, term_height;
  getmaxyx(stdscr, term_height, term_width);

  /* creating input and output windows */
  WINDOW *inputwin = newwin(3, term_width - 12, 1, 5);
  WINDOW *outputwin = newwin(term_height - 20, term_width - 12, 5, 5);

  /* display all files when query empty  */
  struct dirent *de;
  DIR *dr = opendir(".");
  while ((de = readdir(dr)) != NULL) {
    wprintw(outputwin, "%s\n", de->d_name);
    wrefresh(outputwin);
  }
  closedir(dr);

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

    struct dirent *de;
    DIR *dr = opendir(".");

    while ((de = readdir(dr)) != NULL) {
      if (fuzzy_find(query, de->d_name)) {
        wprintw(outputwin, "%s\n", de->d_name);
        wmove(inputwin, 0, strlen(query));
        wrefresh(outputwin);
      }
    }

    closedir(dr);
  }

  endwin();

  return 0;
}
