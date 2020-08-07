#include "wbaes.h"

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
        for (int j = 0; j < 4; j++)
        {
            for(int k = 0; k < 3; k++)
            {
                for(int e = 0; e < 8; e++)
                {
                    for (int x = 0; x < 16; x++)
                    {
                        for (int y = 0; y < 16; y++)
                        {
                            TypeII_IV[i][j][k][e][x][y] = nibble[nibble_inv[x] ^ nibble_inv[y]];
                            TypeIII_IV[i][j][k][e][x][y] = nibble[nibble_inv[x] ^ nibble_inv[y]];
                        }
                    }
                }
            }
        }
    }

}
void wbaes_encrypt(u8 input[16], u8 output[16]) 
{
    u32 a, b, c, d, ab, cd;
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

            ab = (TypeII_IV[i][j][0][0][(a >> 28) & 0xf][(b >> 28) & 0xf] << 28) | (TypeII_IV[i][j][0][1][(a >> 24) & 0xf][(b >> 24) & 0xf] << 24) | (TypeII_IV[i][j][0][2][(a >> 20) & 0xf][(b >> 20) & 0xf] << 20) |(TypeII_IV[i][j][0][3][(a >> 16) & 0xf][(b >> 16) & 0xf] << 16) |\
            (TypeII_IV[i][j][0][4][(a >> 12) & 0xf][(b >> 12) & 0xf] << 12) | (TypeII_IV[i][j][0][5][(a >> 8) & 0xf][(b >> 8) & 0xf] << 8) | (TypeII_IV[i][j][0][6][(a >> 4) & 0xf][(b >> 4) & 0xf] << 4) | TypeII_IV[i][j][0][7][a & 0xf][b & 0xf];
            
            cd = (TypeII_IV[i][j][1][0][(c >> 28) & 0xf][(d >> 28) & 0xf] << 28) | (TypeII_IV[i][j][1][1][(c >> 24) & 0xf][(d >> 24) & 0xf] << 24) | (TypeII_IV[i][j][1][2][(c >> 20) & 0xf][(d >> 20) & 0xf] << 20) |(TypeII_IV[i][j][1][3][(c >> 16) & 0xf][(d >> 16) & 0xf] << 16) |\
            (TypeII_IV[i][j][1][4][(c >> 12) & 0xf][(d >> 12) & 0xf] << 12) | (TypeII_IV[i][j][1][5][(c >> 8) & 0xf][(d >> 8) & 0xf] << 8) | (TypeII_IV[i][j][1][6][(c >> 4) & 0xf][(d >> 4) & 0xf] << 4) | TypeII_IV[i][j][1][7][c & 0xf][d & 0xf];
            
            state[4*j + 0] = (TypeII_IV[i][j][3][0][(ab >> 28) & 0xf][(cd >> 28) & 0xf] << 4) | TypeII_IV[i][j][3][1][(ab >> 24) & 0xf][(cd >> 24) & 0xf];
            state[4*j + 1] = (TypeII_IV[i][j][3][2][(ab >> 20) & 0xf][(cd >> 20) & 0xf] << 4) | TypeII_IV[i][j][3][3][(ab >> 16) & 0xf][(cd >> 16) & 0xf];
            state[4*j + 2] = (TypeII_IV[i][j][3][4][(ab >> 12) & 0xf][(cd >> 12) & 0xf] << 4) | TypeII_IV[i][j][3][5][(ab >> 8) & 0xf][(cd >> 8) & 0xf];
            state[4*j + 3] = (TypeII_IV[i][j][3][6][(ab >> 4) & 0xf][(cd >> 4) & 0xf] << 4) | TypeII_IV[i][j][3][7][ab & 0xf][cd & 0xf];

            a = TypeIII[i][4*j + 0][state[4*j + 0]];
            b = TypeIII[i][4*j + 1][state[4*j + 1]];
            c = TypeIII[i][4*j + 2][state[4*j + 2]];
            d = TypeIII[i][4*j + 3][state[4*j + 3]];

            ab = (TypeIII_IV[i][j][0][0][(a >> 28) & 0xf][(b >> 28) & 0xf] << 28) | (TypeIII_IV[i][j][0][1][(a >> 24) & 0xf][(b >> 24) & 0xf] << 24) | (TypeIII_IV[i][j][0][2][(a >> 20) & 0xf][(b >> 20) & 0xf] << 20) |(TypeIII_IV[i][j][0][3][(a >> 16) & 0xf][(b >> 16) & 0xf] << 16) |\
            (TypeIII_IV[i][j][0][4][(a >> 12) & 0xf][(b >> 12) & 0xf] << 12) | (TypeIII_IV[i][j][0][5][(a >> 8) & 0xf][(b >> 8) & 0xf] << 8) | (TypeIII_IV[i][j][0][6][(a >> 4) & 0xf][(b >> 4) & 0xf] << 4) | TypeIII_IV[i][j][0][7][a & 0xf][b & 0xf];
            
            cd = (TypeIII_IV[i][j][1][0][(c >> 28) & 0xf][(d >> 28) & 0xf] << 28) | (TypeIII_IV[i][j][1][1][(c >> 24) & 0xf][(d >> 24) & 0xf] << 24) | (TypeIII_IV[i][j][1][2][(c >> 20) & 0xf][(d >> 20) & 0xf] << 20) |(TypeIII_IV[i][j][1][3][(c >> 16) & 0xf][(d >> 16) & 0xf] << 16) |\
            (TypeIII_IV[i][j][1][4][(c >> 12) & 0xf][(d >> 12) & 0xf] << 12) | (TypeIII_IV[i][j][1][5][(c >> 8) & 0xf][(d >> 8) & 0xf] << 8) | (TypeIII_IV[i][j][1][6][(c >> 4) & 0xf][(d >> 4) & 0xf] << 4) | TypeIII_IV[i][j][1][7][c & 0xf][d & 0xf];
            
            state[4*j + 0] = (TypeIII_IV[i][j][3][0][(ab >> 28) & 0xf][(cd >> 28) & 0xf] << 4) | TypeIII_IV[i][j][3][1][(ab >> 24) & 0xf][(cd >> 24) & 0xf];
            state[4*j + 1] = (TypeIII_IV[i][j][3][2][(ab >> 20) & 0xf][(cd >> 20) & 0xf] << 4) | TypeIII_IV[i][j][3][3][(ab >> 16) & 0xf][(cd >> 16) & 0xf];
            state[4*j + 2] = (TypeIII_IV[i][j][3][4][(ab >> 12) & 0xf][(cd >> 12) & 0xf] << 4) | TypeIII_IV[i][j][3][5][(ab >> 8) & 0xf][(cd >> 8) & 0xf];
            state[4*j + 3] = (TypeIII_IV[i][j][3][6][(ab >> 4) & 0xf][(cd >> 4) & 0xf] << 4) | TypeIII_IV[i][j][3][7][ab & 0xf][cd & 0xf];
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