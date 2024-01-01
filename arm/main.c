// BSD 2-Clause License
// 
// Copyright (c) 2021, Stephen Illingworth
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <stdbool.h>
#include <stdarg.h>
#include <defines_cdfj.h>
#include <advland_helpers.h>
#include "glyphs.h"
#include "shared_defines.h"

// if gameEnded is set then the game has ended
bool gameEnded = false;

// minimum number of glyphs that can we pack into a data stream. the number may
// acutally be more because some gylphs are less than 4 pixels wide
#define CHARS_PER_COLUMN 2

// minimum width of screen in characters. the value is derived from the
// CHARS_PER_COLUMN value so the number of glyphs per rom may actually be more
// for the same reasons
#define CHARS_PER_TEXTAREA_ROW _NUM_TEXTAREA_DATASTREAMS * CHARS_PER_COLUMN

// number of rows in textarea
#define ROWS_PER_TEXTAREA _SCANLINES_IN_TEXTAREA / (LINE_HEIGHT + LINE_SPACING)

// the location on the screen (in characters) for the next output character
int drawX = 0; 
int drawY = 0;

// the data used to update the screen
unsigned char drawColumn[LINE_HEIGHT];
int drawPos = 0;

// the maximum number of characters to buffer before flushing to screen. is
// flushed at whitespace or when the buffer size is reached.
char wordBuffer[CHARS_PER_TEXTAREA_ROW];
int wordBufferIdx = 0;

// the number of characters in the line thus far. not the same as drawX
int lineX = 0;

// the previous character to be output. we use this to prevent double-spaces.
char prevOutputChar = '\0';


// the input string returned by gets(), which is called by the main advland
// code. the values of these variables are changed by the on screen keyboard
#define MAX_INPUT_CHARS 20
char input[MAX_INPUT_CHARS];
int inputIdx = 0;

// the index of the selected key on the keyboard. the maximum value is the upper
// case Z plus three. this is because we also want to draw a space, backspace
// and return key on the keyboard
#define minSelectedKey upperCaseAoffset
#define spaceKey upperCaseZoffset + 1
#define backspaceKey upperCaseZoffset + 2
#define returnKey upperCaseZoffset + 3
#define maxSelectedKey returnKey
int selectedKey = minSelectedKey;

// keyboard dimensions
#define keyboardWidth  6
#define keyboardHeight 5

// most recent input values from console
char SWCHA;
bool leftPlayerFire;
bool rightPlayerFire;

// adventureland functions
void initAdvland();
void stepAdvland();
void welcome();

// local functions
void initialise();
void setDataStreams();
int updateInputString(int key);
void drawKeyboardCoords(unsigned char c[LINE_HEIGHT], const int x, const int y);
void drawKeyboard();
void drawTextInputCoords(unsigned char c[LINE_HEIGHT], const int x);
void drawTextInput();
void drawTextAreaCoords(unsigned char c[LINE_HEIGHT], const int x, const int y);
void printTextArea(char text[], int length);
void flushWordBuffer();
void updateWordBuffer(const char c);
void nextOutputLine();
void scrollscr();
void quickCommand(char command[]); 
void submitCommand(); 

int main() {
	switch (RAM[_RUN_FUNC]) {
		case _FN_INIT:
			gameEnded = false;
			initialise();
			setDataStreams();
			break;
		case _FN_GAME:
		{
			if (gameEnded == true) {
				setDataStreams();
				return 0;
			}

			// whether to redraw the screen
			bool redrawScreen = false;

			// INPT4 (left player fire button)
			bool f = (RAM[_INPT4]&0x80) == 0x00;
			if (f == 0x00 && f != leftPlayerFire) {
				switch (selectedKey) {
				case spaceKey:
					if (inputIdx > 0 && inputIdx < MAX_INPUT_CHARS-1) {
						input[inputIdx] = ' ';
						inputIdx++;
						redrawScreen = true;
					}
					break;
				case backspaceKey:
					if (inputIdx > 0) {
						inputIdx--;
						input[inputIdx] = '\0';
						redrawScreen = true;
					}
					break;
				case returnKey:
					if (inputIdx > 0) {
						submitCommand();
						// not redrawing screen after submitCommand() because
						// the screen will be redrawn as a consequence of the
						// submission
					}
					break;
				default:
					if (inputIdx < MAX_INPUT_CHARS-1) {
						input[inputIdx] = selectedKey + asciiAdj;
						inputIdx++;
						redrawScreen = true;
					}
				}
			}
			leftPlayerFire = f;

			// right player fire button
			f = (RAM[_INPT5]&0x80) == 0x00;
			if (f == 0x00 && f != rightPlayerFire) {
		 		quickCommand("look"); 
				redrawScreen = true;
			}
			rightPlayerFire = f;

			// joysticks 
			if (RAM[_SWCHA] != SWCHA) {
				// SWCHA has changed (joystick direction for both players)
				SWCHA = RAM[_SWCHA];

				if ((SWCHA&0x80) == 0x00) {
					if (selectedKey < maxSelectedKey) {
						selectedKey++;
						redrawScreen = true;
					}
				} else if ((SWCHA&0x40) == 0x00) {
					if (selectedKey > minSelectedKey) {
						selectedKey--;
						redrawScreen = true;
					}
				} else if ((SWCHA&0x20) == 0x00) {
					if (selectedKey < maxSelectedKey-keyboardWidth) {
						selectedKey += keyboardWidth;
					} else {
						selectedKey = maxSelectedKey;
					}
					redrawScreen = true;
				} else if ((SWCHA&0x10) == 0x00) {
					if (selectedKey > minSelectedKey+keyboardWidth) {
						selectedKey -= keyboardWidth;
					} else {
						selectedKey = minSelectedKey;
					}
					redrawScreen = true;
				} else {
					// not redrawing screen after quickCommand() because
					// the screen will be redrawn as a consequence of the
					// submission
					if ((SWCHA&0x08) == 0x00) {
						quickCommand("go e");
					} else if ((SWCHA&0x04) == 0x00) {
						quickCommand("go w");
					} else if ((SWCHA&0x02) == 0x00) {
						quickCommand("go s");
					} else if ((SWCHA&0x01) == 0x00) {
						quickCommand("go n");
					}
				}
			}

			if (redrawScreen) {
				drawTextInput();
				drawKeyboard();
			}
			setDataStreams();
		}
		break;
	}

	return 0;
}

void initialise() {
	// clear memory
    myMemsetInt(RAM_INT, 0, 4096/4);

	// set datastream increments to one
    for (int i = 0; i <= 34; i++) {
        setIncrement(i, 1, 0);
	}

	// set textarea variables to inital values
	drawX = 0; 
	drawY = 0;
	drawPos = 0;
	wordBufferIdx = 0;
	lineX = 0;
	prevOutputChar = '\0';

	initAdvland();

	// set input/keyboard variable to initial values
	input[0] = '\0';
	inputIdx = 0;
	selectedKey = minSelectedKey;
	SWCHA = RAM[_SWCHA];
	leftPlayerFire = (RAM[_INPT4]&0x80) == 0x00;
	rightPlayerFire = (RAM[_INPT5]&0x80) == 0x00;

	drawTextInput();
	drawKeyboard();
}


void setDataStreams() {
	for (int i = 0; i < _NUM_TEXTAREA_DATASTREAMS; i ++ ) {
		int addr;
		addr = _TEXTAREA_ORIGIN + (i*_SCANLINES_IN_TEXTAREA);
		setPointer(_TEXTAREA_DATASTREAM_BASE_REG+i, addr);
	}

	for (int i = 0; i < _NUM_TEXTINPUT_DATASTREAMS; i ++ ) {
		int addr;
		addr = _TEXTINPUT_ORIGIN + (i*_SCANLINES_IN_TEXTINPUT);
		setPointer(_TEXTINPUT_DATASTREAM_BASE_REG+i, addr);
	}

	for (int i = 0; i < _NUM_KEYBOARD_DATASTREAMS; i ++ ) {
		int addr;
		addr = _KEYBOARD_ORIGIN + (i*_SCANLINES_IN_KEYBOARD);
		setPointer(_KEYBOARD_DATASTREAM_BASE_REG+i, addr);
	}
}

void drawKeyboardCoords(unsigned char c[LINE_HEIGHT], const int x, const int y) {
	int addr = _KEYBOARD_ORIGIN + (x*_SCANLINES_IN_KEYBOARD);
	int sl = LINE_SPACING_TOP + (y * (LINE_HEIGHT+LINE_SPACING_BOTTOM));

	for (int l = 0; l < LINE_HEIGHT; l++) {
		RAM[addr + sl] = c[l];
		sl++;
	}
}

void drawKeyboard() {
	int x, y;
	int c = upperCaseAoffset;

	// main keys on the keyboard A to Z
	for (y = 0; y <= 5 && c <= upperCaseZoffset; y++ ) {
		for (x = 0; x <= 5 && c <= upperCaseZoffset; x++ ) {
			struct glyph g;
			if (c == upperCaseIoffset) {
				g = keyboardLetterI;
			} else {
				g = asciiGlyphs[c];
			}

			unsigned char d[LINE_HEIGHT] = {0};
			for (int l = 0; l < LINE_HEIGHT; l++) {
				d[l] = g.data[l] << 2;

				// add indicator to the glyph for the currently selected key
				if (selectedKey == c) {
					d[l] |= 0x41;
				}
			}

			drawKeyboardCoords(d, x, y);
			c++;
		}
	}

	// space key
	unsigned char d[LINE_HEIGHT] = {0};
	for (int l = 0; l < LINE_HEIGHT; l++) {
		d[l] = keyboardSpace.data[l] << 2;
		if (selectedKey == spaceKey) {
			d[l] |= 0x41;
		}

	}
	drawKeyboardCoords(d, 2, 4);

	// backspace key
	for (int l = 0; l < LINE_HEIGHT; l++) {
		d[l] = keyboardBackSpace.data[l] << 2;
		if (selectedKey == backspaceKey) {
			d[l] |= 0x41;
		}
	}
	drawKeyboardCoords(d, 3, 4);

	// return key
	for (int l = 0; l < LINE_HEIGHT; l++) {
		d[l] = keyboardReturn.data[l] << 2;
		if (selectedKey == returnKey) {
			d[l] |= 0x41;
		}
	}
	drawKeyboardCoords(d, 4, 4);
}

void drawTextInputCoords(unsigned char c[LINE_HEIGHT], const int x) {
	int addr = _TEXTINPUT_ORIGIN + (x*_SCANLINES_IN_TEXTINPUT);
	int sl = 0;

	for (int l = 0; l < LINE_HEIGHT; l++) {
		RAM[addr + sl] = c[l];
		sl++;
	}
}

void drawTextInput() {
	char prev = '\0';
	unsigned char d[LINE_HEIGHT] = {0};
	int pos = 0;
	int x = 0;

	for (int i = 0; i < MAX_INPUT_CHARS; i++) {
		struct glyph g;
		unsigned char c = '\0';

		if (i == inputIdx) {
			g = inputCursor;
		} else {
			c = input[i];
			g = asciiGlyphs[(c&asciiMask)-asciiAdj];
		}


		// the width of the glyph including any kerning adjustments
		int width = g.width;
		for (int i = 0; i < numKernings; i++) {
			struct kerning d = kernings[i];

			if (d.previous == prev && d.current == c ) {
				width -= d.adj;
				break; // for loop
			}
		}

		int shift = 8-pos-width;
		for (int l = 0; l < LINE_HEIGHT; l++) {
			// bit pattern for the glyph at the current scanline
			int b = g.data[l];
			if (shift >= 0) {
				d[l] |= b << shift;
			} else if (shift < 0) {
				d[l] |= b >> -shift;
			}
		}

		pos += width;
		if (pos >= 8) {
			drawTextInputCoords(d, x);
			pos -= 8;
			x++;
			myMemset(d, 0x00, LINE_HEIGHT);

			// the last glyph was only partially drawn so we need to prepare 
			// d for the next iteration. we don't need to draw it, just
			// prepare it
			if (pos > 0) {
				int shift = 8 - pos;
				for (int l = 0; l < LINE_HEIGHT; l++) {
					// bit pattern for the glyph at the current scanline
					int b = g.data[l];
					d[l] |= b << shift;
				}
			}
		}

		prev = c;
	}

	drawTextInputCoords(d, x);

	// draw rest of textinput line. this has the effect of blanking out any
	// unused text from a previous entry
	//
	// the for loop is a bit unusual in that we begin by increasing the existing
	// x value by one. this is intentional
	for (x++ ; x < MAX_INPUT_CHARS; x++) {
		drawTextInputCoords(emptyGlyph.data, x);
	}
}

void drawTextAreaCoords(unsigned char c[LINE_HEIGHT], const int x, const int y) {
	int addr = _TEXTAREA_ORIGIN + (x*_SCANLINES_IN_TEXTAREA);
	int sl = LINE_SPACING_TOP + (y * (LINE_HEIGHT+LINE_SPACING_BOTTOM));

	for (int l = 0; l < LINE_HEIGHT; l++) {
		RAM[addr + sl] = c[l];
		sl++;
	}
}

void printTextArea(char text[], int length) {
	char prev = '\0';

	for (int i = 0; i < length; i++) {
		unsigned char c = text[i];

		if (c == '\n') {
			drawTextAreaCoords(drawColumn, drawX, drawY);
			nextOutputLine();
			prev = c;
			continue; // for loop
		}

		// the glyph to use by default
		struct glyph g = asciiGlyphs[(c&asciiMask)-asciiAdj];

		// the width of the glyph including any kerning adjustments
		int width = g.width;
		for (int i = 0; i < numKernings; i++) {
			struct kerning d = kernings[i];

			if (d.previous == prev && d.current == c ) {
				width -= d.adj;
				break; // for loop
			}
		}

		int shift = 8-drawPos-width;
		for (int l = 0; l < LINE_HEIGHT; l++) {
			// bit pattern for the glyph at the current scanline
			int b = g.data[l];
			if (shift >= 0) {
				drawColumn[l] |= b << shift;
			} else if (shift < 0) {
				drawColumn[l] |= b >> -shift;
			}
		}

		drawPos += width;
		if (drawPos >= 8) {
			drawTextAreaCoords(drawColumn, drawX, drawY);
			drawPos -= 8;
			drawX++;
			myMemset(drawColumn, 0x00, LINE_HEIGHT);

			// the last glyph was only partially drawn so we need to prepare the
			// drawColumn for the next iteration. we don't need to draw it, just
			// prepare it
			if (drawPos > 0) {
				int shift = 8 - drawPos;
				for (int l = 0; l < LINE_HEIGHT; l++) {
					// bit pattern for the glyph at the current scanline
					int b = g.data[l];
					drawColumn[l] |= b << shift;
				}
			}
		}

		prev = c;
	}

	drawTextAreaCoords(drawColumn, drawX, drawY);
}

// affects the text area
void flushWordBuffer() {
	// if word cannot fit on the current line, insert a newline before
	// continuing
	if (lineX + wordBufferIdx >= CHARS_PER_TEXTAREA_ROW) {
		nextOutputLine();
	}

	printTextArea(wordBuffer, wordBufferIdx);
	lineX += wordBufferIdx;
	wordBufferIdx = 0;
}

// affects the text area
void updateWordBuffer(const char c) {
	// prevent double-spaces
	if (c == ' ' && prevOutputChar == ' ' && drawX > 0) {
		return;
	}

	prevOutputChar = c;
	wordBuffer[wordBufferIdx] = c;
	wordBufferIdx++;

	// flush buffer on newline a space or when buffer is full
	if (c == '\n' || c == ' ' || c == '.' || wordBufferIdx >= CHARS_PER_TEXTAREA_ROW) {
		flushWordBuffer();
	}
}

#define SCROLL_BOUNDARY 1

void nextOutputLine() {
	drawY++;
	drawX = 0;
	lineX = 0;
	myMemset(drawColumn, 0x00, LINE_HEIGHT);
	drawPos = 0;

	if (drawY > ROWS_PER_TEXTAREA-SCROLL_BOUNDARY) {
		scrollscr();
	}
}

void scrollscr() {
	// the copy distance within the datastream
	#define DISTANCE (SCROLL_BOUNDARY*(LINE_HEIGHT+LINE_SPACING))

	for (int d = 0; d < _NUM_TEXTAREA_DATASTREAMS; d++) {
		int addr = _TEXTAREA_ORIGIN + (d * _SCANLINES_IN_TEXTAREA);

		// copy scanlines up by specified DISTANCE
		for (int y = 0; y < _SCANLINES_IN_TEXTAREA-DISTANCE-1; y++) {
			RAM[addr+y] = RAM[addr+y+DISTANCE-1];
		}

		// clear remainder of the datastream
		for (int y = _SCANLINES_IN_TEXTAREA-DISTANCE; y < _SCANLINES_IN_TEXTAREA; y++) {
			RAM[addr+y] = 0x00;
		}
	}

	drawY = ROWS_PER_TEXTAREA - SCROLL_BOUNDARY;
}

// quickCommand is a way of putting a command into the input buffer and executing it
void quickCommand(char command[]) {
	int i;
	for (i = 0; command[i] != '\0'; i++) {
		input[i] = command[i];
	}
	input[i] = '\0';
	inputIdx = i;
	submitCommand();
}

// submitCommand() displays the input and steps the main advland program
void submitCommand() {
	// protect the user from accidentally quitting the game. normally, there
	// would be a "do you really want to quit?" type prompt but for simplicity
	// we've removed it
	switch (inputIdx) {
	case 1:
		if (input[0] == 'Q') {
			return;
		}
	case 2:
		if (input[0] == 'Q' && input [1] == 'U') {
			return;
		}
	case 3:
		if (input[0] == 'Q' && input[1] == 'U' && input[2] == 'I') {
			return;
		}
	}

	_printf("\n> %s\n", input);
	nextOutputLine();
	stepAdvland();
	drawTextInput();
	drawKeyboard();
}

/********************* advland helper function *********************/

// affects the text area
void clrscr() {
	// clear integers (32bit) instead of chars (8bit) for performance reasons.
	// size of memset therefore is a quarter of the datastreams size
	#define CLEAR_TEXTAREA _TEXTAREA_SIZE / 4

	myMemsetInt((unsigned int *)RAM+_TEXTAREA_ORIGIN-4, 0x00, CLEAR_TEXTAREA);
	drawY = 0;
	drawX = 0;

	myMemset(drawColumn, 0x00, LINE_HEIGHT);
	drawPos = 0;
}

int wherex() {
	return lineX;
}

int kbhit() {
	return 0;
}

// _printf should only be used for output from the engine
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
						updateWordBuffer(*p);
					}
				} else {
					int i;
					char *p;
					for (i = 0, p = va_arg(ap, char *); i < fieldWidth && *p; i++, p++) {
						updateWordBuffer(*p);
					}
				}
			} else if (*s == 'c' || *s == 'C') {
				// chars are promoted to int when passed in a vararg
				updateWordBuffer((char)va_arg(ap, int));
			} else if (*s == 'd' || *s == 'D' || *s == 'u' || *s == 'U') {
				int d;
				d = va_arg(ap, int);
				if (d >= 0 && d <= 9) {
					updateWordBuffer(d + 48);
				} else {
					updateWordBuffer('&');
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
				updateWordBuffer('&');
			}
		} else {
			updateWordBuffer(*s);
		}

		s++;

		if (skipFieldWidthReset == 0) {
			fieldWidth = -1;
		} else {
			skipFieldWidthReset = 0;
		}
	}

	prevOutputChar = '\0';

	va_end(ap);

	flushWordBuffer();
}

char * _gets(char *s) {
	int i;
	for (i = 0; i < inputIdx; i++) {
		s[i] = input[i];
		input[i] = '\0';
	}
	s[i] = '\0';
	inputIdx = 0;
	return s;
}

int _getch() {
	return 0;
}

int _toupper(int c) {
	if (c >= 97 && c <= 122) {
		return c-32;
	}
	return c;
}

unsigned int _rand() {
	return getRandom32();
}

void endgame() {
	gameEnded = true;
}
