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

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <string.h>
#include <math.h>

#include "message_api.h"

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */
#define VISION_PROTOCOL_STREAM_QUEUE_COUNT 4
#define VISION_PROTOCOL_CTRL_QUEUE_COUNT 6
#define VISION_PROTOCOL_RECV_POLL_TIMEOUT_MS 100

/* ========================================================================== */
/*                       Internal Function Declarations                       */
/* ========================================================================== */

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */
static uint32_t sentSyncStamp = 0;

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */
int32_t MessageCreate(MessageHandle *pHandle)
{
	int32_t status = 0;
	message_context_t *pContext;

	*pHandle = NULL;

	pContext = malloc(sizeof(*pContext));
	if (pContext == NULL)
	{
		return -1;
	}

	memset(pContext, 0, sizeof(*pContext));

	*pHandle = (MessageHandle)pContext;

	return status;
}

int32_t MessageDestroy(MessageHandle handle)
{
	int32_t status = 0;

	if (handle)
	{
		free(handle);
	}

	return status;
}

message_status_t MessageGetStatus(MessageHandle handle)
{
	int32_t status;

	message_context_t *pContext = (message_context_t *)handle;

	status = pContext->status;

	return status;
}

int32_t MessageOpen(MessageHandle handle, message_stream_status_t streamMode, uint64_t *baseOffset, int32_t bufferCount, uint32_t bufferSize, char *pTargetipAddress, uint16_t streamPort, uint16_t messagePort)
{
	int32_t ret = -1;

	message_context_t *pContext = (message_context_t *)handle;

	if (pContext->status == MESSAGE_STATUS_IDLE)
	{
		int32_t bufIndex;

		vision_user_config_t messageConfig;
		memset(&messageConfig, 0, sizeof(vision_user_config_t));
		messageConfig.netInterface 			= INTERFACE_ETHERNET;
		messageConfig.netRole 				= CLIENT;
		messageConfig.netBuf.sendBufSize	= SYS_DEFAULT_BUFSIZE;
		messageConfig.netBuf.recvBufSize	= SYS_DEFAULT_BUFSIZE;
		strcpy(messageConfig.ip, pTargetipAddress);
		messageConfig.port 					= messagePort;
		messageConfig.reconnection 			= OFF;
		messageConfig.opMode 				= MESSAGE_MODE;

		messageConfig.sendQ.maxDataSize 	= VISION_PROTOCOL_CTRL_DATA_SIZE;
		messageConfig.sendQ.numQ 			= VISION_PROTOCOL_CTRL_QUEUE_COUNT;
		messageConfig.recvQ.maxDataSize 	= VISION_PROTOCOL_CTRL_DATA_SIZE;
		messageConfig.recvQ.numQ 			= VISION_PROTOCOL_CTRL_QUEUE_COUNT;

		ret = Vision_API_Initialization(&messageConfig, &pContext->messageHandle);
		if(ret == 0)
		{
			ret = Vision_API_Connection(pContext->messageHandle);
			if(ret == 0)
			{
				pContext->status = MESSAGE_STATUS_OPENED;
			}
			else
			{
				printf("[%s][Error] Vision_API_Connection(MESSAGE_HANDLE)\n", __FUNCTION__);
			}
		}
		else
		{
			printf("[%s][Error] Vision_API_Initialization(MESSAGE_HANDLE)\n", __FUNCTION__);
		}

		vision_user_config_t streamConfig;
		memset(&streamConfig, 0, sizeof(vision_user_config_t));
		streamConfig.netInterface 			= INTERFACE_ETHERNET;
		streamConfig.netRole 				= CLIENT;
		streamConfig.netBuf.sendBufSize		= SYS_DEFAULT_BUFSIZE;
		streamConfig.netBuf.recvBufSize		= SYS_DEFAULT_BUFSIZE;
		strcpy(streamConfig.ip, pTargetipAddress);
		streamConfig.port 					= streamPort;
		streamConfig.reconnection 			= OFF;
		streamConfig.streamZeroCopy			= ON;
		streamConfig.opMode 				= STREAM_MODE;

		if(streamMode == MESSAGE_STREAM_OFF) // stream mode off
		{
			streamConfig.sendQ.maxDataSize 		= 0;
			streamConfig.sendQ.numQ 			= 0;
			streamConfig.recvQ.maxDataSize 		= 0;
			streamConfig.recvQ.numQ 			= 0;
		}
		else if(streamMode == MESSAGE_STREAM_SEND) // projection mode
		{
			streamConfig.sendQ.maxDataSize 		= bufferSize;
			streamConfig.sendQ.numQ 			= bufferCount;
			streamConfig.recvQ.maxDataSize 		= 0;
			streamConfig.recvQ.numQ 			= 0;
		}
		else // injection mode (MESSAGE_STREAM_RECV)
		{
			streamConfig.sendQ.maxDataSize 		= 0;
			streamConfig.sendQ.numQ 			= 0;
			streamConfig.recvQ.maxDataSize 		= bufferSize;
			streamConfig.recvQ.numQ 			= bufferCount;
		}

		if((pContext->status == MESSAGE_STATUS_OPENED) && (streamMode > 0) && (bufferCount > 0) && (bufferSize > 0))
		{
			ret = Vision_API_Initialization(&streamConfig, &pContext->streamHandle);
			if(ret == 0)
			{
				if(streamMode == MESSAGE_STREAM_SEND) // projection
				{
					pContext->sendBufferCount = streamConfig.sendQ.numQ;
					pContext->pmSendBuffer = (message_buffer_t *)malloc(sizeof(message_buffer_t) * streamConfig.sendQ.numQ);
					for (bufIndex = 0; bufIndex < pContext->sendBufferCount; bufIndex++)
					{
						int32_t memFd = 0;
						pContext->pmSendBuffer[bufIndex].length = bufferSize;
						pContext->pmSendBuffer[bufIndex].phyAddr = baseOffset[bufIndex];

						memFd = open("/dev/mem", O_RDWR | O_SYNC);
						if (memFd < 0)
						{
							printf("[%s] mem open error\n", __FUNCTION__);
							ret = -1;
							break;
						}
						else
						{
							pContext->pmSendBuffer[bufIndex].virAddr = (uint8_t *)mmap(NULL, pContext->pmSendBuffer[bufIndex].length, PROT_READ | PROT_WRITE, MAP_SHARED, memFd, (off_t)pContext->pmSendBuffer[bufIndex].phyAddr);
							if (pContext->pmSendBuffer[bufIndex].virAddr == NULL)
							{
								printf("[%s] mmap failed!!\n", __FUNCTION__);
								exit(0);
							}

							ret = Vision_API_RegisterStreamSendBuffer(pContext->streamHandle, pContext->pmSendBuffer[bufIndex].virAddr, pContext->pmSendBuffer[bufIndex].length);
							if(ret == 0)
							{
								printf("[%s, SendBuf] [vir:%p] [phy:%lu] [size:%u] [buf idx:%d]\n", __FUNCTION__, pContext->pmSendBuffer[bufIndex].virAddr, pContext->pmSendBuffer[bufIndex].phyAddr, pContext->pmSendBuffer[bufIndex].length, bufIndex);
							}
							else
							{
								printf("[%s, SendBuf] Error [buf idx:%d]\n", __FUNCTION__, bufIndex);
							}
							close(memFd);
						}
					}

					if (bufIndex == pContext->sendBufferCount)
					{
						ret = 0;
					}
					else
					{
						ret = -1;
					}
				}
				else if(streamMode == MESSAGE_STREAM_RECV) // injection
				{
					pContext->recvBufferCount = streamConfig.recvQ.numQ;
					pContext->pmRecvBuffer = (message_buffer_t *)malloc(sizeof(message_buffer_t) * streamConfig.recvQ.numQ);
					for (bufIndex = 0; bufIndex < pContext->recvBufferCount; bufIndex++)
					{
						int32_t memFd = 0;
						pContext->pmRecvBuffer[bufIndex].length = bufferSize;
						pContext->pmRecvBuffer[bufIndex].phyAddr = baseOffset[bufIndex];

						memFd = open("/dev/mem", O_RDWR | O_SYNC);
						if (memFd < 0)
						{
							printf("[%s] mem open error\n", __FUNCTION__);
							ret = -1;
							break;
						}
						else
						{
							pContext->pmRecvBuffer[bufIndex].virAddr = (uint8_t *)mmap(NULL, pContext->pmRecvBuffer[bufIndex].length, PROT_READ | PROT_WRITE, MAP_SHARED, memFd, (off_t)pContext->pmRecvBuffer[bufIndex].phyAddr);
							if (pContext->pmRecvBuffer[bufIndex].virAddr == NULL)
							{
								printf("[%s] mmap failed!!\n", __FUNCTION__);
								exit(0);
							}

							ret = Vision_API_RegisterStreamRecvBuffer(pContext->streamHandle, pContext->pmRecvBuffer[bufIndex].virAddr, pContext->pmRecvBuffer[bufIndex].length);
							if(ret == 0)
							{
								printf("[%s, RecvBuf] [vir:%p] [phy:%lu] [size:%u] [buf idx:%d]\n", __FUNCTION__, pContext->pmRecvBuffer[bufIndex].virAddr, pContext->pmRecvBuffer[bufIndex].phyAddr, pContext->pmRecvBuffer[bufIndex].length, bufIndex);
							}
							else
							{
								printf("[%s, RecvBuf] Error [buf idx:%d]\n", __FUNCTION__, bufIndex);
							}
							close(memFd);
						}
					}

					if (bufIndex == pContext->recvBufferCount)
					{
						ret = 0;
					}
					else
					{
						ret = -1;
					}
				}

				if(ret == 0)
				{
					printf("[%s] wait for target signal ...\n", __FUNCTION__);
					ret = Vision_API_Connection(pContext->streamHandle);
					if(ret == 0)
					{
						pContext->status = MESSAGE_STATUS_STREAMING;
					}
				}
			}
		}
		else
		{
			printf("[%s] Not used streaming\n", __FUNCTION__);
			pContext->status = MESSAGE_STATUS_OPENED;
			ret = 0;
		}
	}
	return ret;
}

int32_t MessageClose(MessageHandle handle)
{
	int32_t ret = -1;

	message_context_t *pContext = (message_context_t *)handle;

	if ((pContext->status > MESSAGE_STATUS_IDLE) && (pContext->status <= MESSAGE_STATUS_STREAMING))
	{
		Vision_API_Disconnection(pContext->messageHandle);
		Vision_API_Deinitialization(&pContext->messageHandle);
		if (pContext->status == MESSAGE_STATUS_STREAMING)
		{
			Vision_API_Disconnection(pContext->streamHandle);
			Vision_API_Deinitialization(&pContext->streamHandle);

			if (pContext->pmRecvBuffer != NULL)
			{
				int32_t bufIndex, res;
				for (bufIndex = 0; bufIndex < pContext->recvBufferCount; bufIndex++)
				{
					res = munmap(pContext->pmRecvBuffer[bufIndex].virAddr, pContext->pmRecvBuffer[bufIndex].length);
					printf("[%s] Recv Buffer [vir:%p] [phy:%lu]\n", __FUNCTION__, pContext->pmRecvBuffer[bufIndex].virAddr, pContext->pmRecvBuffer[bufIndex].phyAddr);
					pContext->pmRecvBuffer[bufIndex].virAddr = NULL;
					if (res < 0)
					{
						printf("[%s] munmap error [index : %d]\n", __FUNCTION__, bufIndex);
					}
				}
				free(pContext->pmRecvBuffer);
				pContext->pmRecvBuffer = NULL;
			}

			if (pContext->pmSendBuffer != NULL)
			{
				int32_t bufIndex, res;
				for (bufIndex = 0; bufIndex < pContext->sendBufferCount; bufIndex++)
				{
					res = munmap(pContext->pmSendBuffer[bufIndex].virAddr, pContext->pmSendBuffer[bufIndex].length);
					printf("[%s] Send Buffer [vir:%p] [phy:%lu]\n", __FUNCTION__, pContext->pmSendBuffer[bufIndex].virAddr, pContext->pmSendBuffer[bufIndex].phyAddr);
					pContext->pmSendBuffer[bufIndex].virAddr = NULL;
					if (res < 0)
					{
						printf("[%s] munmap error [index : %d]\n", __FUNCTION__, bufIndex);
					}
				}
				free(pContext->pmSendBuffer);
				pContext->pmSendBuffer = NULL;
			}
		}
		pContext->messageHandle = NULL;
		pContext->streamHandle = NULL;
		pContext->status = MESSAGE_STATUS_IDLE;
		ret = 0;
	}
	else
	{
		printf("[%s] Vision Protocol close error\n", __FUNCTION__);
	}

	return ret;
}

int32_t MessagePopReceiveBuffer(MessageHandle handle, uint8_t **virtualAddr, uint64_t *baseOffset, uint64_t *syncStamp)
{
	int32_t ret = -1;

	message_context_t *pContext = (message_context_t *)handle;

	*virtualAddr = NULL;
	*baseOffset = 0ull;
	*syncStamp = 0ull;

	if (pContext->status > MESSAGE_STATUS_OPENED)
	{
		uint8_t bufIndex = pContext->recvBufferCount;
		vision_stream_info_t *popInfo;

		ret = Vision_API_RecvStream(pContext->streamHandle, &popInfo, &bufIndex,
									VISION_PROTOCOL_RECV_POLL_TIMEOUT_MS);
		if (ret != VISION_SUCCESS)
		{
			return ret;
		}
		if ((bufIndex >= pContext->recvBufferCount) || (popInfo == NULL))
		{
			printf("[%s] Invalid buffer index\n", __FUNCTION__);
			return VISION_ERROR;
		}
		if (popInfo->pBuffer != pContext->pmRecvBuffer[bufIndex].virAddr)
		{
			printf("[%s] Return buffer doesn't match with buffer at index\n", __FUNCTION__);
			return VISION_ERROR;
		}

		*virtualAddr = pContext->pmRecvBuffer[bufIndex].virAddr;
		*baseOffset = pContext->pmRecvBuffer[bufIndex].phyAddr;
		*syncStamp = popInfo->seqNum;
		pContext->popInfo[bufIndex] = popInfo;
	}
	else
	{
		printf("[%s] Device is not opened\n", __FUNCTION__);
	}
	return ret;
}

int32_t MessagePushReceiveBuffer(MessageHandle handle, uint64_t baseOffset)
{
	int32_t ret = -1;

	message_context_t *pContext = (message_context_t *)handle;

	if (pContext->status > MESSAGE_STATUS_OPENED)
	{
		uint8_t bufIndex;
		for (bufIndex = 0; bufIndex < pContext->recvBufferCount; bufIndex++)
		{
			if (pContext->pmRecvBuffer[bufIndex].phyAddr == baseOffset)
			{
				ret = Vision_API_ReleaseStreamRecvBuffer(pContext->streamHandle, pContext->popInfo[bufIndex], bufIndex);
				if (ret < 0)
				{
					printf("[%s] Push buffer error [index : %d]\n", __FUNCTION__, bufIndex);
				}
				break;
			}
		}
	}
	else
	{
		printf("[%s] Device is not opened\n", __FUNCTION__);
	}

	return ret;
}

int32_t MessagePopSendBuffer(MessageHandle handle, uint8_t **virtualAddr, uint64_t *baseOffset, uint32_t *bufferIndex)
{
	int32_t ret = -1;
	vision_stream_info_t *popInfo;
	uint8_t idx;
	*virtualAddr = NULL;
	*baseOffset = 0ull;

	message_context_t *pContext = (message_context_t *)handle;

	if (pContext->status > MESSAGE_STATUS_OPENED)
	{
		ret = Vision_API_GetStreamSendBuffer(pContext->streamHandle, &popInfo, &idx, BLOCKING);
		if(ret == 0)
		{
			popInfo->id = 0x11;
			popInfo->seqNum = sentSyncStamp++;
			popInfo->timestamp = 0;

			if (idx < pContext->sendBufferCount)
			{
				if (popInfo->pBuffer == pContext->pmSendBuffer[idx].virAddr)
				{
					*virtualAddr = pContext->pmSendBuffer[idx].virAddr;
					*baseOffset = pContext->pmSendBuffer[idx].phyAddr;
					pContext->popInfo[idx] = popInfo;
					*bufferIndex = idx;
				}
				else
				{
					printf("[%s] Return buffer doesn't match with buffer at index\n", __FUNCTION__);
				}
			}
			else
			{
				printf("[%s] Invalid buffer index\n", __FUNCTION__);
			}
		}
	}
	else
	{
		printf("[%s] Device is not opened\n", __FUNCTION__);
	}

	return ret;
}

int32_t MessagePushSendBuffer(MessageHandle handle, uint8_t *virtualAddr, uint8_t bufferIndex)
{
	int32_t ret = -1;

	message_context_t *pContext = (message_context_t *)handle;

	if (pContext->status > MESSAGE_STATUS_OPENED)
	{
		if (bufferIndex < pContext->sendBufferCount)
		{
			if (virtualAddr == pContext->pmSendBuffer[bufferIndex].virAddr)
			{
				ret = Vision_API_SendStream(pContext->streamHandle, pContext->popInfo[bufferIndex], bufferIndex);
				if (ret < 0)
				{
					printf("[%s] Push buffer error [addr : %p][index : %d]\n", __FUNCTION__, virtualAddr, bufferIndex);
				}
			}
			else
			{
				printf("[%s] mismatch buffer index and address\n", __FUNCTION__);
			}
		}
		else
		{
			printf("[%s] Invalid buffer index\n", __FUNCTION__);
		}
	}
	else
	{
		printf("[%s] Device is not opened\n", __FUNCTION__);
	}

	return ret;
}
