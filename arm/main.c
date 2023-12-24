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

#include <stdarg.h>
#include <defines_cdfj.h>
#include <advland_helpers.h>
#include "glyphs.h"
#include "shared_defines.h"

// how many glyphs can we pack into a data stream
#define GLYPHS_PER_COLUMN 2

// width of screen in characters
#define CHARS_PER_ROW _NUM_DATASTREAMS * GLYPHS_PER_COLUMN

// height of screen in characters
#define CHARS_PER_DATASTREAM _SCANLINES_IN_DATASTREAM / (LINE_HEIGHT + LINE_SPACING)

// the location on the screen (in characters) for the next output character
int drawX = 0; 
int drawY = 0;

// the data used to update the screen
unsigned char drawColumn[LINE_HEIGHT];
int drawPos = 0;

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

// the number of characters in the line thus far. not the same as drawX
int lineX = 0;

// the previous character to be output. we use this to prevent double-spaces.
char prevOutputChar = '\0';

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
void draw(unsigned char c[LINE_HEIGHT], const int x, const int y);
void drawText(char text[], int length);
void flushWordBuffer();
void updateWordBuffer(const char c);
void nextOutputLine();
void scrollscr();


int main() {
	switch (RAM[_RUN_FUNC]) {
		case _FN_INIT:
			initialise();
			setDataStreams();
			printInputCursor();
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
					stepAdvland();
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

	// set all global variables to inital values
	drawX = 0; 
	drawY = 0;
	drawPos = 0;
	prevInputKey = -1;
	inputIdx = 0;
	keyGroup = -1;
	keyGroupOpt = -1;
	runCountVBLANK = 0;
	wordBufferIdx = 0;
	lineX = 0;
	prevOutputChar = '\0';

	initAdvland();
	nextOutputLine();
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
}

void printInputCursor() {
}

void draw(unsigned char c[LINE_HEIGHT], const int x, const int y) {
	int addr = _DATASTREAMS_ORIGIN + (x*_SCANLINES_IN_DATASTREAM);
	int sl = LINE_SPACING_TOP + (y * (LINE_HEIGHT+LINE_SPACING_BOTTOM));

	for (int l = 0; l < LINE_HEIGHT; l++) {
		RAM[addr + sl] = c[l];
		sl++;
	}
}

void drawText(char text[], int length) {
	char prev = '\0';

	for (int i = 0; i < length; i++) {
		unsigned char c = text[i];

		if (c == '\n') {
			draw(drawColumn, drawX, drawY);
			nextOutputLine();
			prev = c;
			continue; // for loop
		}

		// the glyph to use by default
		struct glyph g = symbolGlyphs[placeholderGlyph];

		if (c >= '0' && c <= '9') {
			g = digitsGlyphs[c-numberAdj];
		} else if (c >= 'A' && c <= 'Z') {
			g = upperCaseGlyphs[c-upperCaseAdj];
		} else if (c >= 'a' && c <= 'z') {
			g = upperCaseGlyphs[c-lowerCaseAdj];
		} else if (c == ' ') {
			g = symbolGlyphs[spaceGlyph];
		} else if (c == '&') {
			g = symbolGlyphs[ampersandGlyph];
		} else if (c == '-') {
			g = symbolGlyphs[hyphenGlyph];
		} else if (c == '*') {
			g = symbolGlyphs[asteriskGlyph];
		} else if (c == '?') {
			g = symbolGlyphs[questionMarkGlyph];
		} else if (c == '(' || c == '[' || c == '{') {
			g = symbolGlyphs[openParenGlyph];
		} else if (c == ')' || c == ']' || c == '}') {
			g = symbolGlyphs[closeParenGlyph];
		} else if (c == '_') {
			g = symbolGlyphs[underscoreGlyph];
		} else if (c == '"') {
			g = symbolGlyphs[quoteGlyph];
		} else if (c == 0xff) {
			g = symbolGlyphs[cursorGlyph];
		} else if (c == '.') {
			g = symbolGlyphs[fullStopGlyph];
		} else if (c == ',') {
			g = symbolGlyphs[commaGlyph];
		} else if (c == '\'') {
			g = symbolGlyphs[apostropheGlyph];
		} else if (c == '\0') {
			break; // for loop
		} else {
			prev = c;
			continue; // for loop
		}

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
			draw(drawColumn, drawX, drawY);
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

	draw(drawColumn, drawX, drawY);
}

void flushWordBuffer() {
	// if word cannot fit on the current line, insert a newline before
	// continuing
	if (drawX + wordBufferIdx >= CHARS_PER_ROW) {
		nextOutputLine();
	}

	drawText(wordBuffer, wordBufferIdx);
	lineX += wordBufferIdx;
	wordBufferIdx = 0;
}

void updateWordBuffer(const char c) {
	// prevent double-spaces
	if (c == ' ' && prevOutputChar == ' ' && drawX > 0) {
		return;
	}

	prevOutputChar = c;
	wordBuffer[wordBufferIdx] = c;
	wordBufferIdx++;

	// flush buffer on newline a space or when buffer is full
	if (c == '\n' || c == ' ' || wordBufferIdx >= CHARS_PER_ROW) {
		flushWordBuffer();
	}
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

void clrscr() {
	// clear integers (32bit) instead of chars (8bit) for performance reasons.
	// size of memset therefore is a quarter of the datastreams size
	#define CLEAR_DATASTREAM _DATASTREAMS_SIZE / 4

	myMemsetInt((unsigned int *)RAM+_DATASTREAMS_ORIGIN-4, 0x00, CLEAR_DATASTREAM);
	drawY = 0;
	drawX = 0;

	myMemset(drawColumn, 0x00, LINE_HEIGHT);
	drawPos = 0;
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
	return drawX;
}

unsigned int _rand() {
	return getRandom32();
}

void nextOutputLine() {
	drawY++;
	drawX = 0;
	lineX = 0;
	myMemset(drawColumn, 0x00, LINE_HEIGHT);
	drawPos = 0;
	if (drawY >= CHARS_PER_DATASTREAM-1) {
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

	// we need to adjust drawY which is measure in characters rather than
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

	drawY = SCROLL_CHARS;
}
