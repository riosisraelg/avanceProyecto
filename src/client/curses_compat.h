#ifndef CURSES_COMPAT_H
#define CURSES_COMPAT_H

#ifdef _WIN32
    #include <curses.h>   /* PDCurses */
#else
    #include <ncurses.h>  /* ncurses en POSIX */
#endif

#endif /* CURSES_COMPAT_H */
