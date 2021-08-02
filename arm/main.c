#include <stdarg.h>
#include <defines_cdfj.h>
#include <advland_helpers.h>
#include "shared_defines.h"

// adventureland functions
void initAdvland();
void stepAdvland();
void welcome();

// local functions
void initialise();
void setDataStreams();
void addToInputString(int key);
int updateScr(const char c, const int x, const int y);
int printChar(const char c);
void nextOutputLine();

// number of scanlines in a glyph
#define LINE_HEIGHT  5

// the number of scanlines between character rows
#define LINE_SPACING  1

// how many glyphs can we pack into a data stream
#define GLYPHS_PER_COLUMN 2

// width of screen in characters
#define CHARS_PER_ROW _NUM_DATASTREAMS * GLYPHS_PER_COLUMN

// height of screen in characters
#define CHARS_PER_COL _SCANLINES_IN_DATASTREAM / (LINE_HEIGHT + LINE_SPACING)

// the amount to adjust the ASCII value by to correctly index the
// glyphs array
const int upperCaseAdj = 65;
const int lowerCaseAdj = 97;
const int numberAdj = 22;

// some ASCII characters need to be converted to specific glyphs. we're not
// grouping these in the same order as the ASCII table although arguably that
// would make for easier conversion - requiring the code to explicitly map the
// ASCII to the glyph makes for clearer code IMO
const int spaceChar = 36;
const int dotChar = 37;
const int commaChar = 38;
const int apostropheChar = 39;
const int ampersandChar = 40;
const int hyphenChar = 41;
const int asteriskChar = 42;
const int questionMarkChar = 43;
const int openParenChar = 44;
const int closeParenChar = 45;

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
	0,0,0,0,0, // space
	0,0,0,2,2, // dot
	0,0,0,2,4, // comma
	2,4,0,0,0, // apostrophe
	0,2,7,2,0, // ampersand
	0,0,7,0,0, // hyphen
	0,2,5,2,0, // asterisk
	7,1,2,0,2, // question mark
	1,2,2,2,1, // open parenthesis
	4,2,2,2,4, // close parenthesis
};

// the location on the screen for the next output character
int outputX; 
int outputY;

// maximum number of characters in the input string. note that this has to
// CHARS_PER_ROW (or less I suppose) in order for text rendering to work as
// intended
#define MAX_INPUT_CHARS CHARS_PER_ROW

// the input string returned by gets(), which is called by stepAdvland()
char input[MAX_INPUT_CHARS];
int inputIdx = 0;

// previous key to be pressed. we use this to prevent key repetition.
int prevInputKey = -1;

// indicates whether stepAdvland() should be called on the next VBLANK
int commitInput = 0;

int main() {
	switch (RAM[_RUN_FUNC]) {
		case _FN_INIT:
			initialise();
			setDataStreams();
			nextOutputLine();
			break;
		case _FN_GAME_VB:
			{
				int key;
				key = RAM[_INPUT_KEY];
				if (key != prevInputKey && key > 0) {
					addToInputString(key);
				}
				prevInputKey = key;

				if (commitInput == 1) {
					// three newlines before continuing 
					nextOutputLine();
					nextOutputLine();
					nextOutputLine();

					stepAdvland();
					commitInput = 0;
					nextOutputLine();
				}
				setDataStreams();
			}
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

void setDataStreams() {
	for (int i = 0; i < _NUM_DATASTREAMS; i ++ ) {
		int addr;
		addr = _DATASTREAMS_ORIGIN + (i*_SCANLINES_IN_DATASTREAM);
		setPointer(_DATASTREAM_BASE_REG+i, addr);
	}
}

void addToInputString(int key) {
	// hash key. commit input
	if (key == 12) {
		commitInput = 1;
		return;
	}

	// asterisk key. delete last character
	if (key == 10) {
		if (inputIdx > 0) {
			inputIdx --;
		}
		input[inputIdx] = ' ';
	} else {
		if (inputIdx >= MAX_INPUT_CHARS) {
			return;
		}

		if (key >= 1 && key <= 9) {
			// 48 is the ASCII value for 1
			input[inputIdx] = key+48;
		} else if (key == 11) {
			input[inputIdx] = '0';
		}

		inputIdx++;
	}

	// print current input string
	for (int x = 0; x < inputIdx; x ++) {
		updateScr(input[x], x, outputY+1);
	}

	// blank rest of line with space char
	for (int x = inputIdx; x < MAX_INPUT_CHARS; x ++) {
		updateScr(' ', x, outputY+1);
	}
}

// add a character to the screen buffer at the character x/y position
//
// returns:
//	0 if character was not printed
//	1 if it was
//	2 for newline
int updateScr(const char c, const int x, const int y) {
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
	} else if (c == '(' || c == '[' || c == '{') {
		g = openParenChar;
	} else if (c == ')' || c == ']' || c == '}') {
		g = closeParenChar;
	} else if (c == '\0') {
		return 0;
	}

	int sl = y * (LINE_HEIGHT+LINE_SPACING);

	for (int l = 0; l < LINE_HEIGHT; l++) {
		// bit pattern for this scanline of glyph
		int b = glyphs[l+(g*LINE_HEIGHT)];

		if ((x&0x01) == 0x00) {
			int addr = _DATASTREAMS_ORIGIN + ((x>>1)*_SCANLINES_IN_DATASTREAM);
			RAM[addr + sl] &= 0xf0;
			RAM[addr + sl] |= b << 1; 
		} else {
			int addr = _DATASTREAMS_ORIGIN + (((x>>1)+1)*_SCANLINES_IN_DATASTREAM);
			RAM[addr + sl] &= 0x0f;
			RAM[addr + sl] |= b << 5; 
		}

		sl++;
	}

	return 1;
}

// printChar adds a character to the output stream using current outputX and
// outputY values as arugments to updateScr().
//
// advances outputX and outputY as appropriate (see nextOutputLine() function).
//
// returns 1 if character was printed and 0 if not. if return value is 1 then
// outputX and outputY values will have changed.
int printChar(const char c) {
	int v = updateScr(c, outputX, outputY);

	switch (v) {
		case 0:
			// no output
			return 0;

		case 1:
			// normal output
			outputX++;
			if (outputX >= CHARS_PER_ROW) {
				nextOutputLine();
			}
			break;

		case 2:
			// newline character
			nextOutputLine();
			break;
	}

	return 1;
}

void _printf(const char * s, ...) {
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
				continue; // while loop (will cause loop to finished immediately)
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
	for (int i = _DATASTREAMS_ORIGIN; i < _DATASTREAMS_MEMTOP; i++) {
		RAM[i] = 0;
	}
	outputY = 0;
	outputX = 0;
}

int _toupper(int c) {
	if (c >= 97 && c <= 122) {
		return c-32;
	}
	return c;
}

int _getch() {
	return 0;
}

char * _gets(char *s){
	int i;
	for (i = 0; i < inputIdx; i ++) {
		s[i] = input[i];

		// blank input string as we go along
		input[i] = ' ';
	}
	s[i] = '\0';
	inputIdx = 0;
	return s;
}

int kbhit(){
	return 0;
}

int wherex(){
	return outputX;
}

unsigned int _rand() {
	return getRandom32();
}

void nextOutputLine() {
	outputY++;
	outputX = 0;
	if (outputY >= CHARS_PER_COL-1) {
		clrscr();
	}
}
