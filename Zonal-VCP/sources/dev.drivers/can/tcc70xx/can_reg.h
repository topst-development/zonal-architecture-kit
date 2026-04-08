// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : can_reg.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#ifndef MCU_BSP_CAN_REG_HEADER
#define MCU_BSP_CAN_REG_HEADER

#if ( MCU_BSP_SUPPORT_DRIVER_CAN == 1 )

/**************************************************************************************************
*                                           INCLUDE FILES
**************************************************************************************************/


/**************************************************************************************************
*                                            DEFINITIONS
**************************************************************************************************/

/**************************************************************************************************
*
*                                      CAN Controller Register MAP
*
**************************************************************************************************/
typedef struct CANRegFieldCREL
{
    SALReg32                            rfDAY       :8; //[7:0]
    SALReg32                            rfMON       :8; //[15:8]
    SALReg32                            rfYEAR      :4; //[19:16]
    SALReg32                            rfSUBSTEP   :4; //[23:20]
    SALReg32                            rfSTEP      :4; //[27:24]
    SALReg32                            rfREL       :4; //[31:28]
} CANRegFieldCREL_t;

typedef union CANRegCREL
{
    SALReg32                            rNReg;
    CANRegFieldCREL_t                   rFReg;
} CANRegCREL_t;

typedef struct CANRegFieldENDN
{
    SALReg32                            rfETV       :32; //[31:0]
} CANRegFieldENDN_t;

typedef union CANRegENDN
{
    SALReg32                            rNReg;
    CANRegFieldENDN_t                   rFReg;
} CANRegENDN_t;

typedef struct CANRegFieldDBTP
{
    SALReg32                            rfDSJW      :4; //[3:0]
    SALReg32                            rfDTSEG2    :4; //[7:4]
    SALReg32                            rfDTSEG1    :5; //[12:8]
    SALReg32                            rfRsvd      :3; //[15:13]
    SALReg32                            rfDBRP      :5; //[20:16]
    SALReg32                            rfRsvd1     :2; //[22:21]
    SALReg32                            rfTDC       :1; //[23]
    SALReg32                            rfRsvd2     :8; //[24:31]
} CANRegFieldDBTP_t;

typedef union CANRegDBTP
{
    SALReg32                            rNReg;
    CANRegFieldDBTP_t                   rFReg;
} CANRegDBTP_t;

typedef struct CANRegFieldTEST
{
    SALReg32                            rfRsvd      :4; //[3:0]
    SALReg32                            rfLBCK      :1; //[4]
    SALReg32                            rfTX        :2; //[6:5]
    SALReg32                            rfRX        :1; //[7]
    SALReg32                            rfRsvd1     :24; //[31:8]
} CANRegFieldTEST_t;

typedef union CANRegTEST
{
    SALReg32                            rNReg;
    CANRegFieldTEST_t                   rFReg;
} CANRegTEST_t;

typedef struct CANRegFieldRWD
{
    SALReg32                            rfWDC       :8; //[7:0]
    SALReg32                            rfWDV       :8; //[15:8]
    SALReg32                            rfRsvd      :16; //[31:16]
} CANRegFieldRWD_t;

typedef union CANRegRWD
{
    SALReg32                            rNReg;
    CANRegFieldRWD_t                    rFReg;
} CANRegRWD_t;

typedef struct CANRegFieldCCCR
{
    SALReg32                            rfINIT      :1; //[0]
    SALReg32                            rfCCE       :1; //[1]
    SALReg32                            rfASM       :1; //[2]
    SALReg32                            rfCSA       :1; //[3]
    SALReg32                            rfCSR       :1; //[4]
    SALReg32                            rfMON       :1; //[5]
    SALReg32                            rfDAR       :1; //[6]
    SALReg32                            rfTEST      :1; //[7]
    SALReg32                            rfFDOE      :1; //[8]
    SALReg32                            rfBRSE      :1; //[9]
    SALReg32                            rfRsvd      :2; //[11:10]
    SALReg32                            rfPXHD      :1; //[12]
    SALReg32                            rfEFBI      :1; //[13]
    SALReg32                            rfTXP       :1; //[14]
    SALReg32                            rfNISO      :1; //[15]
    SALReg32                            rfRsvd1     :16; //[31:16]
} CANRegFieldCCCR_t;

typedef union CANRegCCCR
{
    SALReg32                            rNReg;
    CANRegFieldCCCR_t                   rFReg;
} CANRegCCCR_t;

typedef struct CANRegFieldNBTP
{
    SALReg32                            rfNTSEG2    :7; //[6:0]
    SALReg32                            rfRsvd      :1; //[7]
    SALReg32                            rfNTSEG1    :8; //[15:8]
    SALReg32                            rfNBRP      :9; //[24:16]
    SALReg32                            rfNSJW      :7; //[31:25]
} CANRegFieldNBTP_t;

typedef union CANRegNBTP
{
    SALReg32                            rNReg;
    CANRegFieldNBTP_t                   rFReg;
} CANRegNBTP_t;

typedef struct CANRegFieldTSCC
{
    SALReg32                            rfTSS       :2; //[1:0]
    SALReg32                            rfRsvd      :14; //[15:2]
    SALReg32                            rfTCP       :4; //[19:16]
    SALReg32                            rfRsvd1     :12; //[31:20]
} CANRegFieldTSCC_t;

typedef union CANRegTSCC
{
    SALReg32                            rNReg;
    CANRegFieldTSCC_t                   rFReg;
} CANRegTSCC_t;

typedef struct CANRegFieldTSCV
{
    SALReg32                            rfTSC       :16; //[15:0]
    SALReg32                            rfRsvd      :16; //[31:16]
} CANRegFieldTSCV_t;

typedef union CANRegTSCV
{
    SALReg32                            rNReg;
    CANRegFieldTSCV_t                   rFReg;
} CANRegTSCV_t;

typedef struct CANRegFieldTOCC
{
    SALReg32                            rfETOC      :1; //[0]
    SALReg32                            rfTOS       :2; //[2:1]
    SALReg32                            rfRsvd      :13; //[15:3]
    SALReg32                            rfTOP       :16; //[31:16]
} CANRegFieldTOCC_t;

typedef union CANRegTOCC
{
    SALReg32                            rNReg;
    CANRegFieldTOCC_t                   rFReg;
} CANRegTOCC_t;

typedef struct CANRegFieldTOCV
{
    SALReg32                            rfTOC       :16; //[15:0]
    SALReg32                            rfRsvd      :16; //[31:16]
} CANRegFieldTOCV_t;

typedef union CANRegTOCV
{
    SALReg32                            rNReg;
    CANRegFieldTOCV_t                   rFReg;
} CANRegTOCV_t;

typedef struct CANRegFieldECR
{
    SALReg32                            rfTEC       :8; //[7:0]
    SALReg32                            rfREC       :7; //[14:8]
    SALReg32                            rfRP        :1; //[15]
    SALReg32                            rfCEL       :8; //[23:16]
    SALReg32                            rfRsvd      :8; //[31:24]
} CANRegFieldECR_t;

typedef union CANRegECR
{
    SALReg32                            rNReg;
    CANRegFieldECR_t                    rFReg;
} CANRegECR_t;

typedef struct CANRegFieldPSR
{
    SALReg32                            rfLEC       :3; //[2:0]
    SALReg32                            rfACT       :2; //[4:3]
    SALReg32                            rfEP        :1; //[5]
    SALReg32                            rfEW        :1; //[6]
    SALReg32                            rfBO        :1; //[7]
    SALReg32                            rfDLEC      :3; //[10:8]
    SALReg32                            rfRESI      :1; //[11]
    SALReg32                            rfRBRS      :1; //[12]
    SALReg32                            rfRFDF      :1; //[13]
    SALReg32                            rfPXE       :1; //[14]
    SALReg32                            rfRsvd      :1; //[15]
    SALReg32                            rfTDCV      :7; //[22:16]
    SALReg32                            rfRsvd1     :9; //[31:23]
} CANRegFieldPSR_t;

typedef union CANRegPSR
{
    SALReg32                            rNReg;
    CANRegFieldPSR_t                    rFReg;
} CANRegPSR_t;

typedef struct CANRegFieldTDCR
{
    SALReg32                            rfTDCF      :7; //[6:0]
    SALReg32                            rfRsvd      :1; //[7]
    SALReg32                            rfTDCO      :7; //[14:8]
    SALReg32                            rfRsvd1     :17; //[31:15]
} CANRegFieldTDCR_t;

typedef union CANRegTDCR
{
    SALReg32                            rNReg;
    CANRegFieldTDCR_t                   rFReg;
} CANRegTDCR_t;

typedef struct CANRegFieldIR
{
    SALReg32                            rfRF0N      :1; //[0]
    SALReg32                            rfRF0W      :1; //[1]
    SALReg32                            rfRF0F      :1; //[2]
    SALReg32                            rfRF0L      :1; //[3]
    SALReg32                            rfRF1N      :1; //[4]
    SALReg32                            rfRF1W      :1; //[5]
    SALReg32                            rfRF1F      :1; //[6]
    SALReg32                            rfRF1L      :1; //[7]
    SALReg32                            rfHPM       :1; //[8]
    SALReg32                            rfTC        :1; //[9]
    SALReg32                            rfTCF       :1; //[10]
    SALReg32                            rfTFE       :1; //[11]
    SALReg32                            rfTEFN      :1; //[12]
    SALReg32                            rfTEFW      :1; //[13]
    SALReg32                            rfTEFF      :1; //[14]
    SALReg32                            rfTEFL      :1; //[15]
    SALReg32                            rfTSW       :1; //[16]
    SALReg32                            rfMRAF      :1; //[17]
    SALReg32                            rfTOO       :1; //[18]
    SALReg32                            rfDRX       :1; //[19]
    SALReg32                            rfBEC       :1; //[20]
    SALReg32                            rfBEU       :1; //[21]
    SALReg32                            rfELO       :1; //[22]
    SALReg32                            rfEP        :1; //[23]
    SALReg32                            rfEW        :1; //[24]
    SALReg32                            rfBO        :1; //[25]
    SALReg32                            rfWDI       :1; //[26]
    SALReg32                            rfPEA       :1; //[27]
    SALReg32                            rfPED       :1; //[28]
    SALReg32                            rfARA       :1; //[29]
    SALReg32                            rfRsvd      :2; //[31:30]
} CANRegFieldIR_t;

typedef union CANRegIR
{
    SALReg32                            rNReg;
    CANRegFieldIR_t                     rFReg;
} CANRegIR_t;

typedef struct CANRegFieldIE
{
    SALReg32                            rfRF0NE     :1; //[0]
    SALReg32                            rfRF0WE     :1; //[1]
    SALReg32                            rfRF0FE     :1; //[2]
    SALReg32                            rfRF0LE     :1; //[3]
    SALReg32                            rfRF1NE     :1; //[4]
    SALReg32                            rfRF1WE     :1; //[5]
    SALReg32                            rfRF1FE     :1; //[6]
    SALReg32                            rfRF1LE     :1; //[7]
    SALReg32                            rfHPME      :1; //[8]
    SALReg32                            rfTCE       :1; //[9]
    SALReg32                            rfTCFE      :1; //[10]
    SALReg32                            rfTFEE      :1; //[11]
    SALReg32                            rfTEFNE     :1; //[12]
    SALReg32                            rfTEFWE     :1; //[13]
    SALReg32                            rfTEFFE     :1; //[14]
    SALReg32                            rfTEFLE     :1; //[15]
    SALReg32                            rfTSWE      :1; //[16]
    SALReg32                            rfMRAFE     :1; //[17]
    SALReg32                            rfTOOE      :1; //[18]
    SALReg32                            rfDRXE      :1; //[19]
    SALReg32                            rfBECE      :1; //[20]
    SALReg32                            rfBEUE      :1; //[21]
    SALReg32                            rfELOE      :1; //[22]
    SALReg32                            rfEPE       :1; //[23]
    SALReg32                            rfEWE       :1; //[24]
    SALReg32                            rfBOE       :1; //[25]
    SALReg32                            rfWDIE      :1; //[26]
    SALReg32                            rfPEAE      :1; //[27]
    SALReg32                            rfPEDE      :1; //[28]
    SALReg32                            rfARAE      :1; //[29]
    SALReg32                            rfRsvd      :2; //[31:30]
} CANRegFieldIE_t;

typedef union CANRegIE
{
    SALReg32                            rNReg;
    CANRegFieldIE_t                     rFReg;
} CANRegIE_t;

typedef struct CANRegFieldILS
{
    SALReg32                            rfRF0NL     :1; //[0]
    SALReg32                            rfRF0WL     :1; //[1]
    SALReg32                            rfRF0FL     :1; //[2]
    SALReg32                            rfRF0LL     :1; //[3]
    SALReg32                            rfRF1NL     :1; //[4]
    SALReg32                            rfRF1WL     :1; //[5]
    SALReg32                            rfRF1FL     :1; //[6]
    SALReg32                            rfRF1LL     :1; //[7]
    SALReg32                            rfHPML      :1; //[8]
    SALReg32                            rfTCL       :1; //[9]
    SALReg32                            rfTCFL      :1; //[10]
    SALReg32                            rfTFEL      :1; //[11]
    SALReg32                            rfTEFNL     :1; //[12]
    SALReg32                            rfTEFWL     :1; //[13]
    SALReg32                            rfTEFFL     :1; //[14]
    SALReg32                            rfTEFLL     :1; //[15]
    SALReg32                            rfTSWL      :1; //[16]
    SALReg32                            rfMRAFL     :1; //[17]
    SALReg32                            rfTOOL      :1; //[18]
    SALReg32                            rfDRXL      :1; //[19]
    SALReg32                            rfBECL      :1; //[20]
    SALReg32                            rfBEUL      :1; //[21]
    SALReg32                            rfELOL      :1; //[22]
    SALReg32                            rfEPL       :1; //[23]
    SALReg32                            rfEWL       :1; //[24]
    SALReg32                            rfBOL       :1; //[25]
    SALReg32                            rfWDIL      :1; //[26]
    SALReg32                            rfPEAL      :1; //[27]
    SALReg32                            rfPEDL      :1; //[28]
    SALReg32                            rfARAL      :1; //[29]
    SALReg32                            rfRsvd      :2; //[31:30]
} CANRegFieldILS_t;

typedef union CANRegILS
{
    SALReg32                            rNReg;
    CANRegFieldILS_t                    rFReg;
} CANRegILS_t;

typedef struct CANRegFieldILE
{
    SALReg32                            rfEINT0     :1; //[0]
    SALReg32                            rfEINT1     :1; //[1]
    SALReg32                            rfRsvd      :30; //[31:2]
} CANRegFieldILE_t;

typedef union CANRegILE
{
    SALReg32                            rNReg;
    CANRegFieldILE_t                    rFReg;
} CANRegILE_t;

typedef struct CANRegFieldGFC
{
    SALReg32                            rfRRFE      :1; //[0]
    SALReg32                            rfRRFS      :1; //[1]
    SALReg32                            rfANFE      :2; //[3:2]
    SALReg32                            rfANFS      :2; //[5:4]
    SALReg32                            rfRsvd      :26; //[31:6]
} CANRegFieldGFC_t;

typedef union CANRegGFC
{
    SALReg32                            rNReg;
    CANRegFieldGFC_t                    rFReg;
} CANRegGFC_t;

typedef struct CANRegFieldSIDFC
{
    SALReg32                            rfRsvd      :2; //[1:0]
    SALReg32                            rfFLSSA     :14; //[15:2]
    SALReg32                            rfLSS       :8; //[23:16]
    SALReg32                            rfRsvd1     :8; //[31:24]
} CANRegFieldSIDFC_t;

typedef union CANRegSIDFC
{
    SALReg32                            rNReg;
    CANRegFieldSIDFC_t                  rFReg;
} CANRegSIDFC_t;

typedef struct CANRegFieldXIDFC
{
    SALReg32                            rfRsvd      :2; //[1:0]
    SALReg32                            rfFLESA     :14; //[15:2]
    SALReg32                            rfLSE       :7; //[22:16]
    SALReg32                            rfRsvd1     :9; //[31:23]
} CANRegFieldXIDFC_t;

typedef union CANRegXIDFC
{
    SALReg32                            rNReg;
    CANRegFieldXIDFC_t                  rFReg;
} CANRegXIDFC_t;

typedef struct CANRegFieldXIDAM
{
    SALReg32                            rfEIDM      :29; //[28:0]
    SALReg32                            rfRsvd      :3; //[31:29]
} CANRegFieldXIDAM_t;

typedef union CANRegXIDAM
{
    SALReg32                            rNReg;
    CANRegFieldXIDAM_t                  rFReg;
} CANRegXIDAM_t;

typedef struct CANRegFieldHPMS
{
    SALReg32                            rfBIDX      :6; //[5:0]
    SALReg32                            rfMSI       :2; //[7:6]
    SALReg32                            rfFIDX      :7; //[14:8]
    SALReg32                            rfFLST      :1; //[15]
    SALReg32                            rfRsvd      :16; //[31:16]
} CANRegFieldHPMS_t;

typedef union CANRegHPMS
{
    SALReg32                            rNReg;
    CANRegFieldHPMS_t                   rFReg;
} CANRegHPMS_t;

typedef struct CANRegFieldNDAT1
{
    SALReg32                            rfND0       :1; //[0]
    SALReg32                            rfND1       :1; //[1]
    SALReg32                            rfND2       :1; //[2]
    SALReg32                            rfND3       :1; //[3]
    SALReg32                            rfND4       :1; //[4]
    SALReg32                            rfND5       :1; //[5]
    SALReg32                            rfND6       :1; //[6]
    SALReg32                            rfND7       :1; //[7]
    SALReg32                            rfND8       :1; //[8]
    SALReg32                            rfND9       :1; //[9]
    SALReg32                            rfND10      :1; //[10]
    SALReg32                            rfND11      :1; //[11]
    SALReg32                            rfND12      :1; //[12]
    SALReg32                            rfND13      :1; //[13]
    SALReg32                            rfND14      :1; //[14]
    SALReg32                            rfND15      :1; //[15]
    SALReg32                            rfND16      :1; //[16]
    SALReg32                            rfND17      :1; //[17]
    SALReg32                            rfND18      :1; //[18]
    SALReg32                            rfND19      :1; //[19]
    SALReg32                            rfND20      :1; //[20]
    SALReg32                            rfND21      :1; //[21]
    SALReg32                            rfND22      :1; //[22]
    SALReg32                            rfND23      :1; //[23]
    SALReg32                            rfND24      :1; //[24]
    SALReg32                            rfND25      :1; //[25]
    SALReg32                            rfND26      :1; //[26]
    SALReg32                            rfND27      :1; //[27]
    SALReg32                            rfND28      :1; //[28]
    SALReg32                            rfND29      :1; //[29]
    SALReg32                            rfND30      :1; //[30]
    SALReg32                            rfND31      :1; //[31]
} CANRegFieldNDAT1_t;

typedef union CANRegNDAT1
{
    SALReg32                            rNReg;
    CANRegFieldNDAT1_t                  rFReg;
} CANRegNDAT1_t;

typedef struct CANRegFieldNDAT2
{
    SALReg32                            rfND32      :1; //[0]
    SALReg32                            rfND33      :1; //[1]
    SALReg32                            rfND34      :1; //[2]
    SALReg32                            rfND35      :1; //[3]
    SALReg32                            rfND36      :1; //[4]
    SALReg32                            rfND37      :1; //[5]
    SALReg32                            rfND38      :1; //[6]
    SALReg32                            rfND39      :1; //[7]
    SALReg32                            rfND40      :1; //[8]
    SALReg32                            rfND41      :1; //[9]
    SALReg32                            rfND42      :1; //[10]
    SALReg32                            rfND43      :1; //[11]
    SALReg32                            rfND44      :1; //[12]
    SALReg32                            rfND45      :1; //[13]
    SALReg32                            rfND46      :1; //[14]
    SALReg32                            rfND47      :1; //[15]
    SALReg32                            rfND48      :1; //[16]
    SALReg32                            rfND49      :1; //[17]
    SALReg32                            rfND50      :1; //[18]
    SALReg32                            rfND51      :1; //[19]
    SALReg32                            rfND52      :1; //[20]
    SALReg32                            rfND53      :1; //[21]
    SALReg32                            rfND54      :1; //[22]
    SALReg32                            rfND55      :1; //[23]
    SALReg32                            rfND56      :1; //[24]
    SALReg32                            rfND57      :1; //[25]
    SALReg32                            rfND58      :1; //[26]
    SALReg32                            rfND59      :1; //[27]
    SALReg32                            rfND60      :1; //[28]
    SALReg32                            rfND61      :1; //[29]
    SALReg32                            rfND62      :1; //[30]
    SALReg32                            rfND63      :1; //[31]
} CANRegFieldNDAT2_t;

typedef union CANRegNDAT2
{
    SALReg32                            rNReg;
    CANRegFieldNDAT2_t                  rFReg;
} CANRegNDAT2_t;

typedef struct CANRegFieldRXFxC
{
    SALReg32                            rfRsvd      :2; //[1:0]
    SALReg32                            rfFxSA      :14; //[15:2]
    SALReg32                            rfFxS       :7; //[22:16]
    SALReg32                            rfRsvd1     :1; //[23]
    SALReg32                            rfFxWM      :7; //[30:24]
    SALReg32                            rfFxOM      :1; //[31]
} CANRegFieldRXFxC_t;

typedef union CANRegRXFxC
{
    SALReg32                            rNReg;
    CANRegFieldRXFxC_t                  rFReg;
} CANRegRXFxC_t;

typedef struct CANRegFieldRXFxS
{
    SALReg32                            rfFxFL      :7; //[6:0]
    SALReg32                            rfRsvd      :1; //[7]
    SALReg32                            rfFxGI      :6; //[13:8]
    SALReg32                            rfRsvd1     :2; //[15:14]
    SALReg32                            rfFxPI      :6; //[21:16]
    SALReg32                            rfRsvd2     :2; //[23:22]
    SALReg32                            rfFxF       :1; //[24]
    SALReg32                            rfRFxL      :1; //[25]
    SALReg32                            rfRsvd3     :4; //[29:26]
    SALReg32                            rfDMS       :2; //[31:30] only use in FIFO 1
} CANRegFieldRXFxS_t;

typedef union CANRegRXFxS
{
    SALReg32                            rNReg;
    CANRegFieldRXFxS_t                  rFReg;
} CANRegRXFxS_t;

typedef struct CANRegFieldRXFxA
{
    SALReg32                            rfFxAI      :6; //[5:0]
    SALReg32                            rfRsvd      :26; //[31:6]
} CANRegFieldRXFxA_t;

typedef union CANRegRXFxA
{
    SALReg32                            rNReg;
    CANRegFieldRXFxA_t                  rFReg;
} CANRegRXFxA_t;

typedef struct CANRegFieldRXBC
{
    SALReg32                            rfRsvd      :2; //[1:0]
    SALReg32                            rfRBSA      :14; //[15:2]
    SALReg32                            rfRsvd1     :16; //[31:16]
} CANRegFieldRXBC_t;

typedef union CANRegRXBC
{
    SALReg32                            rNReg;
    CANRegFieldRXBC_t                   rFReg;
} CANRegRXBC_t;

typedef struct CANRegFieldRXESC
{
    SALReg32                            rfF0DS      :3; //[2:0]
    SALReg32                            rfRsvd      :1; //[3]
    SALReg32                            rfF1DS      :3; //[6:4]
    SALReg32                            rfRsvd1     :1; //[7]
    SALReg32                            rfRBDS      :3; //[10:8]
    SALReg32                            rfRsvd2     :21; //[31:11]
} CANRegFieldRXESC_t;

typedef union CANRegRXESC
{
    SALReg32                            rNReg;
    CANRegFieldRXESC_t                  rFReg;
} CANRegRXESC_t;

typedef struct CANRegFieldTXBC
{
    SALReg32                            rfRsvd      :2; //[1:0]
    SALReg32                            rfTBSA      :14; //[15:2]
    SALReg32                            rfNDTB      :6; //[21:16]
    SALReg32                            rfRsvd1     :2; //[23:22]
    SALReg32                            rfTFQS      :6; //[29:24]
    SALReg32                            rfTFQM      :1; //[30]
    SALReg32                            rfRsvd2     :1; //[31]
} CANRegFieldTXBC_t;

typedef union CANRegTXBC
{
    SALReg32                            rNReg;
    CANRegFieldTXBC_t                   rFReg;
} CANRegTXBC_t;

typedef struct CANRegFieldTXFQS
{
    SALReg32                            rfTFFL      :6; //[5:0]
    SALReg32                            rfRsvd      :2; //[7:6]
    SALReg32                            rfTFGI      :6; //[13:8]
    SALReg32                            rfRsvd1     :2; //[15:14]
    SALReg32                            rfTFQPI     :5; //[20:16]
    SALReg32                            rfTFQF      :1; //[21]
    SALReg32                            rfRsvd2     :10; //[31:22]
} CANRegFieldTXFQS_t;

typedef union CANRegTXFQS
{
    SALReg32                            rNReg;
    CANRegFieldTXFQS_t                  rFReg;
} CANRegTXFQS_t;

typedef struct CANRegFieldTXESC
{
    SALReg32                            rfTBDS      :3; //[2:0]
    SALReg32                            rfRsvd      :29; //[31:3]
} CANRegFieldTXESC_t;

typedef union CANRegTXESC
{
    SALReg32                            rNReg;
    CANRegFieldTXESC_t                  rFReg;
} CANRegTXESC_t;

typedef struct CANRegFieldTXBRP
{
    SALReg32                            rfTRP0      :1; //[0]
    SALReg32                            rfTRP1      :1; //[1]
    SALReg32                            rfTRP2      :1; //[2]
    SALReg32                            rfTRP3      :1; //[3]
    SALReg32                            rfTRP4      :1; //[4]
    SALReg32                            rfTRP5      :1; //[5]
    SALReg32                            rfTRP6      :1; //[6]
    SALReg32                            rfTRP7      :1; //[7]
    SALReg32                            rfTRP8      :1; //[8]
    SALReg32                            rfTRP9      :1; //[9]
    SALReg32                            rfTRP10     :1; //[10]
    SALReg32                            rfTRP11     :1; //[11]
    SALReg32                            rfTRP12     :1; //[12]
    SALReg32                            rfTRP13     :1; //[13]
    SALReg32                            rfTRP14     :1; //[14]
    SALReg32                            rfTRP15     :1; //[15]
    SALReg32                            rfTRP16     :1; //[16]
    SALReg32                            rfTRP17     :1; //[17]
    SALReg32                            rfTRP18     :1; //[18]
    SALReg32                            rfTRP19     :1; //[19]
    SALReg32                            rfTRP20     :1; //[20]
    SALReg32                            rfTRP21     :1; //[21]
    SALReg32                            rfTRP22     :1; //[22]
    SALReg32                            rfTRP23     :1; //[23]
    SALReg32                            rfTRP24     :1; //[24]
    SALReg32                            rfTRP25     :1; //[25]
    SALReg32                            rfTRP26     :1; //[26]
    SALReg32                            rfTRP27     :1; //[27]
    SALReg32                            rfTRP28     :1; //[28]
    SALReg32                            rfTRP29     :1; //[29]
    SALReg32                            rfTRP30     :1; //[30]
    SALReg32                            rfTRP31     :1; //[31]
} CANRegFieldTXBRP_t;

typedef union CANRegTXBRP
{
    SALReg32                            rNReg;
    CANRegFieldTXBRP_t                  rFReg;
} CANRegTXBRP_t;

typedef struct CANRegFieldTXEFC
{
    SALReg32                            rfRsvd      :2; //[1:0]
    SALReg32                            rfEFSA      :14; //[15:2]
    SALReg32                            rfEFS       :6; //[21:16]
    SALReg32                            rfRsvd1     :2; //[23:22]
    SALReg32                            rfEFWM      :6; //[29:24]
    SALReg32                            rfRsvd2     :2; //[31:30]
} CANRegFieldTXEFC_t;

typedef union CANRegTXEFC
{
    SALReg32                            rNReg;
    CANRegFieldTXEFC_t                  rFReg;
} CANRegTXEFC_t;

typedef struct CANRegFieldTXEFS
{
    SALReg32                            rfEFFL      :6; //[5:0]
    SALReg32                            rfRsvd      :2; //[7:6]
    SALReg32                            rfEFGI      :5; //[12:8]
    SALReg32                            rfRsvd1     :3; //[15:13]
    SALReg32                            rfEFPI      :5; //[20:16]
    SALReg32                            rfRsvd2     :3; //[23:21]
    SALReg32                            rfEFF       :1; //[24]
    SALReg32                            rfTEFL      :1; //[25]
    SALReg32                            rfRsvd3     :6; //[31:26]
} CANRegFieldTXEFS_t;

typedef union CANRegTXEFS
{
    SALReg32                            rNReg;
    CANRegFieldTXEFS_t                  rFReg;
} CANRegTXEFS_t;

typedef struct CANRegFieldTXEFA
{
    SALReg32                            rfEFAI      :5; //[4:0]
    SALReg32                            rfRsvd      :27; //[31:5]
} CANRegFieldTXEFA_t;

typedef union CANRegTXEFA
{
    SALReg32                            rNReg;
    CANRegFieldTXEFA_t                  rFReg;
} CANRegTXEFA_t;


/**************************************************************************************************
*
*                                      CAN Configuration Register MAP
*
**************************************************************************************************/
typedef struct CANRegFieldBaseAddr
{
    SALReg32                            rfBASE_ADDR :16; //[15:0]
    SALReg32                            rfRsvd      :16; //[31:16]
} CANRegFieldBaseAddr_t;

typedef union CANRegBaseAddr
{
    SALReg32                            rNReg;
    CANRegFieldBaseAddr_t               rFReg;
} CANRegBaseAddr_t;

typedef struct CANRegFieldExtTSCtrl0
{
    SALReg32                            rfENABLE    :1; //[0]
    SALReg32                            rfCLEAR     :1; //[1]
    SALReg32                            rfRsvd      :30; //[31:2]
} CANRegFieldExtTSCtrl0_t;

typedef union CANRegExtTSCtrl0
{
    SALReg32                            rNReg;
    CANRegFieldExtTSCtrl0_t             rFReg;
} CANRegExtTSCtrl0_t;

typedef struct CANRegFieldExtTSCtrl1
{
    SALReg32                            rfEXT_TIMESTAMP_DIV_RATIO :16; //[15:0]
    SALReg32                            rfEXT_TIMESTAMP_COMP      :16; //[31:16]
} CANRegFieldExtTSCtrl1_t;

typedef union CANRegExtTSCtrl1
{
    SALReg32                            rNReg;
    CANRegFieldExtTSCtrl1_t             rFReg;
} CANRegExtTSCtrl1_t;

#if 0 //unused structure
typedef struct CANRegExtTSValue
{
    SALReg32                            rNReg;
} CANRegExtTSValue_t;

typedef struct CANRegCfgWrPw
{
    SALReg32                            rNReg;
} CANRegCfgWrPw_t;

typedef struct CANRegFieldCfgWrLock
{
    SALReg32                            rfCFG_WR_LOCK   :1; //[0]
    SALReg32                            rfRsvd          :31; //[31:1]
} CANRegFieldCfgWrLock_t;

typedef union CANRegCfgWrLock
{
    SALReg32                            rNReg;
    CANRegFieldCfgWrLock_t              rFReg;
} CANRegCfgWrLock_t;
#endif

/**************************************************************************************************
*
*                                      CAN Message RAM MAP
*
**************************************************************************************************/
typedef struct CANRamFieldRxBufferR0
{
    SALReg32                            rfID        :29; //[28:0]
    SALReg32                            rfRTR       :1; //[29]
    SALReg32                            rfXTD       :1; //[30]
    SALReg32                            rfESI       :1; //[31]
} CANRamFieldRxBufferR0_t;

typedef union CANRamRxBufferR0
{
    SALReg32                            rNReg;
    CANRamFieldRxBufferR0_t             rFReg;
} CANRamRxBufferR0_t;

typedef struct CANRamFieldRxBufferR1
{
    SALReg32                            rfRXTS      :16; //[15:0]
    SALReg32                            rfDLC       :4; //[19:16]
    SALReg32                            rfBRS       :1; //[20]
    SALReg32                            rfFDF       :1; //[21]
    SALReg32                            rfRsvd      :2; //[23:22]
    SALReg32                            rfFIDX      :7; //[30:24]
    SALReg32                            rfANMF      :1; //[31]
} CANRamFieldRxBufferR1_t;

typedef union CANRamRxBufferR1
{
    SALReg32                            rNReg;
    CANRamFieldRxBufferR1_t             rFReg;
} CANRamRxBufferR1_t;

typedef struct CANRamRxBuffer
{
    CANRamRxBufferR0_t                  rR0;
    CANRamRxBufferR1_t                  rR1;
    uint32                              rData[CAN_DATA_LENGTH_SIZE / sizeof(uint32)];
} CANRamRxBuffer_t;

typedef struct CANRamFieldTxBufferT0
{
    SALReg32                            rfID        :29; //[28:0]
    SALReg32                            rfRTR       :1; //[29]
    SALReg32                            rfXTD       :1; //[30]
    SALReg32                            rfESI       :1; //[31]
} CANRamFieldTxBufferT0_t;

typedef union CANRamTxBufferT0
{
    SALReg32                            rNReg;
    CANRamFieldTxBufferT0_t             rFReg;
} CANRamTxBufferT0_t;

typedef struct CANRamFieldTxBufferT1
{
    SALReg32                            rfRsvd      :16; //[15:0]
    SALReg32                            rfDLC       :4; //[19:16]
    SALReg32                            rfBRS       :1; //[20]
    SALReg32                            rfFDF       :1; //[21]
    SALReg32                            rfRsvd1     :1; //[22]
    SALReg32                            rfEFC       :1; //[23]
    SALReg32                            rfMM        :8; //[31:24]
} CANRamFieldTxBufferT1_t;

typedef union CANRamTxBufferT1
{
    SALReg32                            rNReg;
    CANRamFieldTxBufferT1_t             rFReg;
} CANRamTxBufferT1_t;

typedef struct CANRamTxBuffer
{
    CANRamTxBufferT0_t                  rT0;
    CANRamTxBufferT1_t                  rT1;
    uint32                              rData[16];
} CANRamTxBuffer_t;

typedef struct CANRamFieldStdIDFilterElement
{
    SALReg32                            rfSFID2     :11; //[10:0]
    SALReg32                            rfRsvd      :5; //[15:11]
    SALReg32                            rfSFID1     :11; //[26:16]
    SALReg32                            rfSFEC      :3; //[29:27]
    SALReg32                            rfSFT       :2; //[31:30]
} CANRamFieldStdIDFilterElement_t;

typedef union CANRamStdIDFilterElement
{
    SALReg32                            rNReg;
    CANRamFieldStdIDFilterElement_t     rFReg;
} CANRamStdIDFilterElement_t;

typedef struct CANRamFieldExtIDFilterF0Element
{
    SALReg32                            rfEFID1     :29; //[28:0]
    SALReg32                            rfEFEC      :3; //[31:29]
} CANRamFieldExtIDFilterF0Element_t;

typedef union CANRamExtIDFilterF0Element
{
    SALReg32                            rNReg;
    CANRamFieldExtIDFilterF0Element_t   rFReg;
} CANRamExtIDFilterF0Element_t;

typedef struct CANRamFieldExtIDFilterF1Element
{
    SALReg32                            rfEFID2     :29; //[28:0]
    SALReg32                            rfRsvd      :1; //[29]
    SALReg32                            rfEFT       :2; //[31:30]
} CANRamFieldExtIDFilterF1Element_t;

typedef union CANRamExtIDFilterF1Element
{
    SALReg32                            rNReg;
    CANRamFieldExtIDFilterF1Element_t   rFReg;
} CANRamExtIDFilterF1Element_t;

typedef struct CANRamExtIDFilterElement
{
    CANRamExtIDFilterF0Element_t        rF0;
    CANRamExtIDFilterF1Element_t        rF1;
} CANRamExtIDFilterElement_t;

typedef struct CANRamFieldTxEventFIFOE0
{
    SALReg32                            rfID        :29; //[28:0]
    SALReg32                            rfRTR       :1; //[29]
    SALReg32                            rfXTD       :1; //[30]
    SALReg32                            rfESI       :1; //[31]
} CANRamFieldTxEventFIFOE0_t;

typedef union CANRamTxEventFIFOE0
{
    SALReg32                            rNReg;
    CANRamFieldTxEventFIFOE0_t          rFReg;
} CANRamTxEventFIFOE0_t;

typedef struct CANRamFieldTxEventFIFOE1
{
    SALReg32                            rfTXTS  :16; //[15:0]
    SALReg32                            rfDLC   :4; //[19:16]
    SALReg32                            rfBRS   :1; //[20]
    SALReg32                            rfFDF   :1; //[21]
    SALReg32                            rfET    :2; //[23:22]
    SALReg32                            rfMM    :8; //[31:24]
} CANRamFieldTxEventFIFOE1_t;

typedef union CANRamTxEventFIFOE1
{
    SALReg32                            rNReg;
    CANRamFieldTxEventFIFOE1_t          rFReg;
} CANRamTxEventFIFOE1_t;

typedef struct CANRamTxEventFIFO
{
    CANRamTxEventFIFOE0_t               rE0;
    CANRamTxEventFIFOE1_t               rE1;
} CANRamTxEventFIFO_t;

typedef struct CANControllerRegister
{
    CANRegCREL_t                        crCoreRelease;
    CANRegENDN_t                        crEndian;
    uint32                              crRsvd;
    CANRegDBTP_t                        crDataBitTiming;
    CANRegTEST_t                        crTest;
    CANRegRWD_t                         crRAMWatchdog;
    CANRegCCCR_t                        crCCControl;
    CANRegNBTP_t                        crNominalBitTiming;
    CANRegTSCC_t                        crTimestampCounterConfiguration;
    CANRegTSCV_t                        crTimestampCounterValue;
    CANRegTOCC_t                        crTimeoutCounterConfiguration;
    CANRegTOCV_t                        crTimeoutCounterValue;
    uint32                              crRsvd1[4];
    CANRegECR_t                         crErrorCounterRegister;
    CANRegPSR_t                         crProtocolStatusRegister;
    CANRegTDCR_t                        crTransmitterDelayCompensation;
    uint32                              crRsvd2;
    CANRegIR_t                          crInterruptRegister;
    CANRegIE_t                          crInterruptEnable;
    CANRegILS_t                         crInterruptLineSelect;
    CANRegILE_t                         crInterruptLineEnable;
    uint32                              crRsvd3[8];
    CANRegGFC_t                         crGlobalFilterConfiguration;
    CANRegSIDFC_t                       crStandardIDFilterConfiguration;
    CANRegXIDFC_t                       crExtendedIDFilterConfiguration;
    uint32                              crRsvd4;
    CANRegXIDAM_t                       crExtendedIDANDMask;
    CANRegHPMS_t                        crHighPriorityMessageStatus;
    CANRegNDAT1_t                       crNewData1;
    CANRegNDAT2_t                       crNewData2;
    CANRegRXFxC_t                       crRxFIFO0Configuration;
    CANRegRXFxS_t                       crRxFIFO0Status;
    CANRegRXFxA_t                       crRxFIFO0Acknowledge;
    CANRegRXBC_t                        crRxBufferConfiguration;
    CANRegRXFxC_t                       crRxFIFO1Configuration;
    CANRegRXFxS_t                       crRxFIFO1Status;
    CANRegRXFxA_t                       crRxFIFO1Acknowledge;
    CANRegRXESC_t                       crRxBufferOrFIFOElementSizeConfiguration;
    CANRegTXBC_t                        crTxBufferConfiguration;
    CANRegTXFQS_t                       crTxFIFOOrQueueStatus;
    CANRegTXESC_t                       crTxBufferElementSizeConfiguration;
    CANRegTXBRP_t                       crTxBufferRequestPending;
    uint32                              crTxBufferAddRequest;
    uint32                              crTxBufferCancellationRequest;
    uint32                              crTxBufferTransmissionOccurred;
    uint32                              crTxBufferCancellationFinished;
    uint32                              crTxBufferTransmissionInterruptEnable;
    uint32                              crTxBufferCancellationFinishedInterruptEnable;
    uint32                              crRsvd5[2];
    CANRegTXEFC_t                       crTxEventFIFOConfiguration;
    CANRegTXEFS_t                       crTxEventFIFOStatus;
    CANRegTXEFA_t                       crTxEventFIFOAcknowledge;
} CANControllerRegister_t;

#if 0 //unused structure
typedef struct CANConfigRegister
{
    uint32                              crRsvd;
    CANRegBaseAddr_t                    crBaseAddr[3];
    CANRegExtTSCtrl0_t                  crExtTimeStampCtrl0;
    CANRegExtTSCtrl1_t                  crExtTimeStampCtrl1;
    CANRegExtTSValue_t                  crExtTimeStampValue;
    uint32                              crRsvd1;
    CANRegCfgWrPw_t                     crCfgWrPw;
    CANRegCfgWrLock_t                   crCfgWrLock;
} CANConfigRegister_t;
#endif


/**************************************************************************************************
*                                          LOCAL VARIABLES
**************************************************************************************************/


/**************************************************************************************************
*                                        FUNCTION PROTOTYPES
**************************************************************************************************/

#endif  // ( MCU_BSP_SUPPORT_DRIVER_CAN == 1 )

#endif  // MCU_BSP_CAN_REG_HEADER

