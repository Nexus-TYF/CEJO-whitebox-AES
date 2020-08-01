#ifndef GENTABLES_H
#define GENTABLES_H

#include "aes.h"
#include "WBMatrix/WBMatrix.h"

void wbaes_gen(u8 key[16]);
void wbaes_encrypt(u8 input[16], u8 output[16]);
void printstate(unsigned char * in);

#endif // GENTABLES_H