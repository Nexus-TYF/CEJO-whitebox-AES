#include "wbaes.h"

u32 TypeII[10][16][256];//Type II
u32 TypeIII[9][16][256];//Type III
u8 TypeIV[9][96][16][16];

void printstate(unsigned char * in)
{
    for(int i = 0; i < 16; i++)
    {
        printf("%.2X", in[i]);
    }
    printf("\n");
}

void wbaes_gen(u8 key[16])
{
    u8 expandedKey[176];
    expandKey (key, expandedKey);
    
    static u8 nibble[16] = {0x01, 0x02, 0x0C, 0x05, 0x07, 0x08, 0x0A, 0x0F, 0x04, 0x0D, 0x0B, 0x0E, 0x09, 0x06, 0x00, 0x03};
    static u8 nibble_inv[16] = {0x0e, 0x00, 0x01, 0x0f, 0x08, 0x03, 0x0d, 0x04, 0x05, 0x0c, 0x06, 0x0a, 0x02, 0x09, 0x0b, 0x07}; 

    M8 L[9][16];
    M8 L_inv[9][16];
    M32 MB[9][4];
    M32 MB_inv[9][4];
    for(int i = 0; i < 9; i++)
    {
        for(int j = 0; j < 16; j++)
        {
            genMatpairM8(&L[i][j], &L_inv[i][j]);
        }
    }
    for(int i = 0; i < 9; i++)
    {
        for(int j = 0; j < 4; j++)
        {
            genMatpairM32(&MB[i][j], &MB_inv[i][j]);
        }
    }

    u32 Tyi[4][256];
    for (int x = 0; x < 256; x++)
    {
      Tyi[0][x] = (gMul(2, x) << 24) | (x << 16) | (x << 8) | gMul(3, x);
      Tyi[1][x] = (gMul(3, x) << 24) | (gMul(2, x) << 16) | (x << 8) | x;
      Tyi[2][x] = (x << 24) | (gMul(3, x) << 16) | (gMul(2, x) << 8) | x;
      Tyi[3][x] = (x << 24) | (x << 16) | (gMul(3, x) << 8) | gMul(2, x);
    }

    M32 Out_L[9][4];
    for(int i = 0; i < 9; i++)
    {
        for(int j = 0; j < 4; j++)
        {
            MatrixcomM8to32(L[i][4 * j], L[i][4 * j + 1], L[i][4 * j + 2], L[i][4 * j + 3], &Out_L[i][j]);
        }
    }
    
    int columnindex[]={0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3};
    //Round 1
    shiftRows (expandedKey + 16 * 0);
    for(int j = 0; j < 16; j++)//type_II
    {
        u8 temp_u8;
        u32 temp_u32;
        for(int x = 0; x < 256; x++)
        {
            temp_u8 = SBox[x ^ expandedKey[16 * 0 + j]];
            temp_u32 = Tyi[j % 4][temp_u8];
            temp_u32 = MatMulNumM32(MB[0][columnindex[j]], temp_u32);
            TypeII[0][j][x] = (nibble[(temp_u32 & 0xf0000000) >> 28] << 28) | (nibble[(temp_u32 & 0x0f000000) >> 24] << 24) | (nibble[(temp_u32 & 0x00f00000) >> 20] << 20) | (nibble[(temp_u32 & 0x000f0000) >> 16] << 16) | (nibble[(temp_u32 & 0x0000f000) >> 12] << 12) | (nibble[(temp_u32 & 0x00000f00) >> 8] << 8) | (nibble[(temp_u32 & 0x000000f0) >> 4] << 4) | (nibble[(temp_u32 & 0x0000000f)]);
        }
    }
    for(int j = 0; j < 16; j++)//type_III
    {
        u8 temp_u8;
        u32 temp_u32;
        int shiftbit[]={24, 16, 8, 0};
        for(int x = 0; x < 256; x++)
        {
            temp_u8 = x;
            temp_u8 = (nibble_inv[(temp_u8 & 0xf0) >> 4] << 4) | (nibble_inv[(temp_u8 & 0x0f)]); 
            temp_u32 = temp_u8;
            temp_u32 = temp_u32 << shiftbit[j % 4];
            temp_u32 = MatMulNumM32(MB_inv[0][columnindex[j]], temp_u32);
            temp_u32 = MatMulNumM32(Out_L[0][columnindex[j]], temp_u32);
            TypeIII[0][j][x] = (nibble[(temp_u32 & 0xf0000000) >> 28] << 28) | (nibble[(temp_u32 & 0x0f000000) >> 24] << 24) | (nibble[(temp_u32 & 0x00f00000) >> 20] << 20) | (nibble[(temp_u32 & 0x000f0000) >> 16] << 16) | (nibble[(temp_u32 & 0x0000f000) >> 12] << 12) | (nibble[(temp_u32 & 0x00000f00) >> 8] << 8) | (nibble[(temp_u32 & 0x000000f0) >> 4] << 4) | (nibble[(temp_u32 & 0x0000000f)]);
        }
    }

    //Round 2-9
    int shiftindex[]={0, 5, 10, 15, 4, 9, 14, 3, 8, 13, 2, 7, 12, 1, 6, 11};
    for (int i = 1; i < 9; i++)//Type_II
    {
        shiftRows (expandedKey + 16 * i);
        for(int j = 0; j < 16; j++)
        {
            u8 temp_u8;
            u32 temp_u32;
            for(int x = 0; x < 256; x++)
            {
                temp_u8 = x;
                temp_u8 = (nibble_inv[(temp_u8 & 0xf0) >> 4] << 4) | (nibble_inv[(temp_u8 & 0x0f)]);
                temp_u8 = MatMulNumM8(L_inv[i - 1][shiftindex[j]], temp_u8);
                temp_u8 = SBox[temp_u8 ^ expandedKey[16 * i + j]];
                temp_u32 = Tyi[j % 4][temp_u8];
                temp_u32 = MatMulNumM32(MB[i][columnindex[j]], temp_u32);
                TypeII[i][j][x] = (nibble[(temp_u32 & 0xf0000000) >> 28] << 28) | (nibble[(temp_u32 & 0x0f000000) >> 24] << 24) | (nibble[(temp_u32 & 0x00f00000) >> 20] << 20) | (nibble[(temp_u32 & 0x000f0000) >> 16] << 16) | (nibble[(temp_u32 & 0x0000f000) >> 12] << 12) | (nibble[(temp_u32 & 0x00000f00) >> 8] << 8) | (nibble[(temp_u32 & 0x000000f0) >> 4] << 4) | (nibble[(temp_u32 & 0x0000000f)]);
            }
        }
    
        for(int j = 0; j < 16; j++)//type_III
        {
            u8 temp_u8;
            u32 temp_u32;
            int shiftbit[]={24, 16, 8, 0};
            for(int x = 0; x < 256; x++)
            {
                temp_u8 = x;
                temp_u8 = (nibble_inv[(temp_u8 & 0xf0) >> 4] << 4) | (nibble_inv[(temp_u8 & 0x0f)]);
                temp_u32 = temp_u8;
                temp_u32 = temp_u32 << shiftbit[j % 4];
                temp_u32 = MatMulNumM32(MB_inv[i][columnindex[j]], temp_u32);
                temp_u32 = MatMulNumM32(Out_L[i][columnindex[j]], temp_u32);
                TypeIII[i][j][x] = (nibble[(temp_u32 & 0xf0000000) >> 28] << 28) | (nibble[(temp_u32 & 0x0f000000) >> 24] << 24) | (nibble[(temp_u32 & 0x00f00000) >> 20] << 20) | (nibble[(temp_u32 & 0x000f0000) >> 16] << 16) | (nibble[(temp_u32 & 0x0000f000) >> 12] << 12) | (nibble[(temp_u32 & 0x00000f00) >> 8] << 8) | (nibble[(temp_u32 & 0x000000f0) >> 4] << 4) | (nibble[(temp_u32 & 0x0000000f)]);
            }
        }
    }

    //Round 10
    shiftRows (expandedKey + 16 * 9);
    for(int j = 0; j < 16; j++)//type_II
    {
        u8 temp_u8;
        for(int x = 0; x < 256; x++)
        {
            temp_u8 = x;
            temp_u8 = (nibble_inv[(temp_u8 & 0xf0) >> 4] << 4) | (nibble_inv[(temp_u8 & 0x0f)]);
            temp_u8 = MatMulNumM8(L_inv[8][shiftindex[j]], temp_u8);
            temp_u8 = SBox[temp_u8 ^ expandedKey[16 * 9 + j]];
            TypeII[9][j][x] = temp_u8 ^ expandedKey[16 * 10 + j];
        }
    }

    for (int i = 0; i < 9; i++)
    {
        for (int j = 0; j < 96; j++)
        {
            for (int x = 0; x < 16; x++)
            {
                for (int y = 0; y < 16; y++)
                {
                    TypeIV[i][j][x][y] = nibble[nibble_inv[x] ^ nibble_inv[y]];
                }
            }
        }
    }

}
void wbaes_encrypt(u8 input[16], u8 output[16]) 
{
    u32 a, b, c, d, aa, bb, cc, dd;
    u8 state[16];
    for(int i = 0; i < 16; i++)
    {
        state[i] = input[i];
    }
    
    for (int i = 0; i < 9; i++) 
    {
        shiftRows (state);

        for (int j = 0; j < 4; j++)
        {
            a = TypeII[i][4*j + 0][state[4*j + 0]];
            b = TypeII[i][4*j + 1][state[4*j + 1]];
            c = TypeII[i][4*j + 2][state[4*j + 2]];
            d = TypeII[i][4*j + 3][state[4*j + 3]];

            aa = TypeIV[i][24*j + 0][(a >> 28) & 0xf][(b >> 28) & 0xf];
            bb = TypeIV[i][24*j + 1][(c >> 28) & 0xf][(d >> 28) & 0xf];
            cc = TypeIV[i][24*j + 2][(a >> 24) & 0xf][(b >> 24) & 0xf];
            dd = TypeIV[i][24*j + 3][(c >> 24) & 0xf][(d >> 24) & 0xf];
            state[4*j + 0] = (TypeIV[i][24*j + 4][aa][bb] << 4) | TypeIV[i][24*j + 5][cc][dd];

            aa = TypeIV[i][24*j + 6][(a >> 20) & 0xf][(b >> 20) & 0xf];
            bb = TypeIV[i][24*j + 7][(c >> 20) & 0xf][(d >> 20) & 0xf];
            cc = TypeIV[i][24*j + 8][(a >> 16) & 0xf][(b >> 16) & 0xf];
            dd = TypeIV[i][24*j + 9][(c >> 16) & 0xf][(d >> 16) & 0xf];
            state[4*j + 1] = (TypeIV[i][24*j + 10][aa][bb] << 4) | TypeIV[i][24*j + 11][cc][dd];

            aa = TypeIV[i][24*j + 12][(a >> 12) & 0xf][(b >> 12) & 0xf];
            bb = TypeIV[i][24*j + 13][(c >> 12) & 0xf][(d >> 12) & 0xf];
            cc = TypeIV[i][24*j + 14][(a >>  8) & 0xf][(b >>  8) & 0xf];
            dd = TypeIV[i][24*j + 15][(c >>  8) & 0xf][(d >>  8) & 0xf];
            state[4*j + 2] = (TypeIV[i][24*j + 16][aa][bb] << 4) | TypeIV[i][24*j + 17][cc][dd];

            aa = TypeIV[i][24*j + 18][(a >>  4) & 0xf][(b >>  4) & 0xf];
            bb = TypeIV[i][24*j + 19][(c >>  4) & 0xf][(d >>  4) & 0xf];
            cc = TypeIV[i][24*j + 20][(a >>  0) & 0xf][(b >>  0) & 0xf];
            dd = TypeIV[i][24*j + 21][(c >>  0) & 0xf][(d >>  0) & 0xf];
            state[4*j + 3] = (TypeIV[i][24*j + 22][aa][bb] << 4) | TypeIV[i][24*j + 23][cc][dd];


            a = TypeIII[i][4*j + 0][state[4*j + 0]];
            b = TypeIII[i][4*j + 1][state[4*j + 1]];
            c = TypeIII[i][4*j + 2][state[4*j + 2]];
            d = TypeIII[i][4*j + 3][state[4*j + 3]];

            aa = TypeIV[i][24*j + 0][(a >> 28) & 0xf][(b >> 28) & 0xf];
            bb = TypeIV[i][24*j + 1][(c >> 28) & 0xf][(d >> 28) & 0xf];
            cc = TypeIV[i][24*j + 2][(a >> 24) & 0xf][(b >> 24) & 0xf];
            dd = TypeIV[i][24*j + 3][(c >> 24) & 0xf][(d >> 24) & 0xf];
            state[4*j + 0] = (TypeIV[i][24*j + 4][aa][bb] << 4) | TypeIV[i][24*j + 5][cc][dd];

            aa = TypeIV[i][24*j + 6][(a >> 20) & 0xf][(b >> 20) & 0xf];
            bb = TypeIV[i][24*j + 7][(c >> 20) & 0xf][(d >> 20) & 0xf];
            cc = TypeIV[i][24*j + 8][(a >> 16) & 0xf][(b >> 16) & 0xf];
            dd = TypeIV[i][24*j + 9][(c >> 16) & 0xf][(d >> 16) & 0xf];
            state[4*j + 1] = (TypeIV[i][24*j + 10][aa][bb] << 4) | TypeIV[i][24*j + 11][cc][dd];

            aa = TypeIV[i][24*j + 12][(a >> 12) & 0xf][(b >> 12) & 0xf];
            bb = TypeIV[i][24*j + 13][(c >> 12) & 0xf][(d >> 12) & 0xf];
            cc = TypeIV[i][24*j + 14][(a >>  8) & 0xf][(b >>  8) & 0xf];
            dd = TypeIV[i][24*j + 15][(c >>  8) & 0xf][(d >>  8) & 0xf];
            state[4*j + 2] = (TypeIV[i][24*j + 16][aa][bb] << 4) | TypeIV[i][24*j + 17][cc][dd];

            aa = TypeIV[i][24*j + 18][(a >>  4) & 0xf][(b >>  4) & 0xf];
            bb = TypeIV[i][24*j + 19][(c >>  4) & 0xf][(d >>  4) & 0xf];
            cc = TypeIV[i][24*j + 20][(a >>  0) & 0xf][(b >>  0) & 0xf];
            dd = TypeIV[i][24*j + 21][(c >>  0) & 0xf][(d >>  0) & 0xf];
            state[4*j + 3] = (TypeIV[i][24*j + 22][aa][bb] << 4) | TypeIV[i][24*j + 23][cc][dd];
            
        }
    }
    //Round 10
    shiftRows(state);
    for (int j = 0; j < 16; j++) 
    {
        state[j] = TypeII[9][j][state[j]];
    }

    for (int i = 0; i < 16; i++)
    {    
        output[i] = state[i];
    }
}