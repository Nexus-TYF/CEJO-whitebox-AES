#include "wbaes.h"

int main(int argc, char * argv[])
{
    unsigned char IN[16] = {0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff};
    unsigned char OUT[16];
    unsigned char OOUT[16];
    u8 key[16] = {0};
    
    wbaes_gen(key);
    wbaes_encrypt(IN, OUT);
    printstate(OUT);
    
    aes_128_encrypt(IN, key, OOUT);
    printstate(OOUT);

    return 0;
}
