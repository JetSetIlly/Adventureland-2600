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

// number of scanlines in a glyph
#define LINE_HEIGHT  5

// the number of scanlines between character rows
#define LINE_SPACING_TOP  1
#define LINE_SPACING_BOTTOM  2
#define LINE_SPACING  LINE_SPACING_TOP + LINE_SPACING_BOTTOM

// the amount to adjust the ASCII value by to correctly index the
// glyphs array
#define upperCaseAdj 65
#define lowerCaseAdj 97
#define numberAdj 48

// some ASCII characters need to be converted to specific glyphs. we're not
// grouping these in the same order as the ASCII table although arguably that
// would make for easier conversion - requiring the code to explicitly map the
// ASCII to the glyph makes for clearer code IMO
#define spaceGlyph 0
#define ampersandGlyph 1
#define hyphenGlyph 2
#define asteriskGlyph 3
#define questionMarkGlyph 4
#define exclamationGlyph 5
#define openParenGlyph 6
#define closeParenGlyph 7
#define underscoreGlyph 8
#define quoteGlyph 9
#define fullStopGlyph 10
#define commaGlyph 11
#define apostropheGlyph 12
#define cursorGlyph 13
#define placeholderGlyph 14
#define numSymbolsGlyphs 15

struct glyph {
	unsigned char data[LINE_HEIGHT];
	int width;
};


const struct glyph upperCaseGlyphs[26] = {
	[0] ={{2,5,7,5,5}, 4},
	[1] ={{6,5,6,5,6}, 4},
	[2] ={{3,4,4,4,3}, 4},
	[3] ={{6,5,5,5,6}, 4},
	[4] ={{7,4,6,4,7}, 4},
	[5] ={{7,4,6,4,4}, 4},
	[6] ={{3,4,4,5,7}, 4},
	[7] ={{5,5,7,5,5}, 4},
	[8] ={{1,1,1,1,1}, 2}, // I 
	[9] ={{1,1,1,5,7}, 4},
	[10] ={{5,5,6,5,5}, 4},
	[11] ={{4,4,4,4,7}, 4},
	[12] ={{5,7,5,5,5}, 4},
	[13] ={{6,5,5,5,5}, 4},
	[14] ={{2,5,5,5,2}, 4},
	[15] ={{7,5,7,4,4}, 4},
	[16] ={{2,5,5,7,3}, 4},
	[17] ={{6,5,6,5,5}, 4},
	[18] ={{7,4,7,1,7}, 4},
	[19] ={{7,2,2,2,2}, 4},
	[20] ={{5,5,5,5,7}, 4},
	[21] ={{5,5,5,5,2}, 4},
	[22] ={{5,5,5,7,5}, 4},
	[23] ={{5,2,2,2,5}, 4},
	[24] ={{5,5,7,2,2}, 4},
	[25] ={{7,0,2,4,7}, 4},
};

const struct glyph digitsGlyphs[10] = {
	[0] ={{7,5,5,5,7}, 4},
	[1] ={{2,6,2,2,7}, 4},
	[2] ={{7,1,7,4,7}, 4},
	[3] ={{7,1,3,1,7}, 4},
	[4] ={{5,5,7,1,1}, 4},
	[5] ={{7,4,6,1,6}, 4},
	[6] ={{4,4,7,5,7}, 4},
	[7] ={{7,1,1,1,1}, 4},
	[8] ={{7,5,7,5,7}, 4},
	[9] ={{7,5,7,1,1}, 4},
};

const struct glyph symbolGlyphs[numSymbolsGlyphs] = {
	[spaceGlyph] ={{0,0,0,0,0}, 3},
	[ampersandGlyph] ={{0,2,7,2,0}, 4},
	[hyphenGlyph] ={{0,0,3,0,0}, 3},
	[asteriskGlyph] ={{0,2,5,2,0}, 4},
	[questionMarkGlyph] ={{7,1,2,0,2}, 4},
	[exclamationGlyph] ={{1,1,1,0,1}, 2},
	[openParenGlyph] ={{1,2,2,2,1}, 3},
	[closeParenGlyph] ={{4,2,2,2,4}, 3},
	[underscoreGlyph] ={{0,0,0,0,7}, 4},
	[quoteGlyph] ={{2,2,0,0,0}, 4},
	[fullStopGlyph] ={{0,0,0,0,1}, 2},
	[commaGlyph] ={{0,0,0,1,1}, 2},
	[apostropheGlyph] ={{1,1,0,0,0}, 2},
	[cursorGlyph] ={{0,0,0,0,7}, 2},
	[placeholderGlyph] ={{0,7,5,7,0}, 4},
};

struct kerning {
	char previous;
	char current;
	int adj;
};

#define numKernings 4

const struct kerning kernings[numKernings] = {
	[0] = {'T', '.', 1},
	[1] = {'t', '.', 1},
	[2] = {'T', ',', 1},
	[3] = {'t', ',', 1},
};

