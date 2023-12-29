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

// number of scanlines in a glyph. any ascenders and descenders for the glyph is
// included in this value
#define LINE_HEIGHT  6

// the number of scanlines between character rows
#define LINE_SPACING_TOP  1
#define LINE_SPACING_BOTTOM  1
#define LINE_SPACING  LINE_SPACING_TOP + LINE_SPACING_BOTTOM

struct glyph {
	unsigned char data[LINE_HEIGHT];
	int width;
};

#define asciiMask 0x7f
#define asciiAdj 32

#define upperCaseAoffset  33
#define upperCaseIoffset  41
#define upperCaseZoffset  58

const struct glyph asciiGlyphs[] = {
	[32-asciiAdj] ={{0,0,0,0,0}, 3}, // space
	[33-asciiAdj] ={{1,1,1,0,1}, 2}, // !
	[34-asciiAdj] ={{3,3,0,0,0}, 4}, // ""
	[35-asciiAdj] ={{5,7,5,7,5}, 4}, // #
	[36-asciiAdj] ={{0,7,5,7,0}, 4}, // placeholder ($)
	[37-asciiAdj] ={{0,7,5,7,0}, 4}, // placeholder (%)
	[38-asciiAdj] ={{2,5,2,5,3}, 4}, // &
	[39-asciiAdj] ={{1,1,0,0,0}, 2}, // '
	[40-asciiAdj] ={{1,2,2,2,1}, 3}, // (
	[41-asciiAdj] ={{4,2,2,2,4}, 3}, // )
	[42-asciiAdj] ={{0,5,2,5,0}, 4}, // *
	[43-asciiAdj] ={{0,2,7,2,0}, 4}, // +
	[44-asciiAdj] ={{0,0,0,1,1}, 2}, // ,
	[45-asciiAdj] ={{0,0,3,0,0}, 3}, // -
	[46-asciiAdj] ={{0,0,0,0,1}, 2}, // .
	[47-asciiAdj] ={{0,7,5,7,0}, 4}, // placeholder (/)

	[48-asciiAdj] ={{7,5,5,5,7}, 4}, // 0
	[49-asciiAdj] ={{2,6,2,2,7}, 4}, // 1
	[50-asciiAdj] ={{7,1,7,4,7}, 4}, // 2
	[51-asciiAdj] ={{7,1,3,1,7}, 4}, // 3
	[52-asciiAdj] ={{5,5,7,1,1}, 4}, // 4
	[53-asciiAdj] ={{7,4,6,1,6}, 4}, // 5
	[54-asciiAdj] ={{4,4,7,5,7}, 4}, // 6
	[55-asciiAdj] ={{7,1,1,1,1}, 4}, // 7
	[56-asciiAdj] ={{7,5,7,5,7}, 4}, // 8
	[57-asciiAdj] ={{7,5,7,1,1}, 4}, // 9

	[58-asciiAdj] ={{0,1,0,1,0}, 2}, // :
	[59-asciiAdj] ={{0,1,0,1,1}, 2}, // ;
	[60-asciiAdj] ={{1,2,4,2,1}, 4}, // <
	[61-asciiAdj] ={{0,3,0,3,0}, 3}, // =
	[62-asciiAdj] ={{4,2,1,2,4}, 4}, // >
	[63-asciiAdj] ={{6,1,2,0,2}, 4}, // ?
	[64-asciiAdj] ={{0,7,5,7,0}, 4}, // placeholder (@)

	[65-asciiAdj] ={{2,5,7,5,5}, 4}, // A
	[66-asciiAdj] ={{6,5,6,5,6}, 4}, // B
	[67-asciiAdj] ={{3,4,4,4,3}, 4}, // C
	[68-asciiAdj] ={{6,5,5,5,6}, 4}, // D
	[69-asciiAdj] ={{7,4,6,4,7}, 4}, // E
	[70-asciiAdj] ={{7,4,6,4,4}, 4}, // F
	[71-asciiAdj] ={{3,4,4,5,7}, 4}, // G
	[72-asciiAdj] ={{5,5,7,5,5}, 4}, // H
	[73-asciiAdj] ={{1,1,1,1,1}, 2}, // I 
	[74-asciiAdj] ={{1,1,1,5,6}, 4}, // J
	[75-asciiAdj] ={{5,5,6,5,5}, 4}, // K
	[76-asciiAdj] ={{4,4,4,4,7}, 4}, // L
	[77-asciiAdj] ={{5,7,5,5,5}, 4}, // M
	[78-asciiAdj] ={{6,5,5,5,5}, 4}, // N
	[79-asciiAdj] ={{2,5,5,5,2}, 4}, // O
	[80-asciiAdj] ={{6,5,6,4,4}, 4}, // P
	[81-asciiAdj] ={{2,5,5,5,2,1}, 4}, // Q
	[82-asciiAdj] ={{6,5,6,5,5}, 4}, // R
	[83-asciiAdj] ={{3,4,7,1,6}, 4}, // S
	[84-asciiAdj] ={{7,2,2,2,2}, 4}, // T
	[85-asciiAdj] ={{5,5,5,5,7}, 4}, // U
	[86-asciiAdj] ={{5,5,5,5,2}, 4}, // V
	[87-asciiAdj] ={{5,5,5,7,5}, 4}, // W
	[88-asciiAdj] ={{5,2,2,2,5}, 4}, // X
	[89-asciiAdj] ={{5,5,7,2,2}, 4}, // Y
	[90-asciiAdj] ={{7,1,2,4,7}, 4}, // Z

	[91-asciiAdj] ={{0,7,5,7,0}, 4}, // placeholder ([)
	[92-asciiAdj] ={{0,7,5,7,0}, 4}, // placeholder (\)
	[93-asciiAdj] ={{0,7,5,7,0}, 4}, // placeholder (])
	[94-asciiAdj] ={{0,7,5,7,0}, 4}, // placeholder (^)
	[95-asciiAdj] ={{0,0,0,7,0}, 4}, // placeholder (_)
	[96-asciiAdj] ={{0,7,5,7,0}, 4}, // placeholder (`)

	[97-asciiAdj] ={{0,2,5,5,3}, 4}, // a
	[98-asciiAdj] ={{4,6,5,5,6}, 4}, // b
	[99-asciiAdj] ={{0,3,4,4,3}, 4}, // c
	[100-asciiAdj] ={{1,3,5,5,3}, 4}, // d
	[101-asciiAdj] ={{0,2,5,6,3}, 4}, // e
	[102-asciiAdj] ={{1,2,7,2,2}, 4}, // f
	[103-asciiAdj] ={{0,2,5,3,1,2}, 4}, // g
	[104-asciiAdj] ={{4,4,6,5,5}, 4}, // h
	[105-asciiAdj] ={{1,0,1,1,1}, 2}, // i 
	[106-asciiAdj] ={{1,0,1,1,2}, 3}, // j
	[107-asciiAdj] ={{4,4,5,6,5}, 4}, // k
	[108-asciiAdj] ={{2,2,2,2,1}, 3}, // l
	[109-asciiAdj] ={{0,5,7,5,5}, 4}, // m
	[110-asciiAdj] ={{0,6,5,5,5}, 4}, // n
	[111-asciiAdj] ={{0,2,5,5,2}, 4}, // o
	[112-asciiAdj] ={{0,6,5,5,6,4}, 4}, // p
	[113-asciiAdj] ={{0,3,5,5,3,1}, 4}, // q
	[114-asciiAdj] ={{0,1,2,2,2}, 3}, // r
	[115-asciiAdj] ={{0,1,2,1,2}, 3}, // s
	[116-asciiAdj] ={{2,3,2,2,2}, 3}, // t
	[117-asciiAdj] ={{0,5,5,5,7}, 4}, // u
	[118-asciiAdj] ={{0,5,5,5,2}, 4}, // v
	[119-asciiAdj] ={{0,5,5,7,5}, 4}, // w
	[120-asciiAdj] ={{0,5,2,2,5}, 4}, // x
	[121-asciiAdj] ={{0,5,5,3,1,6}, 4}, // y
	[122-asciiAdj] ={{0,7,3,6,7}, 4}, // z

	[123-asciiAdj] ={{0,7,5,7,0}, 4}, // placeholder ({)
	[124-asciiAdj] ={{0,7,5,7,0}, 4}, // placeholder (|)
	[125-asciiAdj] ={{0,7,5,7,0}, 4}, // placeholder (})
	[126-asciiAdj] ={{0,7,5,7,0}, 4}, // placeholder (~)
};

// the I we use for the text area isn't really suitable to display on
// the keyboard, this is the replacement
struct glyph keyboardLetterI = {{7,2,2,2,7}, 4};

// other keys on the keyboard have special glyphs
struct glyph keyboardSpace = {{0,0,0,0,7}, 4};
struct glyph keyboardBackSpace = {{1,3,7,3,1}, 4};
struct glyph keyboardReturn = {{7,7,5,7,7}, 4};

// text input cursor
struct glyph inputCursor = {{0,0,0,0,3}, 3};

struct kerning {
	char previous;
	char current;
	int adj;
};

#define numKernings 11

const struct kerning kernings[numKernings] = {
	[0] = {'T', '.', 1},
	[1] = {'T', ',', 1},
	[2] = {'T', 'e', 1},
	[3] = {'t', 'e', 1},
	[4] = {'t', 'o', 1},
	[5] = {'t', 's', 1},
	[6] = {'t', '.', 1},
	[7] = {'t', ',', 1},
	[8] = {'o', '?', 1},
	[9] = {'(', 's', 1},
	[10] = {')', ' ', 1},
};

