#ifndef GENTABLES_H
#define GENTABLES_H

#include "aes.h"
#include "WBMatrix/WBMatrix.h"

u32 TypeII[10][16][256];//Type II
u32 TypeIII[9][16][256];//Type III
u8 TypeIV_II[9][4][3][8][16][16];
u8 TypeIV_III[9][4][3][8][16][16];
u8 TypeIa[16][256];
u8 TypeIb[16][256];

void wbaes_gen(u8 key[16]);
void wbaes_encrypt(u8 input[16], u8 output[16]);
void printstate(unsigned char * in);

#endif // GENTABLES_H