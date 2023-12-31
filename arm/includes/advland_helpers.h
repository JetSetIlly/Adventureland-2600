#include <stdlib.h>

// conio.h replacements
void clrscr();
int kbhit();
int wherex();

// stdlib replacements
void _printf(const char *, ...);
char * _gets(char *);
int _getch();
int _toupper(int);
unsigned int _rand();

// notify that game has ended
void endgame();
