#include <stdarg.h>
#include <defines_cdfj.h>
#include <advland_helpers.h>
#include "glyphs.h"
#include "shared_defines.h"

// adventureland functions
void initAdvland();
void stepAdvland();
void welcome();

// local functions
void initialise();
void setDataStreams();
int updateInputString(int key);
void printInputString();
void printInputCursor();
int updateScr(const char c, const int x, const int y);
void printOutputChar(const char c);
void flushWordBuffer();
void nextOutputLine();
void scrollscr();

// number of scanlines in a glyph
#define LINE_HEIGHT  5

// the number of scanlines between character rows
#define LINE_SPACING  1

// how many glyphs can we pack into a data stream
#define GLYPHS_PER_COLUMN 2

// width of screen in characters
#define CHARS_PER_ROW _NUM_DATASTREAMS * GLYPHS_PER_COLUMN

// height of screen in characters
#define CHARS_PER_DATASTREAM _SCANLINES_IN_DATASTREAM / (LINE_HEIGHT + LINE_SPACING)

// the location on the screen (in characters) for the next output character
int outputX; 
int outputY;

// maximum number of characters in the input string. note that this has to be
// CHARS_PER_ROW (or less) in order for text rendering to work as intended
//
// the -1 is for the prompt character
#define MAX_INPUT_CHARS CHARS_PER_ROW-1

// previous key pressed and sent as an argument to the VBLANK program. we use
// this to prevent key repetition.
int prevInputKey = -1;

// the input string returned by gets(), which is called by stepAdvland()
char input[MAX_INPUT_CHARS];

// the index to be used next on key input. this is also the position of the
// cursor for display purposes
int inputIdx = 0;

// the last key pressed. distinct from prevInputKey. we use this to cycle
// through available key options in the group.
int keyGroup = -1;

// the next option in the keyGroup to use. -1 indicates that nothing has yet
// been done with the current cursor position. 
int keyGroupOpt = -1;

// the number of times the VBLANK program has been executed. we'll use this to
// create the flashing cursor.
int runCountVBLANK = 0;

// the maximum number of characters to buffer before flushing to screen. is
// flushed at whitespace or when the buffer size is reached.
char wordBuffer[CHARS_PER_ROW];
int wordBufferIdx = 0;

int main() {
	switch (RAM[_RUN_FUNC]) {
		case _FN_INIT:
			initialise();
			setDataStreams();
			nextOutputLine();
			break;
		case _FN_GAME_VB:
			{
				runCountVBLANK++;

				int commit = 0;
				int key = RAM[_INPUT_KEY];

				if (key != prevInputKey && key > 0) {
					commit = updateInputString(key);
					printInputString();
				}
				prevInputKey = key;

				if (commit) {
					// three newlines before continuing 
					nextOutputLine();
					nextOutputLine();
					nextOutputLine();

					stepAdvland();
					nextOutputLine();
				}
				setDataStreams();

				printInputCursor();
			}
			break;
	}
}

void initialise() {
	// clear memory
    myMemsetInt(RAM_INT, 0, 4096/4);

	// set datastream increments to one
    for (int i = 0; i <= 34; i++) {
        setIncrement(i, 1, 0);
	}

	// initialise input string to spaces
	for (int i = 0; i < MAX_INPUT_CHARS; i ++) {
		input[i] = ' ';
	}
	inputIdx = 0;

	// initialise output buffer string to spaces
	for (int i = 0; i < CHARS_PER_ROW; i ++) {
		wordBuffer[i] = ' ';
	}
	wordBufferIdx = 0;

	initAdvland();
}

void setDataStreams() {
	for (int i = 0; i < _NUM_DATASTREAMS; i ++ ) {
		int addr;
		addr = _DATASTREAMS_ORIGIN + (i*_SCANLINES_IN_DATASTREAM);
		setPointer(_DATASTREAM_BASE_REG+i, addr);
	}
}

// returns 1 if input is to be committed
int updateInputString(int key) {
	// hash key. commit input
	if (key == 12) {
		keyGroup = -1;
		keyGroupOpt = -1;
		return 1;
	}

	// asterisk key. delete last character
	if (key == 10) {
		keyGroup = -1;
		keyGroupOpt = -1;
		if (inputIdx > 0) {
			inputIdx --;
			input[inputIdx] = ' ';
		}
		return 0;
	}

	// no input past maximum length
	if (inputIdx >= MAX_INPUT_CHARS) {
		return 0;
	}

	// commit last key group
	if (key == 1) {
		keyGroup = -1;
		keyGroupOpt = -1;
		return 0;
	}
	
	// update key group
	if (key != keyGroup) {
		// start new opt sequence
		keyGroupOpt = 0;
	} else if (inputIdx > 0) {
		// if this key is in the same key group as previous key then overwrite
		// the previous input
		inputIdx--;
	}
	keyGroup = key;

	// most key groups have 3 options but some have a different number. we'll
	// update and use this value to cycle the 
	int numOpts = 2;

	switch (keyGroup) {
		case 2:
			switch (keyGroupOpt) {
				case 0:
					input[inputIdx] = 'A';
					break;
				case 1:
					input[inputIdx] = 'B';
					break;
				case 2:
					input[inputIdx] = 'C';
					break;
			}
			break;
		case 3:
			switch (keyGroupOpt) {
				case 0:
					input[inputIdx] = 'D';
					break;
				case 1:
					input[inputIdx] = 'E';
					break;
				case 2:
					input[inputIdx] = 'F';
					break;
			}
			break;
		case 4:
			switch (keyGroupOpt) {
				case 0:
					input[inputIdx] = 'G';
					break;
				case 1:
					input[inputIdx] = 'H';
					break;
				case 2:
					input[inputIdx] = 'I';
					break;
			}
			break;
	}
	switch (keyGroup) {
		case 5:
			switch (keyGroupOpt) {
				case 0:
					input[inputIdx] = 'J';
					break;
				case 1:
					input[inputIdx] = 'K';
					break;
				case 2:
					input[inputIdx] = 'L';
					break;
			}
			break;
		case 6:
			switch (keyGroupOpt) {
				case 0:
					input[inputIdx] = 'M';
					break;
				case 1:
					input[inputIdx] = 'N';
					break;
				case 2:
					input[inputIdx] = 'O';
					break;
			}
			break;
		case 7:
			switch (keyGroupOpt) {
				case 0:
					input[inputIdx] = 'P';
					break;
				case 1:
					input[inputIdx] = 'Q';
					break;
				case 2:
					input[inputIdx] = 'R';
					break;
				case 3:
					input[inputIdx] = 'S';
					break;
			}
			numOpts = 3;
			break;
	}
	switch (keyGroup) {
		case 8:
			switch (keyGroupOpt) {
				case 0:
					input[inputIdx] = 'T';
					break;
				case 1:
					input[inputIdx] = 'U';
					break;
				case 2:
					input[inputIdx] = 'V';
					break;
			}
			break;
		case 9:
			switch (keyGroupOpt) {
				case 0:
					input[inputIdx] = 'W';
					break;
				case 1:
					input[inputIdx] = 'X';
					break;
				case 2:
					input[inputIdx] = 'Y';
					break;
				case 3:
					input[inputIdx] = 'Z';
					break;
			}
			numOpts = 3;
			break;

		// case 10 has been handled (backspace)
		
		case 11:
			keyGroup = -1;
			keyGroupOpt = -1;

			// don't allow space as the first character in the input and do not
			// allow consecutive spaces
			if (inputIdx == 0 || input[inputIdx-1] == ' ') {
				return 0;
			}

			input[inputIdx] = ' ';
			inputIdx++;
			return 0;

		// case 12 has been handled (return/commit)
	}

	// prepare next key option
	keyGroupOpt++;
	if (keyGroupOpt > numOpts) {
		keyGroupOpt = 0;
	}

	inputIdx++;

	return 0;
}

void printInputString() {
	// print current input string
	for (int x = 0; x < inputIdx; x ++) {
		updateScr(input[x], x, outputY+1);
	}

	// blank rest of line with space char
	for (int x = inputIdx; x < CHARS_PER_ROW; x ++) {
		updateScr(' ', x, outputY+1);
	}
}

void printInputCursor() {
	int cursor = inputIdx;

	// highlight character being edited  
	if (cursor > 0 && keyGroupOpt != -1) {
		cursor--;
	}

	if ((runCountVBLANK & 0x08) == 0x08) {
		updateScr(0xff, cursor, outputY+1);
	} else {
		updateScr(input[cursor], cursor, outputY+1);
	}
}

// add a character to the screen buffer at the character x/y position
//
// returns:
//	0 if character was not printed
//	1 if it was
//	2 for newline
int updateScr(const char c, const int x, const int y) {
	int g = spaceGlyph;

	// convert char to glyph
	if (c == '\n') {
		return 2;
	}

	// don't print out of range
	if (x >= CHARS_PER_ROW) {
		return 0;
	}

	// default to symbolsGlyphs. this will be changed appropriately depending
	// on the character to be output
	const unsigned char *glyphs = symbolsGlyphs;

	if (c == ' ') {
		g = spaceGlyph;
	} else if (c >= '0' && c <= '9') {
		glyphs = digitsGlyphs;
		g = c-numberAdj;
	} else if (c >= 'A' && c <= 'Z') {
		glyphs = upperCaseGlyphs;
		g = c-upperCaseAdj;
	} else if (c >= 'a' && c <= 'z') {
		glyphs = upperCaseGlyphs;
		g = c-lowerCaseAdj;
	} else if (c == '.') {
		g = dotGlyph;
	} else if (c == ',') {
		g = commaGlyph;
	} else if (c == '\'') {
		g = apostropheGlyph;
	} else if (c == '&') {
		g = ampersandGlyph;
	} else if (c == '-') {
		g = hyphenGlyph;
	} else if (c == '*') {
		g = asteriskGlyph;
	} else if (c == '?') {
		g = questionMarkGlyph;
	} else if (c == '(' || c == '[' || c == '{') {
		g = openParenGlyph;
	} else if (c == ')' || c == ']' || c == '}') {
		g = closeParenGlyph;
	} else if (c == '_') {
		g = underscoreGlyph;
	} else if (c == 0xff) {
		g = cursorGlyph;
	} else if (c == '\0') {
		return 0;
	}

	int sl = y * (LINE_HEIGHT+LINE_SPACING);

	for (int l = 0; l < LINE_HEIGHT; l++) {
		// bit pattern for this scanline of glyph
		int b = glyphs[l+(g*LINE_HEIGHT)];

		if ((x&0x01) == 0x01) {
			int addr = _DATASTREAMS_ORIGIN + ((x>>1)*_SCANLINES_IN_DATASTREAM);
			RAM[addr + sl] &= 0xf0;
			RAM[addr + sl] |= b << 1; 
		} else {
			int addr = _DATASTREAMS_ORIGIN + (((x>>1))*_SCANLINES_IN_DATASTREAM);
			RAM[addr + sl] &= 0x0f;
			RAM[addr + sl] |= b << 5; 
		}

		sl++;
	}

	return 1;
}

// printOutputChar adds a character to the output stream using current outputX
// and outputY values as arugments to updateScr().
//
// advances outputX and outputY as appropriate (see nextOutputLine() function).
void printOutputChar(const char c) {
	wordBuffer[wordBufferIdx] = c;
	wordBufferIdx++;

	// flush buffer on whitespace or when buffer is full
	if (c == '\n' || c == ' ' || c == '\t' || wordBufferIdx >= CHARS_PER_ROW) {
		flushWordBuffer();
	}
}

void flushWordBuffer() {
	// if word cannot fit on the current line, insert a newline before
	// continuing
	if (outputX + wordBufferIdx >= CHARS_PER_ROW) {
		nextOutputLine();
	}

	for (int i = 0; i < wordBufferIdx; i++) {
		int v = updateScr(wordBuffer[i], outputX, outputY);
		switch (v) {
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
	}

	wordBufferIdx = 0;
}


// _printf should only be used for output from the engine
void _printf(const char * s, ...) {
	va_list ap;
	va_start(ap, s);

	// complex placeholders require a little bit more work
	int placeholderForce = 0;
	int fieldWidth = -1;
	int skipFieldWidthReset = 0;

	// the previous character to be output. we use this to prevent double-spaces.
	char prevOutputChar = '\0';

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
						printOutputChar(*p);
					}
				} else {
					int i;
					char *p;
					for (i = 0, p = va_arg(ap, char *); i < fieldWidth && *p; i++, p++) {
						printOutputChar(*p);
					}
				}
			} else if (*s == 'c' || *s == 'C') {
				// chars are promoted to int when passed in a vararg
				printOutputChar((char)va_arg(ap, int));
			} else if (*s == 'd' || *s == 'D' || *s == 'u' || *s == 'U') {
				int d;
				d = va_arg(ap, int);
				if (d >= 0 && d <= 9) {
					printOutputChar(d + 48);
				} else {
					printOutputChar('&');
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
				printOutputChar('&');
			}
		} else {
			// prevent double-spaces
			if (!(*s == ' ' && prevOutputChar == ' ' && outputX > 0)) {
				printOutputChar(*s);
			}
			prevOutputChar = *s;
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
	// clear integers (32bit) instead of chars (8bit) for performance reasons.
	// size of memset therefore is a quarter of the datastreams size
	#define CLEAR_DATASTREAM _DATASTREAMS_SIZE / 4

	myMemsetInt((unsigned int *)RAM+_DATASTREAMS_ORIGIN-4, 0x00, CLEAR_DATASTREAM);
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
	// no blank line if output cursor is at the beginning of the screen
	if (outputY == 0 && outputX == 0) {
		return;
	}

	outputY++;
	outputX = 0;
	if (outputY >= CHARS_PER_DATASTREAM-1) {
		scrollscr();
	}
}

void scrollscr() {
	// scrolling so a quarter of the existing screen is visible. this is a good
	// value because we know that the size of the datastreams is a multiple of
	// four, so the division comes out even
	#define SCROLL_PRESERVE _SCANLINES_IN_DATASTREAM / 4

	// the copy distance within the datastream
	#define SCROLL_COPY_OFFSET _SCANLINES_IN_DATASTREAM - SCROLL_PRESERVE

	// we need to adjust outputY which is measure in characters rather than
	// scanlines
	#define SCROLL_CHARS CHARS_PER_DATASTREAM / 4

	// the layout of datastreams isn't ideal for scrolling so we must treat
	// each datastream separately
	for (int i = 0; i < _NUM_DATASTREAMS; i++) {
		int idx = _DATASTREAMS_ORIGIN + (i * _SCANLINES_IN_DATASTREAM);

		// copy bottom of datastream to top of datastream
		for (int j = 0; j < SCROLL_PRESERVE; j++) {
			RAM[idx+j] = RAM[idx+j+SCROLL_COPY_OFFSET];
		}

		// clear remainder of the datastream
		for (int j = SCROLL_PRESERVE; j < _SCANLINES_IN_DATASTREAM; j++) {
			RAM[idx+j] = 0x00;
		}
	}

	outputY = SCROLL_CHARS;
}
