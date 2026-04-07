// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : spi_eccp.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#if ( APLT_LINUX_SUPPORT_SPI_DEMO == 1 )
#ifndef SPI_ECCP_H
#define SPI_ECCP_H

//#define ECCP_DUMP_ENABLE

#if 0
#define ECCP_D(fmt, args...)           {  mcu_printf("[%s:%d] "fmt"\n", __func__, __LINE__, ## args); }
#else
#define ECCP_D(fmt, args...)            {;}
#endif


#define ECCP_GPSB_CH                    (0UL)
#define ECCP_GPSB_PORT                  (2UL)
#define ECCP_GPSB_DMA_SIZE              (512UL)
#define ECCP_GPSB_SPEED                 (10*1000*1000UL)
#define ECCP_GPSB_BPW                   (8UL)

#define ECCP_MAX_DEV_SLOT               (10UL)
#define ECCP_MAX_QUEUE_DEPTH            (2UL)
#define ECCP_MAX_PACKET                 (508UL) // Header(11) + CRC(2) + Maximum Data Length(480) + Dummy(1) + CRC(2)
#define ECCP_TASK_SIZE                  (512UL)
#define ECCP_READ_EVENT                 (0x00000001UL)
#define ECCP_WRITE_EVENT                (0x00000001UL)
#define ECCP_READ_DONE_EVENT            (0x00000002UL)
#define ECCP_EVENT_TIMEOUT              (0UL)
#define ECCP_HEADER_SIZE                (11UL)
#define ECCP_CRC_SIZE                   (2UL)
#define ECCP_DATASIZE_64                (64UL)
#define ECCP_DATASIZE_492               (492UL)
#define ECCP_PACKET_SIZE_64             (80UL) // Header(11) + CRC(2) + Data Length(64) + Dummy(1) + CRC(2)
#define ECCP_PACKET_SIZE_492            (508UL) // Header(11) + CRC(2) + Data Length(492) + Dummy(1) + CRC(2)
#define ECCP_DUMMY_SIZE                 (1UL)
#define ECCP_SPI_XFER_MODE              (GPSB_XFER_MODE_DMA | GPSB_XFER_MODE_WITH_INTERRUPT)
#define ECCP_EVENT_GET_MODE             (SAL_EVENT_OPT_SET_ANY | SAL_OPT_BLOCKING)

#define ECCP_SF0                        (0xFF)
#define ECCP_SF1                        (0x55)
#define ECCP_SF2                        (0xAA)
#define ECCP_ALL_FRAME                  (0x00)
#define ECCP_A72_FRAME                  (0x01)
#define ECCP_A53_FRAME                  (0x02)
#define ECCP_VCP_FRAME                  (0x03)

#define ECCP_SUCCESS                    (0L)
#define ECCP_GPSB_TRANSFER_FAIL         (-1L)
#define ECCP_GPSB_APP_NOT_REGISTERED    (-2L)
#define ECCP_INVALID_DESTINATION        (-3L)
#define ECCP_INVALID_FRAME              (-4L)
#define ECCP_TOO_MUCH_DATA              (-5L)
#define ECCP_NO_SPI_MANAGER             (-6L)
#define ECCP_NOSUCH_SLOT                (-7L)
#define ECCP_ALREADY_INUSE              (-8L)
#define ECCP_QUEUE_FAIL                 (-9L)
#define ECCP_NOT_OPENED                 (-10L)
#define ECCP_INVALID_DATA               (-11L)
#define ECCP_INVALID_DATA_SIZE          (-12L)
#define ECCP_INVALID_HEADER             (-13L)

#define	TC_BUFFER_ERROR		(-1)
#define	TC_BUFFER_FULL		(-2)
#define	TC_BUFFER_EMPTY		(-3)
#define	TC_BUFFER_OK		0

#define ECCP_CHECK_START_FRAME(B)       ((B[ECCP_FF] == ECCP_SF0) && (B[ECCP_55] == ECCP_SF1) && (B[ECCP_AA] == ECCP_SF2))
#define ECCP_CHECK_VCP_FRAME(B)         (B[ECCP_DES] == ECCP_VCP_FRAME)
#define ECCP_CHECK_UNUSED_DEVICE(F)     ((F < ECCP_MAX_DEV_SLOT) && (eccpDevSlot[F].devInUsed == 0UL))
#define ECCP_CHECK_VALID_DEVICE(D)      ((D < ECCP_MAX_DEV_SLOT) && (eccpDevSlot[D].devInUsed == 1UL))
#define ECCP_CHECK_OPEN_DEVICE(D)       ((D != NULL) && ((D)->devInUsed == 1UL))
#define ECCP_CHECK_WAIT_EVENT(I, E, F)  (SAL_RET_SUCCESS == SAL_EventGet(I, E, ECCP_EVENT_TIMEOUT,  ECCP_EVENT_GET_MODE, F))
#define ECCP_CHECK_TIMEOUT(F)           (F != 0UL)
//#define ECCP_CHECK_GPSB_XFER(T, R, L)   (SAL_RET_SUCCESS == GPSB_Xfer(ECCP_GPSB_CH, T, R, L, ECCP_SPI_XFER_MODE))
#define ECCP_CHECK_GPSB_XFER(T, R, L)   (SAL_RET_SUCCESS == GPSB_AsyncXfer(ECCP_GPSB_CH, (uint32 *)T, (uint32 *)R, L, ECCP_SPI_XFER_MODE))
#define ECCP_CHECK_QUEUEPUT(R, Q, M, L) (SAL_RET_SUCCESS != (R = SAL_QueuePut(Q, M, sizeof(L), 0UL, SAL_OPT_BLOCKING)))
#define ECCP_CHECK_QUEUEGET(R, Q, M, L) (SAL_RET_SUCCESS == (R = SAL_QueueGet(Q, M, L, 0UL, SAL_OPT_BLOCKING)))
#define ECCP_CHECK_VALID_LENGTH(L, H)   (L <= (ECCP_MAX_PACKET - H))
#define ECCP_CHECK_VALID_DATA(L)        (L > 0UL)
#define ECCP_CHECK_CRC(E, R)            (E == R)
#define ECCP_CHECK_ONLY_HEADER(M)       ((M)->msgLen == 0UL)
#define ECCP_CHECK_NO_ERROR(R)          (R == ECCP_SUCCESS)
#define ECCP_CHECK_BL_CMD(B)            (5UL == B[ECCP_CMD2H])
#define ECCP_RECEIVED_DEVICE(D, B)      (D = (B[ECCP_CMD1H] << 8) | B[ECCP_CMD1L])
#define ECCP_RECEIVED_LENGTH(M, B)      ((M)->msgLen = ((B[ECCP_LENH] << 8) | B[ECCP_LENL]))
#define ECCP_RECEIVED_COMMAND(M, B)     ((M)->msgCmd = ((B[ECCP_CMD2H] << 8) | B[ECCP_CMD2L]))
#define ECCP_RECEIVED_SOURCE(M, B)      ((M)->msgSource = B[ECCP_SRC])
//#define ECCP_RECEIVED_CRC(C, M, B, L)   (C = (B[(M)->msgLen + L] << 8UL) | B[(M)->msgLen + L + 1UL])
#define ECCP_RECEIVED_CRC(C, L, B)      (C = (B[L] << 8UL) | B[L + 1UL])
#define ECCP_RECEIVED_DATA(M, B)        ((void) SAL_MemCopy((M)->msgPtr, B, (M)->msgLen))
//#define ECCP_EXTRACT_CRC(C, M, B)       (C = ECCP_CalcCrc16(B, (M)->msgLen))
#define ECCP_EXTRACT_CRC(C, L, B)       (C = ECCP_CalcCrc16(B, L))
#define ECCP_REMOVE_CRC_LENGTH(M)       ((M)->msgLen -= ECCP_CRC_SIZE)

#define ECCP_CLEAR_EVENT(I, E)                                        \
{                                                                     \
    if (SAL_RET_SUCCESS != SAL_EventSet(I, E, SAL_EVENT_OPT_CLR_ALL)) \
    {                                                                 \
        mcu_printf("%s Event Clear Fail\n", __func__);                \
    }                                                                 \
};

#define ECCP_NOTIFY_READ_DONE           \
{                                       \
    (void) GPIO_Set(GPIO_GPA(23), 1UL); \
    (void) GPIO_Set(GPIO_GPA(23), 0UL); \
};

typedef struct ECCPManager
{
    uint32                              init;
    uint32                              mgrTxTask;
    uint32                              mgrRxTask;
    uint32                              mgrTxQueue;
    uint32                              mgrTxEvent;
    uint32                              mgrRxEvent;
    uint32                              mgrRxDoneEvent;
    uint32                              mgrLock;
} ECCPManager_t;

typedef struct ECCPMsg
{
    uint32                              msgLen;
    uint8                               msgPtr[ECCP_TASK_SIZE];
} ECCPMsg_t;

typedef struct ECCPUnMarMsg
{
    uint8                               msgSource;
    uint16                              msgCmd;
    uint32                              msgLen;
    uint8                               msgPtr[ECCP_MAX_PACKET];
} ECCPUnMarMsg_t;

typedef struct ECCPDev
{
    const uint8 *                       devName;
    uint16                              devDevId;
    uint32                              devSpiCh;
    uint32                              devQueue;
    uint32                              devLock;
    ECCPMsg_t                           devTxMsg;
    ECCPUnMarMsg_t                      devRxMsg;
    uint32                              devInUsed;
} ECCPDev_t;

typedef enum
{
    ECCP_FF                              = 0,
    ECCP_55,
    ECCP_AA,
    ECCP_SRC,
    ECCP_DES,
    ECCP_CMD1H,
    ECCP_CMD1L,
    ECCP_CMD2H,
    ECCP_CMD2L,
    ECCP_LENH,
    ECCP_LENL,
    ECCP_DATA,
} ECCPHeader_t;

typedef enum
{
    ECCP_CONTROL                        = 0,
    ECCP_STR_APP,
    ECCP_KEY_APP,
    ECCP_UPDATE_APP,
    ECCP_SYSMON_APP,
} ECCPAppId_t;

typedef struct stTC_RINGBUF
{
	uint32 		Head;		//read position
	uint32		Tail;		//write position
	uint32		MaxBufferSize;
	ECCPMsg_t *     pBuffer;
}TC_RINGBUF;


void ECCP_InitSPIManager
(
    void
);

ECCPDev_t* ECCP_Open
(
    const uint8 *                       pucDevName,
    uint16                              ucFeature
);

int32 ECCP_Send
(
    ECCPDev_t *                         eccpDev,
    uint8                               ucTarget,
    uint16                              puiCmd2,
    const uint8 *                       pucBuff,
    uint32                              uiLength
);

void ECCP_CheckRx
(
    ECCPMsg_t *                         psRxMsg
);

int32 ECCP_Recv
(
    ECCPDev_t *                         eccpDev,
    uint8 *                             pucSource,
    uint16 *                            puiCmd,
    uint8 *                             pucBuff
);

uint16 ECCP_CalcCrc16
(
    const uint8 *                       pucBuffer,
    uint32                              uiLength
);


#ifdef ECCP_DUMP_ENABLE
void ECCP_Dump
(
    const int8 *                        pucFunc,
    uint8 *                             pucBuff,
    uint32                              uiLen
);
#endif
#endif
#endif
