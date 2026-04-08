// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : snor_mio_gdma.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#ifndef _GDMA_H_

#define _GDMA_H_


#define HwIOBUS             0x16000000		// TCC8020
#define HwGDMA0              ((volatile GDMA         *)(HwIOBUS   + 0x020000))

#define GDMA0	0
#define GDMA1	1
#define GDMA2	2
#define GDMA3	3
#define GDMA4	4

#define DMACH0	0
#define DMACH1	1
#define DMACH2	2

//	Source Parameter
/////////////////////////////////////////////////////////
typedef	struct	{
	uint32	SINC		: 8;
	uint32	SMASK		:24;
} SPARAM;

typedef	union {
	uint32		nSPARAM;
	SPARAM		bSPARAM;
} SPARAM_U;

//	Destination Parameter
/////////////////////////////////////////////////////////
typedef	struct	{
	uint32	DINC		: 8;
	uint32	DMASK		:24;
} DPARAM;

typedef	union {
	uint32		nDPARAM;
	DPARAM		bDPARAM;
} DPARAM_U;

//	Channel Control
/////////////////////////////////////////////////////////
#define	DMA_8BIT	0
#define	DMA_16BIT	1
#define	DMA_32BIT	2

#define	DMA_1BST	0
#define	DMA_2BST	1
#define	DMA_4BST	2
#define	DMA_8BST	3

#define	DREQ_EDGE	0
#define	DREQ_HW		1
#define	DREQ_SW		2
#define	DREQ_LEVEL	3

#define	DMARD		0x0
#define	DMAWR		0x1

typedef	struct	{
	uint32	CHEN		: 1;
	uint32	REP			: 1;
	uint32	IEN			: 1;
	uint32	FLG			: 1;
	uint32	WSIZE		: 2;
	uint32	BSIZE		: 2;
	uint32	TYPE		: 2;
	uint32	ARB			: 1;
	uint32	LOCK		: 1;
	uint32	HRD			: 1;
	uint32	SYN			: 1;
	uint32	DTM			: 1;
	uint32	CONT		: 1;
	uint32				:16;
} CHCTRL;

typedef	union {
	uint32		nCHCTRL;
	CHCTRL		bCHCTRL;
} CHCTRL_U;

//	Repeat Control
/////////////////////////////////////////////////////////
typedef	struct	{
	uint32	RPTCNT		:24;
	uint32				: 6;
	uint32	EOT			: 1;
	uint32	DRI			: 1;
} RPTCTRL;

typedef	union {
	uint32		nRPTCTRL;
	RPTCTRL		bRPTCTRL;
} RPTCTRL_U;

//	Channel Configuration
/////////////////////////////////////////////////////////
typedef	struct	{
	uint32	RPTCNT		:24;
	uint32				: 6;
	uint32	EOT			: 1;
	uint32	DRI			: 1;
} CHCOFIG;

typedef	union {
	uint32		nCHCOFIG;
	CHCOFIG		bCHCOFIG;
} CHCOFIG_U;

//	DMA Request Selection
/////////////////////////////////////////////////////////
#define DREQ_GPSB03_TX  0x00000001	// DREQ_GPSB0_TX, DREQ_GPSB3_TX
#define DREQ_GPSB14_TX  0x00000002	// DREQ_GPSB1_TX, DREQ_GPSB4_TX
#define DREQ_GPSB25_TX  0x00000004	// DREQ_GPSB2_TX, DREQ_GPSB5_TX
#define DREQ_TS0       	0x00000008
#define DREQ_GPSB03_RX  0x00000010	// DREQ_GPSB0_RX, DREQ_GPS03_RX
#define DREQ_GPSB14_RX  0x00000020	// DREQ_GPSB1_RX, DREQ_GPSB4_RX
#define DREQ_GPSB25_RX  0x00000040	// DREQ_GPSB2_RX, DREQ_GPSB5_RX
#define DREQ_TS1       	0x00000080
#define DREQ_UTTX26    	0x00000100	// DREQ_UTTX2, DREQ_UTTX6
#define DREQ_UTRX26    	0x00000200	// DREQ_UTRX2, DREQ_UTRX6
#define DREQ_UTTX37    	0x00000400	// DREQ_UTTX3, DREQ_UTTX7
#define DREQ_UTRX37    	0x00000800	// DREQ_UTRX3, DREQ_UTRX7
#define DREQ_SPTX1		0x00001000
#define DREQ_SPTX0		0x00002000
#define DREQ_DATX      	0x00004000
#define DREQ_DARX      	0x00008000
#define DREQ_MSTICK0	0x00010000
#define DREQ_MSTICK1	0x00020000
#define DREQ_NFC       	0x00040000
#define DREQ_I2CS01RX   0x00080000	// DREQ_I2CS0RX, DREQ_I2CS1RX
#define DREQ_MSPTX1		0x00100000
#define DREQ_MSPTX0		0x00200000
#define DREQ_MCDRX     	0x00400000
#define DREQ_MDATX     	0x00800000
#define DREQ_MDARX     	0x01000000
#define DREQ_I2CS01TX  	0x02000000	// DREQ_I2CS0TX, DREQ_I2CS1TX
#define DREQ_UTTX04    	0x04000000	// DREQ_UTTX0, DREQ_UTTX4
#define DREQ_UTRX04    	0x08000000	// DREQ_UTRX0, DREQ_UTRX4
#define DREQ_MSPRX     	0x10000000
#define DREQ_UTTX15    	0x20000000	// DREQ_UTTX1, DREQ_UTTX5
#define DREQ_UTRX15    	0x40000000	// DREQ_UTRX1, DREQ_UTRX5
#define DREQ_TS2       	0x80000000

//	GDMA0 Register
/////////////////////////////////////////////////////////

typedef	struct	_GDMA {
	uint32		uSADDR0;			// 0x00
	SPARAM_U	uSPARAM0;			// 0x04
	uint32		_undef0;			// 0x08
	uint32		uCSADDR0;			// 0x0C
	uint32		uDADDR0;			// 0x10
	DPARAM_U	uDPARAM0;			// 0x14
	uint32		_undef1;			// 0x18
	uint32		uCDADDR0;			// 0x1C
	uint32		uHCOUNT0;			// 0x20
	CHCTRL_U	uCHCTRL0;			// 0x24
	RPTCTRL_U	uRPTCTRL0;			// 0x28
	uint32		uDREQSEL0;			// 0x2C
	uint32		uSADDR1;			// 0x30
	SPARAM_U	uSPARAM1;			// 0x34
	uint32		_undef2;			// 0x38
	uint32		uCSADDR1;			// 0x3C
	uint32		uDADDR1;			// 0x40
	DPARAM_U	uDPARAM1;			// 0x44
	uint32		_undef3;			// 0x48
	uint32		uCDADDR1;			// 0x4C
	uint32		uHCOUNT1;			// 0x50
	CHCTRL_U	uCHCTRL1;			// 0x54
	RPTCTRL_U	uRPTCTRL1;			// 0x58
	uint32		uDREQSEL1;			// 0x5C
	uint32		uSADDR2;			// 0x60
	SPARAM_U	uSPARAM2;			// 0x64
	uint32		_undef4;			// 0x68
	uint32		uCSADDR2;			// 0x6C
	uint32		uDADDR2;			// 0x70
	DPARAM_U	uDPARAM2;			// 0x74
	uint32		_undef5;			// 0x78
	uint32		uCDADDR2;			// 0x7C
	uint32		uHCOUNT2;			// 0x80
	CHCTRL_U	uCHCTRL2;			// 0x84
	RPTCTRL_U	uRPTCTRL2;			// 0x88
	uint32		uDREQSEL2;			// 0x8C
	uint32		_undef6;			// 0x90
}	GDMA;


///////////////////////////////////////////////////////////
//
//	for I2C Controller
//
//////////////////////////////////////////////////////////

//=================================//
//  I2C Channel Control Registers  //
//=================================//
typedef struct {
  uint32  CH0   : 16;  //M0
  uint32  CH1   : 16;  //M1
} CHCFG0;

typedef struct {
  uint32  CH2   : 16;  //M2
  uint32  CH3   : 16;  //M3
} CHCFG1;

typedef struct {
  uint32  CH4   : 16;  //M4
  uint32  CH5   : 16;  //M5
} CHCFG2;


typedef struct {
  uint32  CH6   : 16;  //M6
  uint32  CH7   : 16;  //M7
} CHCFG3;

typedef struct {
  uint32  CH8   : 16;  //S0
  uint32  CH9   : 16;  //S1
} CHCFG4;

typedef struct {
  uint32  CH10  : 16;  //S2
  uint32  CH11  : 16;  //S3
} CHCFG5;

typedef union {
  uint32   nCHCFG0;
  CHCFG0 bCHCFG0;
} CHCFG0_U;

typedef union {
  uint32   nCHCFG1;
  CHCFG1 bCHCFG1;
} CHCFG1_U;

typedef union {
  uint32   nCHCFG2;
  CHCFG2 bCHCFG2;
} CHCFG2_U;

typedef union {
  uint32   nCHCFG3;
  CHCFG3 bCHCFG3;
} CHCFG3_U;

typedef union {
  uint32   nCHCFG4;
  CHCFG4 bCHCFG4;
} CHCFG4_U;

typedef union {
  uint32   nCHCFG5;
  CHCFG5 bCHCFG5;
} CHCFG5_U;


// Channel IRQ Status Register
typedef struct {
  uint32  MIRQ0 : 1;
  uint32  MIRQ1 : 1;
  uint32  MIRQ2 : 1;
  uint32  MIRQ3 : 1;
  uint32  MIRQ4 : 1;
  uint32  MIRQ5 : 1;
  uint32  MIRQ6 : 1;
  uint32  MIRQ7 : 1;
  uint32  SIRQ0 : 1;
  uint32  SIRQ1 : 1;
  uint32  SIRQ2 : 1;
  uint32  SIRQ3 : 1;
  uint32        :20;
} IRQSTR;

typedef union {
  uint32   nIRQSTR;
  IRQSTR bIRQSTR;
} IRQSTR_U;

typedef struct {
  CHCFG0_U     uCHCFG0;  // 0x00
  CHCFG1_U     uCHCFG1;  // 0x04
  CHCFG2_U     uCHCFG2;  // 0x08
  CHCFG3_U     uCHCFG3;  // 0x0C
  CHCFG4_U     uCHCFG4;  // 0x10
  CHCFG5_U     uCHCFG5;  // 0x14
  uint32         :32;  // 0x14
  IRQSTR_U     uIRQSTR;  // 0x1C
} I2CCFG;



//========================//
//  I2C Master Registers  //
//========================//
// TCC8960 has eight  I2C master
// TCC8930 has four I2C master

// Timing Register
typedef struct {
  uint32  FC    : 5;
  uint32  CS    : 1;
  uint32        : 2;
  uint32  RC    : 8;
  uint32        :16;
} TIME;

typedef union {
  uint32  nTIME;
  TIME  bTIME;
} TIME_U;

// Status Register
typedef struct {
  uint32  IF    : 1; // intr flag ( 1 for pending )
  uint32  TIP   : 1; // transfer completed ( 1 for transferring )
  uint32  STR   : 1;
  uint32        : 2;
  uint32  AL    : 1; // arbitration lost ( 1 for lost )
  uint32  BUSY  : 1; // '1' after START signal detected,
                   // '0' for STOP
  uint32  RXACK : 1; // acknowledge received ( 0 for no-ack )
  uint32        :24;
} STS;

typedef union {
  uint32  nSTATUS;
  STS   bSTATUS;
} MSTAT_U;

// Receive Register
typedef struct {
  uint32  RXDATA :16;
  uint32         :16;
} RXDAT;

typedef union {
  uint32  nRXDATA;
  RXDAT bRXDATA;
} RXDATA_U;

// Command Register
typedef struct {
  uint32  IACK  : 1;	// intr ack ( writing '1' for clear )
  //uint32        : 2;
  uint32        : 1;
  uint32  DEL   : 1;	// delay condition generation
  uint32  ACK	  : 1;	// sent acknowledge ( 1 for disabled )
  uint32  WR    : 1;	// write to slave
  uint32  RD    : 1;	// read from slave
  uint32  STO	  : 1;	// stop condition generation
  uint32  STA	  : 1;	// start condition generation
  uint32        :24;
} CMD;

typedef union {
  uint32  nCOMMAND;
  CMD   bCOMMAND;
} COMMAND_U;

// Transmit Register
typedef struct {
  uint32  TXDATA :16;
  uint32         :16;
} TXDAT;

typedef union {
  uint32  nTXDATA;
  TXDAT bTXDATA;
} TXDATA_U;

// Control Register
typedef struct {
  uint32        : 5;
  uint32  MODE  : 1;
  uint32  IEN	  : 1;
  uint32  EN    : 1;
  uint32        :24;
} CTRL_I2C;

typedef union {
  uint32  	nCONTROL;
  CTRL_I2C  bCONTROL;
} CONTROL_U;

// Prescale Register
typedef struct {
  uint32  CPD   :16;
  uint32        :16;
} PRES;

typedef union {
  uint32  nPRESCALE;
  PRES  bPRESCALE;
} PRESCALE_U;

typedef struct {
  uint32  DLY   :16;
  uint32        :16;
} DLY;

typedef union {
  uint32 nDLY;
  DLY  bDLY;
} DLY_U;

typedef struct {
  PRESCALE_U   uPRESCALE;  // 0x00
  CONTROL_U    uCONTROL;   // 0x04
  TXDATA_U     uTXDATA;    // 0x08
  COMMAND_U    uCOMMAND;   // 0x0C
  RXDATA_U     uRXDATA;    // 0x10
  MSTAT_U      uSTATUS;    // 0x14
  TIME_U       uTIME;      // 0x18
  DLY_U        uDLY;       // 0x1C
} I2CM;

//=======================//
//  I2C Slave Registers  //
//=======================//
// TCC8960 has four I2C slave

// Data Buffer 1
typedef struct {
  uint32  DB4   : 8;
  uint32  DB5   : 8;
  uint32  DB6   : 8;
  uint32  DB7   : 8;
} MB1;

typedef union {
  uint32  nMB1;
  MB1   bMB1;
} MB1_U;

// Data Buffer 0
typedef struct {
  uint32  DB0   : 8;
  uint32  DB1   : 8;
  uint32  DB2   : 8;
  uint32  DB3   : 8;
} MB0;

typedef union {
  uint32  nMB0;
  MB0   bMB0;
} MB0_U;

// Buffer Valid Flag Register
typedef struct {
  uint32  MBFR  : 8;
  uint32        : 8;
  uint32  MBFT  : 8;
  uint32        : 8;
} MBF;

typedef union {
  uint32  nMBF;
  MBF   bMBF;
} MBF_U;

// Status Register
typedef struct {
  uint32  SADR  : 8;
  uint32		    : 8;
  uint32  SMCS  : 7;
  uint32  DDIR	: 1;
  uint32        : 8;
} STAT;

typedef union {
  uint32  nSTAT;
  STAT  bSTAT;
} STAT_U;

// Interrupt Register
typedef struct {
  uint32  IRQEN   :14;
  uint32          : 2;
  uint32  IRQSTAT :14;
  uint32          : 2;
} INT;

typedef union {
  uint32  nINT;
  INT   bINT;
} INT_U;

// Address Register
typedef struct {
	uint32	    	: 1;
	uint32	ADDR	: 7;
	uint32		    :24;
} ADDR;

typedef union {
  uint32  nADDR;
  ADDR  bADDR;
} ADDR_U;

// Control register
typedef struct {
  uint32	EN	  : 1;
  uint32	AM	  : 1; //address mode
  uint32	CLR	  : 1; //FIFO control (clear)
  uint32	SDA	  : 1; //SDA control
  uint32	WS    : 1; //SCL wait control
  uint32	RCLR  : 1; //Clear int
  uint32	WSM	  : 1; //SCL wait for MB
  uint32 	drqm  : 1; //DRQ control for MB
  uint32 	RXTH  : 2;
  uint32		  : 2;
  uint32	TXTH  : 2;
  uint32		  : 2;
  uint32	DRQEN : 4;
  uint32	FC	  : 9;
  uint32  FM    : 1; //Counter(Filter) Control
  uint32	SLV   : 2; //Master Signal Control
} CTL;

typedef union {
  uint32  nCTL;
  CTL   bCTL;
} CTL_U;

// Data Port register
typedef struct {
  uint32  PORT : 8;
  uint32       : 8;
  uint32  RXVC : 3;
  uint32       : 5;
  uint32  TXVC : 3;
  uint32       : 5;
} PORT;

typedef union {
  uint32  nPORT;
  PORT  bPORT;
} PORT_U;

typedef struct {
  PORT_U       uPORT;  // 0x80
  CTL_U        uCTL;   // 0x84
  ADDR_U       uADDR;  // 0x88
  INT_U        uINT;   // 0x8C
  STAT_U       uSTAT;  // 0x90
  uint32         undef1; // 0x94
  uint32         undef2; // 0x98
  MBF_U        uMBF;   // 0x9C
  MB0_U        uMB0;   // 0xA0
  MB1_U        uMB1;   // 0xA4
} I2CS;

typedef	struct	_I2C_CH {
	uint32	nPRESCALE;			// 16 bits data [15:00]
	union	{
		uint32	nCONTROL;
		struct	_I2C_CONTROL {
			uint32	_reser0	: 5;
			uint32	MODE	: 1;
			uint32	IEN	: 1;
			uint32	EN	: 1;
			uint32	_undef0	:24;
		}	bCONTROL;
	}	uCONTROL;
	uint32	nTXDATA;			// 16 bits data [15:00]
	union	{
		uint32	nCOMMAND;
		struct	_I2C_COMMAND {
			uint32	IACK	: 1;	// interrupt acknowledge ( writing '1' for clear )
			uint32	_reser0	: 1;
			//uint32	_reser0	: 2;
			//uint32    DEL : 0;	// delay condition generation
			uint32     : 1;	// delay condition generation
			uint32	ACK	: 1;	// sent acknowledge ( 1 for disabled )
			uint32	WR	: 1;	// write to slave
			uint32	RD	: 1;	// read from slave
			uint32	STO	: 1;	// stop condition generation
			uint32	STA	: 1;	// start condition generation
			uint32	_undef0	:24;
		}	bCOMMAND;
	}	uCOMMAND;
	uint32	nRXDATA;			// 16 bits data [15:00]
	union	{
		uint32	nSTATUS;
		struct	_I2C_STATUS {
			uint32	IF	: 1;	// interrupt flag ( 1 for pending )
			uint32	TIP	: 1;	// transfer completed ( 0 for transferring )
  			uint32  	STR   : 1; // delay completed ( '1' duing delay counting)
			uint32	_undef0	: 2;
			//uint32	_undef0	: 3;
			uint32	AL	: 1;	// arbitration lost ( 1 for lost arbitration )
			uint32	BUSY	: 1;	// '1' after START signal detected, '0' for STOP
			uint32	RXACK	: 1;	// acknowledge received ( 0 for no-ack )
			uint32	_undef1	:24;
		}	bSTATUS;
	}	uSTATUS;
	union	{
		uint32	nTIME;
		struct	_I2C_TIME {
			uint32	FC	: 5;
			uint32 	CS	: 1;
			uint32		: 2;
			uint32	RC	: 8;
			uint32		: 16;
		}	bTIME;
	}	uTIME ;
	uint32	undef0[9];
}	I2C_CH;



typedef struct _I2C{
	I2C_CH	pCH0;
	I2C_CH	pCH1;
	union	{
		uint32	nPORT;
		struct _I2C_PORTS {
			uint32		PORT : 8;
			uint32			 : 8;
			uint32		RXVC : 3;
			uint32			 : 5;
			uint32        TXVC : 3;
			uint32			 : 5;
		} bPORT;
	}	uPORT;
	union	{
		uint32	nCTL;
		struct _I2C_CTL {
			uint32	EN	: 1;
			uint32		: 1;
			uint32	CLR : 1;
			uint32	SDA	: 1;
			uint32	WS  : 1;
			uint32	RCLR: 1;
			uint32		: 2;
			uint32 	RXTH: 2;
			uint32		: 2;
			uint32	TXTH: 2;
			uint32		: 2;
			uint32	DRQEN : 4;
			uint32	FC	: 5;
			uint32		: 5;
			uint32	SLV : 2;
		} bCTL;
	}	uCTL;
	union	{
		uint32	nADDR;
		struct _I2C_ADDR {
			uint32		: 1;
			uint32	ADDR: 7;
			uint32		:24;
		} bADDR;
	}	uADDR;
	union {
		uint32	nINT;
		struct _I2C_INT {
			uint32	IRQEN: 12;
			uint32		:  4;
			uint32	IRQSTAT : 12;
			uint32		: 4;
		} bINT;
	} uINT;
	union {
		uint32	nSTAT;
		struct _I2C_STAT {
			uint32 SADR 	: 8;
			uint32		:15;
			uint32 DDIR	: 1;
			uint32		: 8;
		} bSTAT;
	} uSTAT;
	uint32  tempp[2];
	union {
		uint32	nMBF;
		struct _I2C_MBF {
			uint32	MBFR : 8;
			uint32		 : 8;
			uint32	MBRT : 8;
			uint32 		 : 8;
		} bMBF;
	} uMBF;
//	unsigned char DATA[7];
	union {
		uint32  nDATA0;
	} uDATA0;
	union {
		uint32  nDATA1;
	} uDATA1;
} I2C;

#define I2C_PORT0 0
#define I2C_PORT1 1
#define I2C_PORT2 2
#define I2C_PORT3 3
#define I2C_PORT4 4
#define I2C_PORT5 5
#define I2C_PORT6 6
#define I2C_PORT7 7
#define I2C_PORT8 8
#define I2C_PORT9 9
#define I2C_PORT10 10
#define I2C_PORT11 11

#define I2CM_CH0 0
#define I2CM_CH1 1
#define I2CM_CH2 2
#define I2CM_CH3 3
#define I2CM_CH4 4
#define I2CM_CH5 5
#define I2CM_CH6 6
#define I2CM_CH7 7
#define I2CS_CH0 8
#define I2CS_CH1 9
#define I2CS_CH2 10
#define I2CS_CH3 11
//
#define I2C_8BIT  0
#define I2C_16BIT 1

#define	I2C_START	((0x1)<<7)
#define	I2C_STOP	((0x1)<<6)
#define	I2C_RD		((0x1)<<5)
#define	I2C_WR		((0x1)<<4)
#define	I2C_ACK		((0x1)<<3)
#define	I2C_DELAY	((0x1)<<2)
#define	I2C_IACK	((0x1)<<0)


#if 1  // pnor

typedef struct{
    uint32 A2X : 4; //[03:00]  //External Bus Controller Enable
    uint32     : 28;
} XCFG0;

typedef union{
    uint32    nXCFG0;
    XCFG0   bXCFG0;
} XCFG0_U;


typedef struct{
    uint32 penX      : 4; //[03:00] //Prefetch Enable, Burst Length Define
    uint32 BEC_CS0   : 1; //[04] //Byte Enable Control for Read
    uint32 BEC_CS1   : 1; //[05] //Byte Enable Control for Read
    uint32 BEC_CS2   : 1; //[06] //Byte Enable Control for Read
    uint32 BEC_CS3   : 1; //[07] //Byte Enable Control for Read
    uint32 wbthX     : 4; //[11:08] //Write Buffer Theshold
    uint32           : 12; //[23:12]
    uint32 DIMUX_CS0 : 2; //[25:24] //Data Input Mux in CS0, 01: xio[15:8], 10: xio[31:16], 11: xio[31:24]
    uint32 DIMUX_CS1 : 2; //[27:26] //Data Input Mux in CS1
    uint32 DIMUX_CS2 : 2; //[29:28] //Data Input Mux in CS2
    uint32 DIMUX_CS3 : 2; //[31:30] //Data Input Mux in CS3
} XCFG1;

typedef union{
    uint32    nXCFG1;
    XCFG1   bXCFG1;
} XCFG1_U;


typedef struct{
    uint32 sizeX0     : 6; //[05:00] //Prefetch Size
    uint32            : 4; //[09:06]
    uint32 baseX0     : 19; //[28:10] //Prefetch Base
    uint32            : 3; //[31:29]
} XCFG;

typedef union{
    uint32 nXCFG;
    XCFG bXCFG;
} XCFG_U;

//Timing Control Description
//
//<----->|<------------>|<------>|
// SETUP |     PW+1     |  HOLD  |
//       | data tranfer |        |
//HOLD is used when MASK register is 0

typedef struct{
    uint32 REC       :   4; //[03:00] Recovery (CS high)
    uint32 HOLD      :   4; //[07:04] Hold
    uint32 PW        :   8; //[15:08] Pulse Width
    uint32 SETUP     :   6; //[21:16] Setup
    uint32 MASK      :   3; //[24:22] Page Address Mask (4=32bytes), (3=16bytes). This register means how long the PNOR controller can transfer data.
    uint32 BYTE_EN   :   1; //[25]    Byte Enable
    uint32 BW        :   2; //[27:26] Bus Width (2=16-bit)
    uint32 RDY_POL   :   1; //[28]    Ready Polarity
    uint32 RDY_EN    :   1; //[29]    Ready Enable
    uint32 BASE      :   2; //[31:30] Base
} XSCS;

typedef union{
    uint32    nXSCS;
    XSCS    bXSCS;
} XSCS_U;


//    Parallel NOR Controller Register
/////////////////////////////////////////////////////////
typedef struct _PNOR {
    XCFG0_U               uXCFG0;     // 0x100
    XCFG1_U               uXCFG1;     // 0x104
    XCFG_U                uXCFG2;     // 0x108
    XCFG_U                uXCFG3;     // 0x10C
    XSCS_U                uXSCS0;     // 0x110
    XSCS_U                uXSCS1;     // 0x114
    XSCS_U                uXSCS2;     // 0x118
    XSCS_U                uXSCS3;     // 0x11C
    uint32                  uXTMOV;     // 0x120
} PNOR;

#endif
#endif
