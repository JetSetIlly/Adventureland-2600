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

// the amount to adjust the ASCII value by to correctly index the
// glyphs array
const int upperCaseAdj = 65;
const int lowerCaseAdj = 97;
const int numberAdj = 48;

// some ASCII characters need to be converted to specific glyphs. we're not
// grouping these in the same order as the ASCII table although arguably that
// would make for easier conversion - requiring the code to explicitly map the
// ASCII to the glyph makes for clearer code IMO
const int spaceGlyph = 0;
const int dotGlyph = 1;
const int commaGlyph = 2;
const int apostropheGlyph = 3;
const int ampersandGlyph = 4;
const int hyphenGlyph = 5;
const int asteriskGlyph = 6;
const int questionMarkGlyph = 7;
const int openParenGlyph = 8;
const int closeParenGlyph = 9;
const int underscoreGlyph = 10;
const int quoteGlyph = 11;
const int cursorGlyph = 12;

const unsigned char upperCaseGlyphs[] = {
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
	7,5,7,4,4,
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
};

const unsigned char alternativeGlyphs[] = {
	0,7,5,7,5, // A
	0,4,7,5,7,
	0,7,4,4,7,
	0,1,7,5,7,
	0,7,6,4,7,
	0,7,4,6,4,
	0,7,4,5,7,
	0,4,7,5,5,
	0,7,2,2,7,
	0,1,1,5,7,
	0,5,6,5,5,
	0,4,4,4,7, // L
	0,5,7,5,5,
	0,7,5,5,5,
	0,7,5,5,7,
	0,7,5,7,4,
	0,7,5,6,1,
	0,7,5,6,5,
	0,7,4,3,7,
	0,7,2,2,2,
	0,5,5,5,7,
	0,5,5,5,2,
	0,5,5,7,5,
	0,5,2,2,5,
	0,5,5,7,2,
	0,7,2,4,7,
};

const unsigned char digitsGlyphs[] = {
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
};

const unsigned char symbolsGlyphs[] = {
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
	0,0,0,0,7, // underscore
	2,2,0,0,0, // quote
	0,0,0,0,7, // cursor 
};

