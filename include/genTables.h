#ifndef GENTABLES_H
#define GENTABLES_H

#include "aes.h"
#include "WBMatrix/WBMatrix.h"

void computeTables (u8 expandedKey[176]);
void aes_128_table_encrypt (u8 input[16], u8 output[16]);
void printstate(unsigned char * in);

#endif // GENTABLES_H