#include <stdarg.h>
#include <defines_cdfj.h>
#include <advland_helpers.h>
#include "shared_defines.h"
#include "shared_ram.h"

// adventureland functions
void initAdvland();
void stepAdvland();

// local functions
void initialise();
void repoint();
int printCharAt(const char c, const int x, const int y);
int printChar(const char c);

// the maximum number of characters that can be be displayed on a line
const int charsPerRow = 20;
const int charsPerCol = 32;
const int lineSpacing = 1;

// number of scanlines in a glyph
const int lineHeight = 5;

// the amount to adjust the ASCII value by to correctly index the
// glyphs array
const int upperCaseAdj = 65;
const int lowerCaseAdj = 97;
const int numberAdj = 21;

// some ASCII characters need to be converted to specific glyphs
const int spaceChar = 26;
const int dotChar = 37;
const int commaChar = 38;
const int apostropheChar = 39;
const int ampersandChar = 40;
const int hyphenChar = 41;
const int asteriskChar = 42;
const int questionMarkChar = 43;

const unsigned char glyphs[] = {
	2,5,7,5,5, // A
	6,5,6,5,6, 
	3,4,4,4,3,
	6,5,5,5,6,
	7,4,6,4,7,
	7,4,6,4,4,
	3,4,4,5,7,
	5,5,7,5,5,
	7,2,2,2,7,
	1,1,1,5,7,
	5,5,6,5,5,
	4,4,4,4,7, // L
	5,7,5,5,5,
	6,5,5,5,5,
	2,5,5,5,2,
	7,5,6,4,4,
	2,5,5,7,3,
	6,5,6,5,5,
	7,4,7,1,7,
	7,2,2,2,2,
	5,5,5,5,7,
	5,5,5,5,2,
	5,5,5,7,5, 
	5,2,2,2,5,
	5,5,7,2,2,
	7,0,2,4,7,
	0,0,0,0,0, // space
	7,5,5,5,7, // digits (0 to 9)
	2,6,2,2,7,
	7,1,7,4,7,
	7,1,3,1,7,
	5,5,7,1,1,
	7,4,6,1,6,
	4,4,7,5,7,
	7,1,1,1,1,
	7,5,7,5,7,
	7,5,7,1,1,
	0,0,0,2,2, // dot
	0,0,0,2,4, // comma
	2,4,0,0,0, // apostrophe
	0,2,7,2,0, // ampersand
	0,0,7,0,0, // hyphen
	2,5,0,5,2, // asterisk
	7,1,2,0,2, // question mark
};

// the location on the screen for the next character
int col; // max chars per col
int row; // max chars per row

// the previous character to be printed
char prevChar;

// state of program between calls to main()
int prevFireButton = 0;
int scriptCt = 0;

int main() {
	switch (RAM[_RUN_FUNC]) {
		case _FN_INIT:
			initialise();
			repoint();
			break;
		case _FN_GAME_VB:
			if (prevFireButton != RAM[_FIREBUTTON]) {
				prevFireButton = RAM[_FIREBUTTON];
				if ((RAM[_FIREBUTTON] & 0x80) == 0x00 ) {
					scriptCt++;
					switch (scriptCt) {
						case 1:
							_printf("\nGO E\n");
							break;

						case 3:
							_printf("\nGO E\n");
							break;

						case 5:
							_printf("\nTAKE AXE\n");
							break;

						case 7:
							_printf("\nINV\n");
							break;

						default:
							stepAdvland();
							break;
					}
				}
			}

			repoint();
			break;
		case _FN_GAME_OS:
			break;
	}
}

void initialise() {
	// clear memory
    myMemsetInt(RAM_INT, 0, 4096/4);

	// set datastream increments to one
    for(int i = 0; i <= 34; i++) {
        setIncrement(i, 1, 0);
	}

	initAdvland();
}

// repoint the data streams
void repoint() {
    setPointer(_DS_COL0, _COL0_DATA);
    setPointer(_DS_COL1, _COL1_DATA);
    setPointer(_DS_COL2, _COL2_DATA);
    setPointer(_DS_COL3, _COL3_DATA);
    setPointer(_DS_COL4, _COL4_DATA);
    setPointer(_DS_COL5, _COL5_DATA);
    setPointer(_DS_COL6, _COL6_DATA);
    setPointer(_DS_COL7, _COL7_DATA);
    setPointer(_DS_COL8, _COL8_DATA);
    setPointer(_DS_COL9, _COL9_DATA);
}

// returns:
//	0 if character was not printed
//	1 if it was
//	2 for newline
int printCharAt(const char c, const int x, const int y) {
	char g = spaceChar;

	// convert char to glyph
	if (c == '\n') {
		return 2;
	}

	if (c == ' ') {
		g = spaceChar;
	} else if (c >= '0' && c <= '9') {
		g = c-numberAdj;
	} else if (c >= 'A' && c <= 'Z') {
		g = c-upperCaseAdj;
	} else if (c >= 'a' && c <= 'z') {
		g = c-lowerCaseAdj;
	} else if (c == '.') {
		g = dotChar;
	} else if (c == ',') {
		g = commaChar;
	} else if (c == '\'') {
		g = apostropheChar;
	} else if (c == '&') {
		g = ampersandChar;
	} else if (c == '-') {
		g = hyphenChar;
	} else if (c == '*') {
		g = asteriskChar;
	} else if (c == '?') {
		g = questionMarkChar;
	} else if (c == '\0') {
		return 0;
	}

	int sl = y * (lineHeight+lineSpacing);

	for (int l = 0; l < lineHeight; l++) {
		// bit pattern for this row of glyph
		int b = glyphs[l+(g*lineHeight)];

		switch (x) {
			case 0:
				RAM[_COL0_DATA + sl] &= 0x0f;
				RAM[_COL0_DATA + sl] |= b << 5; 
				break;
			case 1:
				RAM[_COL0_DATA + sl] &= 0xf0;
				RAM[_COL0_DATA + sl] |= b << 1; 
				break;

			case 2:
				RAM[_COL1_DATA + sl] &= 0x0f;
				RAM[_COL1_DATA + sl] |= b << 5; 
				break;
			case 3:
				RAM[_COL1_DATA + sl] &= 0xf0;
				RAM[_COL1_DATA + sl] |= b << 1; 
				break;
		}

		switch (x) {
			case 4:
				RAM[_COL2_DATA + sl] &= 0x0f;
				RAM[_COL2_DATA + sl] |= b << 5; 
				break;
			case 5:
				RAM[_COL2_DATA + sl] &= 0xf0;
				RAM[_COL2_DATA + sl] |= b << 1; 
				break;

			case 6:
				RAM[_COL3_DATA + sl] &= 0x0f;
				RAM[_COL3_DATA + sl] |= b << 5; 
				break;
			case 7:
				RAM[_COL3_DATA + sl] &= 0xf0;
				RAM[_COL3_DATA + sl] |= b << 1; 
				break;
		}

		switch (x) {
			case 8:
				RAM[_COL4_DATA + sl] &= 0x0f;
				RAM[_COL4_DATA + sl] |= b << 5; 
				break;
			case 9:
				RAM[_COL4_DATA + sl] &= 0xf0;
				RAM[_COL4_DATA + sl] |= b << 1; 
				break;

			case 10:
				RAM[_COL5_DATA + sl] &= 0x0f;
				RAM[_COL5_DATA + sl] |= b << 5; 
				break;
			case 11:
				RAM[_COL5_DATA + sl] &= 0xf0;
				RAM[_COL5_DATA + sl] |= b << 1; 
				break;
		}


		switch (x) {
			case 12:
				RAM[_COL6_DATA + sl] &= 0x0f;
				RAM[_COL6_DATA + sl] |= b << 5; 
				break;
			case 13:
				RAM[_COL6_DATA + sl] &= 0xf0;
				RAM[_COL6_DATA + sl] |= b << 1; 
				break;

			case 14:
				RAM[_COL7_DATA + sl] &= 0x0f;
				RAM[_COL7_DATA + sl] |= b << 5; 
				break;
			case 15:
				RAM[_COL7_DATA + sl] &= 0xf0;
				RAM[_COL7_DATA + sl] |= b << 1; 
				break;
		}

		switch (x) {
			case 16:
				RAM[_COL8_DATA + sl] &= 0x0f;
				RAM[_COL8_DATA + sl] |= b << 5; 
				break;
			case 17:
				RAM[_COL8_DATA + sl] &= 0xf0;
				RAM[_COL8_DATA + sl] |= b << 1; 
				break;

			case 18:
				RAM[_COL9_DATA + sl] &= 0x0f;
				RAM[_COL9_DATA + sl] |= b << 5; 
				break;
			case 19:
				RAM[_COL9_DATA + sl] &= 0xf0;
				RAM[_COL9_DATA + sl] |= b << 1; 
				break;
		}

		sl++;
	}

	// record previous character
	prevChar = c;

	return 1;
}

// returns 1 if character was printed and 0 if not. if return value is 1 then
// col and row values will have changed.
int printChar(const char c) {
	int v = printCharAt(c, col, row);

	switch (v) {
		case 0:
			// no output
			return 0;

		case 1:
			// normal output
			col++;
			if (col >= charsPerRow) {
				col = 0;
				row++;
				if (row >= charsPerCol) {
					clrscr();
				}
			}
			break;

		case 2:
			// newline character
			col = 0;
			row++;
			if (row >= charsPerCol) {
				clrscr();
			}
			break;
	}

	return 1;
}

void _printf(const char * s, ...)
{
	va_list ap;
	va_start(ap, s);

	// complex placeholders require a little bit more work
	int placeholderForce = 0;
	int fieldWidth = -1;
	int skipFieldWidthReset = 0;

	while (*s != '\0') {
		if (placeholderForce == 1 || *s == '%') {
			// reset placeholderForce flag or advance pointer. the pointer
			// should be in the correct place if placeholderForce is 'true'
			if (placeholderForce == 1) {
				placeholderForce = 0;
			} else {
				s++;
			}

			if (*s == '\0') {
				continue; // while loop
			} else if (*s == 's' || *s == 'S') {
				if (fieldWidth == -1) {
					for (char *p = va_arg(ap, char *); *p; p++) {
						printChar(*p);
					}
				} else {
					int i;
					char *p;
					for (i = 0, p = va_arg(ap, char *); i < fieldWidth && *p; i++, p++) {
						printChar(*p);
					}
				}
			} else if (*s == 'c' || *s == 'C') {
				// chars are promoted to int when passed in a vararg
				printChar((char)va_arg(ap, int));
			} else if (*s == 'd' || *s == 'D' || *s == 'u' || *s == 'U') {
				int d;
				d = va_arg(ap, int);
				if (d >= 0 && d <= 9) {
					printChar(d + 48);
				} else {
					printChar('&');
				}
			} else if (*s == '.') {
				// field width modifier
				s++;
				if (*s == '\0') {
					continue; // while loop
				} else if (*s == '*') {
					fieldWidth = va_arg(ap, int);
					skipFieldWidthReset = 1;
					placeholderForce = 1;
				} else {
					placeholderForce = 1;
				}
			} else {
				printChar('&');
			}
		} else {
			printChar(*s);
		}

		s++;

		if (skipFieldWidthReset == 0) {
			fieldWidth = -1;
		} else {
			skipFieldWidthReset = 0;
		}
	}

	va_end(ap);
}

void clrscr() {
	for (int x = 0; x < charsPerRow; x++) {
		for (int y = 0; y < charsPerCol; y++) {
			printCharAt(' ', x, y);
		}
	}

	row = 0;
	col = 0;
	prevChar = '\0';
}

int _toupper(int c) {
	if (c >= 97 && c <= 122) {
		return c-32;
	}
	return c;
}

int getch() {
	return 0;
}

char * _gets(char *s){
	switch (scriptCt) {
		case 2:
			s[0] = 'g';
			s[1] = 'o';
			s[2] = ' ';
			s[3] = 'e';
			s[4] = '\0';
			break;

		case 4:
			s[0] = 'g';
			s[1] = 'o';
			s[2] = ' ';
			s[3] = 'e';
			s[4] = '\0';
			break;

		case 6:
			s[0] = 't';
			s[1] = 'a';
			s[2] = 'k';
			s[3] = 'e';
			s[4] = ' ';
			s[5] = 'a';
			s[6] = 'x';
			s[7] = 'e';
			s[8] = '\0';
			break;

		case 8:
			s[0] = 'i';
			s[1] = 'n';
			s[2] = 'v';
			s[3] = '\0';
			break;
	
		default:
			s[0] = '\0';
			break;
	}

	return s;
}

int kbhit(){
	return 0;
}

int wherex(){
	return col;
}

unsigned int _rand() {
	return getRandom32();
}
