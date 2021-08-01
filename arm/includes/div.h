/******************************************************************************
File: div.h
******************************************************************************/

#ifndef __DIV_H
#define __DIV_H

/* Divide Mod Function (this one is from the GCC library) 
unsigned int divmodu(unsigned int num, unsigned int den, int modwanted) {
    unsigned int bit = 1;
    unsigned int res = 0;

    while (den < num && bit && !(den & (1<<31))) {
        den <<= 1;
        bit <<= 1;
    }

    while (bit) {
        if (num >= den) {
            num -= den;
            res |= bit;
        }
        bit >>= 1;
        den >>= 1;
    }
    if (modwanted) return num;
    return res;
}
*/

// Divide Mod Function (alternative smaller version from Hackers Delight) 
static unsigned int divmodu(unsigned int u, unsigned int v, int modwanted) {
    int i;
    unsigned int x = 0;

    for (i = 0; i < 32; ++i) {
        x = (x << 1) | (u >> 31);
        u = u << 1;
        if (x >= v) {
            x = x - v;
            u = u + 1;
        }
    }
    if (modwanted) return x;
    return u;
}


// Unsigned Divide
static unsigned int divu(unsigned int a, unsigned int b) {
    return divmodu(a, b, 0);
}

// Unsigned Mod
static unsigned int modu(unsigned int a, unsigned int b) {
    return divmodu(a, b, 1);
}

// Signed Divide
static int divs(int a, int b) {
    int res, neg = 0;

    if (a < 0) {
        a = -a;
        neg = !neg;
    }
    if (b < 0) {
        b = -b;
        neg = !neg;
    }

    res = divmodu(a, b, 0);
    if (neg) res = -res;
    return res;
}

// Signed Mod
static int mods(int a, int b) {
    int res, neg = 0;

    if (a < 0) {
        a = -a;
        neg = 1;
    }
    if (b < 0) b = -b;

    res = divmodu(a, b, 1);
    if (neg) res = -res;
    return res;
} 

#endif // __DIV_H
