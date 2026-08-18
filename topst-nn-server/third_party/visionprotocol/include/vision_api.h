/*
 * Copyright Telechips Inc.
 *
 * TCC Version 1.0
 *
 * This source code contains confidential information of Telechips.
 *
 * Any unauthorized use without a written permission of Telechips including not
 * limited to re-distribution in source or binary form is strictly prohibited.
 *
 * This source code is provided "AS IS" and nothing contained in this source code
 * shall constitute any express or implied warranty of any kind, including without
 * limitation, any warranty of merchantability, fitness for a particular purpose
 * or non-infringement of any patent, copyright or other third party intellectual
 * property right.
 * No warranty is made, express or implied, regarding the information's accuracy,
 * completeness, or performance.
 *
 * In no event shall Telechips be liable for any claim, damages or other
 * liability arising from, out of or in connection with this source code or
 * the use in the source code.
 *
 * This source code is provided subject to the terms of a Mutual Non-Disclosure
 * Agreement between Telechips and Company.
 */

#ifndef VISION_API_H
#define VISION_API_H

#ifdef WIN_DLL_BUILD
	#define DLLEXPORT __declspec(dllexport)
	#define STDCALL __stdcall
#else
	#define DLLEXPORT
	#define STDCALL
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h> // using NULL
#include <string.h>

// Blocking/NonBlocking Mode
#define NONBLOCKING 0
#define BLOCKING -1

// SYSTEM_DEFAULT_SOCKET_BUFSIZE
#define SYS_DEFAULT_BUFSIZE 0

// vision_api function return value
#define VISION_SUCCESS              0
#define VISION_ERROR                -1
#define ARGUMENT_ERROR              -2
#define CONNECTION_ERROR            -3
#define AUTHENTICATION_ERROR        -4
#define QUEUE_OVERFLOW              -5
#define QUEUE_UNDERFLOW             -6
#define TIMEOUT_ERROR               -7
#define MEMORY_ALLOCATION_ERROR     -8
#define BUF_REGISTRATION_ERROR      -9
#define RECONNECTING                -10

typedef enum tag_operation_mode
{
    MESSAGE_MODE = 0,
    STREAM_MODE  = 1
} operation_mode_t;

typedef enum tag_network_interface
{
    INTERFACE_ETHERNET = 0,
    INTERFACE_PCIE     = 1
} network_interface_t;

typedef enum tag_network_role
{
    SERVER = 0,
    CLIENT = 1
} network_role_t;

typedef enum tag_activation_state
{
    OFF = 0,
    ON = 1
} activation_state_t;

typedef struct tag_queue_info
{
	uint32_t maxDataSize;
	uint8_t  numQ;
} queue_info_t;

typedef struct tag_network_bufsize_t
{
	unsigned int sendBufSize;
	unsigned int recvBufSize;
} network_bufsize_t;

typedef struct tag_vision_user_config
{
    network_interface_t  netInterface;
    network_role_t       netRole;
    network_bufsize_t    netBuf;
    char                 ip[16];
    uint16_t             port;
    operation_mode_t     opMode;
    activation_state_t   reconnection;
    activation_state_t   streamZeroCopy;
    queue_info_t         sendQ;
    queue_info_t         recvQ;
} vision_user_config_t;

typedef struct tag_vision_stream_info
{
	uint32_t id;
    uint8_t *pBuffer;
	uint32_t length;
    uint64_t seqNum;
    uint64_t timestamp;
} vision_stream_info_t;

// #pragma pack(push, 1)
typedef struct tag_vision_message_header
{
	uint32_t id;
	uint32_t length;
	uint64_t seqNum;
	uint64_t timestamp;
} vision_message_header_t;
// #pragma pack(pop)

int STDCALL DLLEXPORT Vision_API_Initialization(const vision_user_config_t *pUserConfig, void **pHandle);
int STDCALL DLLEXPORT Vision_API_Deinitialization(void **pHandle);
int STDCALL DLLEXPORT Vision_API_Connection(void *pHandle);
int STDCALL DLLEXPORT Vision_API_Disconnection(void *pHandle);

int STDCALL DLLEXPORT Vision_API_SendMessage(const void *pHandle, const void *pBuffer, uint32_t length, int16_t timeout_ms);
int STDCALL DLLEXPORT Vision_API_RecvMessage(const void *pHandle, void *pBuffer, uint32_t length, int16_t timeout_ms);
int STDCALL DLLEXPORT Vision_API_PeekMessage(const void *pHandle, void *pBuffer, uint32_t length);

int STDCALL DLLEXPORT Vision_API_RegisterStreamSendBuffer(const void *pHandle, void *pBuffer, uint32_t length);
int STDCALL DLLEXPORT Vision_API_RegisterStreamRecvBuffer(const void *pHandle, void *pBuffer, uint32_t length);

int STDCALL DLLEXPORT Vision_API_GetStreamSendBuffer(const void *pHandle, vision_stream_info_t **pStreamInfo, uint8_t *pIndex, int16_t timeout_ms);
int STDCALL DLLEXPORT Vision_API_SendStream(const void *pHandle, const vision_stream_info_t *pStreamInfo, uint8_t index);

int STDCALL DLLEXPORT Vision_API_RecvStream(const void *pHandle, vision_stream_info_t **pStreamInfo, uint8_t *pIndex, int16_t timeout_ms);
int STDCALL DLLEXPORT Vision_API_PeekStream(const void *pHandle, vision_stream_info_t **pStreamInfo, uint8_t *pIndex);
int STDCALL DLLEXPORT Vision_API_ReleaseStreamRecvBuffer(const void *pHandle, const vision_stream_info_t *pStreamInfo, uint8_t index);

#endif
