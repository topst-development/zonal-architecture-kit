#include "ecc.h"

void gen_parity(uint32_t upper, uint32_t lower, uint8_t *ecc){

    //ECC decoding
    unsigned int i;
    char enc[72];
    char enc_pari[8];
    char target_ecc[8];
    char target_data[64];

    for(i=0; i<8; i++)
    {
        target_ecc[i] = 0;
    }

    for(i=0; i<32; i++)
    {
        target_data[i   ] = (lower>>i)&1;
    }

    for(i=0; i<32; i++)
    {
        target_data[i+32] = (upper>>i)&1;
    }

    enc[ 0] = target_ecc [7];
    enc[ 1] = target_ecc [0];
    enc[ 2] = target_ecc [1];
    enc[ 3] = target_data[0];
    enc[ 4] = target_ecc [2];
    enc[ 5] = target_data[1];
    enc[ 6] = target_data[2];
    enc[ 7] = target_data[3];
    enc[ 8] = target_ecc [3];
    enc[ 9] = target_data[4];
    enc[10] = target_data[5];
    enc[11] = target_data[6];
    enc[12] = target_data[7];
    enc[13] = target_data[8];
    enc[14] = target_data[9];
    enc[15] = target_data[10];
    enc[16] = target_ecc [4];
    enc[17] = target_data[11];
    enc[18] = target_data[12];
    enc[19] = target_data[13];
    enc[20] = target_data[14];
    enc[21] = target_data[15];
    enc[22] = target_data[16];
    enc[23] = target_data[17];
    enc[24] = target_data[18];
    enc[25] = target_data[19];
    enc[26] = target_data[20];
    enc[27] = target_data[21];
    enc[28] = target_data[22];
    enc[29] = target_data[23];
    enc[30] = target_data[24];
    enc[31] = target_data[25];
    enc[32] = target_ecc [5];
    enc[33] = target_data[26];
    enc[34] = target_data[27];
    enc[35] = target_data[28];
    enc[36] = target_data[29];
    enc[37] = target_data[30];
    enc[38] = target_data[31];
    enc[39] = target_data[32];
    enc[40] = target_data[33];
    enc[41] = target_data[34];
    enc[42] = target_data[35];
    enc[43] = target_data[36];
    enc[44] = target_data[37];
    enc[45] = target_data[38];
    enc[46] = target_data[39];
    enc[47] = target_data[40];
    enc[48] = target_data[41];
    enc[49] = target_data[42];
    enc[50] = target_data[43];
    enc[51] = target_data[44];
    enc[52] = target_data[45];
    enc[53] = target_data[46];
    enc[54] = target_data[47];
    enc[55] = target_data[48];
    enc[56] = target_data[49];
    enc[57] = target_data[50];
    enc[58] = target_data[51];
    enc[59] = target_data[52];
    enc[60] = target_data[53];
    enc[61] = target_data[54];
    enc[62] = target_data[55];
    enc[63] = target_data[56];
    enc[64] = target_ecc [6];
    enc[65] = target_data[57];
    enc[66] = target_data[58];
    enc[67] = target_data[59];
    enc[68] = target_data[60];
    enc[69] = target_data[61];
    enc[70] = target_data[62];
    enc[71] = target_data[63];

    enc_pari[0] = enc[71] ^           enc[69] ^           enc[67] ^           enc[65] ^
                  enc[63] ^           enc[61] ^           enc[59] ^           enc[57] ^
                  enc[55] ^           enc[53] ^           enc[51] ^           enc[49] ^
                  enc[47] ^           enc[45] ^           enc[43] ^           enc[41] ^
                  enc[39] ^           enc[37] ^           enc[35] ^           enc[33] ^
                  enc[31] ^           enc[29] ^           enc[27] ^           enc[25] ^
                  enc[23] ^           enc[21] ^           enc[19] ^           enc[17] ^
                  enc[15] ^           enc[13] ^           enc[11] ^           enc[ 9] ^
                  enc[ 7] ^           enc[ 5] ^           enc[ 3] ^           enc[ 1]            ;
    enc_pari[1] = enc[71] ^ enc[70] ^                     enc[67] ^ enc[66] ^
                  enc[63] ^ enc[62] ^                     enc[59] ^ enc[58] ^
                  enc[55] ^ enc[54] ^                     enc[51] ^ enc[50] ^
                  enc[47] ^ enc[46] ^                     enc[43] ^ enc[42] ^
                  enc[39] ^ enc[38] ^                     enc[35] ^ enc[34] ^
                  enc[31] ^ enc[30] ^                     enc[27] ^ enc[26] ^
                  enc[23] ^ enc[22] ^                     enc[19] ^ enc[18] ^
                  enc[15] ^ enc[14] ^                     enc[11] ^ enc[10] ^
                  enc[ 7] ^ enc[ 6] ^                     enc[ 3] ^ enc[ 2]                      ;
    enc_pari[2] = enc[71] ^ enc[70] ^ enc[69] ^ enc[68] ^
                  enc[63] ^ enc[62] ^ enc[61] ^ enc[60] ^
                  enc[55] ^ enc[54] ^ enc[53] ^ enc[52] ^
                  enc[47] ^ enc[46] ^ enc[45] ^ enc[44] ^
                  enc[39] ^ enc[38] ^ enc[37] ^ enc[36] ^
                  enc[31] ^ enc[30] ^ enc[29] ^ enc[28] ^
                  enc[23] ^ enc[22] ^ enc[21] ^ enc[20] ^
                  enc[15] ^ enc[14] ^ enc[13] ^ enc[12] ^
                  enc[ 7] ^ enc[ 6] ^ enc[ 5] ^ enc[ 4]                                          ;
    enc_pari[3] =
                  enc[63] ^ enc[62] ^ enc[61] ^ enc[60] ^ enc[59] ^ enc[58] ^ enc[57] ^ enc[56] ^

                  enc[47] ^ enc[46] ^ enc[45] ^ enc[44] ^ enc[43] ^ enc[42] ^ enc[41] ^ enc[40] ^

                  enc[31] ^ enc[30] ^ enc[29] ^ enc[28] ^ enc[27] ^ enc[26] ^ enc[25] ^ enc[24] ^

                  enc[15] ^ enc[14] ^ enc[13] ^ enc[12] ^ enc[11] ^ enc[10] ^ enc[ 9] ^ enc[ 8]
                                                                                                 ;
    enc_pari[4] =
                  enc[63] ^ enc[62] ^ enc[61] ^ enc[60] ^ enc[59] ^ enc[58] ^ enc[57] ^ enc[56] ^
                  enc[55] ^ enc[54] ^ enc[53] ^ enc[52] ^ enc[51] ^ enc[50] ^ enc[49] ^ enc[48] ^


                  enc[31] ^ enc[30] ^ enc[29] ^ enc[28] ^ enc[27] ^ enc[26] ^ enc[25] ^ enc[24] ^
                  enc[23] ^ enc[22] ^ enc[21] ^ enc[20] ^ enc[19] ^ enc[18] ^ enc[17] ^ enc[16]

                                                                                                 ;
    enc_pari[5] =
                  enc[63] ^ enc[62] ^ enc[61] ^ enc[60] ^ enc[59] ^ enc[58] ^ enc[57] ^ enc[56] ^
                  enc[55] ^ enc[54] ^ enc[53] ^ enc[52] ^ enc[51] ^ enc[50] ^ enc[49] ^ enc[48] ^
                  enc[47] ^ enc[46] ^ enc[45] ^ enc[44] ^ enc[43] ^ enc[42] ^ enc[41] ^ enc[40] ^
                  enc[39] ^ enc[38] ^ enc[37] ^ enc[36] ^ enc[35] ^ enc[34] ^ enc[33] ^ enc[32]



                                                                                                 ;
    enc_pari[6] = enc[71] ^ enc[70] ^ enc[69] ^ enc[68] ^ enc[67] ^ enc[66] ^ enc[65] ^ enc[64]







                                                                                                 ;
    enc_pari[7] = enc[71] ^ enc[70] ^ enc[69] ^ enc[68] ^ enc[67] ^ enc[66] ^ enc[65] ^ enc[64] ^
                  enc[63] ^ enc[62] ^ enc[61] ^ enc[60] ^ enc[59] ^ enc[58] ^ enc[57] ^ enc[56] ^
                  enc[55] ^ enc[54] ^ enc[53] ^ enc[52] ^ enc[51] ^ enc[50] ^ enc[49] ^ enc[48] ^
                  enc[47] ^ enc[46] ^ enc[45] ^ enc[44] ^ enc[43] ^ enc[42] ^ enc[41] ^ enc[40] ^
                  enc[39] ^ enc[38] ^ enc[37] ^ enc[36] ^ enc[35] ^ enc[34] ^ enc[33] ^ enc[32] ^
                  enc[31] ^ enc[30] ^ enc[29] ^ enc[28] ^ enc[27] ^ enc[26] ^ enc[25] ^ enc[24] ^
                  enc[23] ^ enc[22] ^ enc[21] ^ enc[20] ^ enc[19] ^ enc[18] ^ enc[17] ^ enc[16] ^
                  enc[15] ^ enc[14] ^ enc[13] ^ enc[12] ^ enc[11] ^ enc[10] ^ enc[ 9] ^ enc[ 8] ^
                  enc[ 7] ^ enc[ 6] ^ enc[ 5] ^ enc[ 4] ^ enc[ 3] ^ enc[ 2] ^ enc[ 1] ^ enc[ 0] ^
                  enc_pari[6] ^ enc_pari[5] ^ enc_pari[4] ^ enc_pari[3] ^ enc_pari[2] ^ enc_pari[1] ^ enc_pari[0] ;

    *ecc = 0;
    for(i=0; i<8; i++)
    {
        *ecc = *ecc | ((uint8_t)enc_pari[i]) << i;
    }
}

