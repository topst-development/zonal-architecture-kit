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

#ifndef __MESSAGE_API_H__
#define __MESSAGE_API_H__

#include <stdint.h>
#include <stdbool.h>
#include <vision_api.h>
#include <json-c/json.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */
#define VISION_PROTOCOL_CTRL_DATA_SIZE (1024 * 320)

#define VISION_PROTOCOL_STREAM_QUEUE_COUNT 4
#define VISION_PROTOCOL_CTRL_QUEUE_COUNT 6

typedef enum tagMSGs
{
	VISION_MSG_EVENT_WAKEUP = 1, // EVENT + NO PARAM

	VISION_MSG_REQUEST_VERSION = 2,	 // REQEUST + NO_PARAM
	VISION_MSG_RESPONSE_VERSION = 3, // RESPONS + VERSION(string)

	VISION_MSG_REQUEST_STREAMINFO = 4,	// REQEUST + NO_PARAM
	VISION_MSG_RESPONSE_STREAMINFO = 5, // RESPONS + IsStreamSender(unit8_t)+IsStreamReceiver(unit8_t)+SendstreamSize(uint32_t)
	VISION_MSG_EVENT_STREAMINFO = 6,	// EVENT + IsStreamSender(unit8_t)+IsStreamReceiver(unit8_t)+SendstreamSize(uint32_t)

	VISION_MSG_REQUEST_NPU_INFERENCE_TIME = 7,	// REQEUST + NO_PARAM
	VISION_MSG_RESPONSE_NPU_INFERENCE_TIME = 8, // RESPONS + ??
	VISION_MSG_EVENT_NPU_INFERENCE_TIME = 9,	// EVENT + ??

	VISION_MSG_REQUEST_NPU_UTILIZATION = 10,  // REQEUST + NO_PARAM
	VISION_MSG_RESPONSE_NPU_UTILIZATION = 11, // RESPONS + ??
	VISION_MSG_EVENT_NPU_UTILIZATION = 12,	  // EVENT + ??

	VISION_MSG_REQUEST_NPU_INFERENCE_OD_BOUNDINGBOX = 13,  // REQEUST + NO_PARAM
	VISION_MSG_RESPONSE_NPU_INFERENCE_OD_BOUNDINGBOX = 14, // RESPONS + xmin(uint16_t) + xmin(uint16_t) + xmax(uint16_t) + ymax(uint16_t) + score(uint16_t) + class(uint16_t) + label (char[20])
	VISION_MSG_EVENT_NPU_INFERENCE_OD_BOUNDINGBOX = 15,	   // EVENT + xmin(uint16_t) + xmin(uint16_t) + xmax(uint16_t) + ymax(uint16_t) + score(uint16_t) + class(uint16_t) + label (char[20])

	VISION_MSG_REQUEST_NPU_INFERENCE_LD_RESULT = 16,  // REQEUST + NO_PARAM
	VISION_MSG_RESPONSE_NPU_INFERENCE_LD_RESULT = 17, // RESPONS + ??
	VISION_MSG_EVENT_NPU_INFERENCE_LD_RESULT = 18,	  // EVENT + ??

	VISION_MSG_REQUEST_NPU_INFERENCE_HBA_RESULT = 19,  // REQEUST + NO_PARAM
	VISION_MSG_RESPONSE_NPU_INFERENCE_HBA_RESULT = 20, // RESPONS + EnDisable(uint8)
	VISION_MSG_EVENT_NPU_INFERENCE_HBA_RESULT = 21,	   // EVENT + EnDisable(uint8)

	VISION_MSG_REQUEST_NPU_INFERENCE_TSR_RESULT = 22,  // REQEUST + NO_PARAM
	VISION_MSG_RESPONSE_NPU_INFERENCE_TSR_RESULT = 23, // RESPONS + class(uint16_t)+label(char[20])
	VISION_MSG_EVENT_NPU_INFERENCE_TSR_RESULT = 24,	   // EVENT + class(uint16_t)+label(char[20])

	VISION_MSG_REQUEST_CPU_UTILIZATION = 25,  // REQEUST + NO_PARAM
	VISION_MSG_RESPONSE_CPU_UTILIZATION = 26, // RESPONS +utilization(uint16)
	VISION_MSG_EVENT_CPU_UTILIZATION = 27,	  // EVENT + utilization(uint16)

	VISION_MSG_REQUEST_SDK_MEM_USAGE = 28,	// REQEUST + NO_PARAM
	VISION_MSG_RESPONSE_SDK_MEM_USAGE = 29, // RESPONS + usage(uin32_t)
	VISION_MSG_EVENT_SDK_MEM_USAGE = 30,	// EVENT + usage(uin32_t)

	VISION_MSG_REQUEST_SDK_VIDEO_FPS = 31,	// REQEUST + NO_PARAM
	VISION_MSG_RESPONSE_SDK_VIDEO_FPS = 32, // RESPONS + fps(unsigned float)
	VISION_MSG_EVENT_SDK_VIDEO_FPS = 33,	// EVENT + fps(unsigned float)

	VISION_MSG_REQUEST_SDK_VIDEO_IMAGE_INFO = 34,  // REQEUST + NO_PARAM
	VISION_MSG_RESPONSE_SDK_VIDEO_IMAGE_INFO = 35, // RESPONS + resolutionX(uint16_t) + resolutionY(uint16_t) + fps (unsigned float) + colorSpace(uint8_t)+PTS(uint64_t)
	VISION_MSG_EVENT_SDK_VIDEO_IMAGE_INFO = 36,	   // EVENT + resolutionX(uint16_t) + resolutionY(uint16_t) + fps (unsigned float) + colorSpace(uint8_t)+PTS(uint64_t)

	VISION_MSG_REQUEST_STREAM_START = 37,  // REQUEST + NO_PARAM
	VISION_MSG_RESPONSE_STREAM_START = 38, // REPONSE + NO_PARAM
	VISION_MSG_EVENT_STREAM_START = 39,	   // EVENT + NO_PARAM

	VISION_MSG_REQUEST_STREAM_STOP = 40,  // REQUEST + NO_PARAM
	VISION_MSG_RESPONSE_STREAM_STOP = 41, // REPONSE + NO_PARAM
	VISION_MSG_EVENT_STREAM_STOP = 42,	  // EVENT + NO_PARAM

	VISION_MSG_EVENT_RESULT_STARTSTOP = 43,

	VISION_MSG_EVENT_RESULT_DATA_JSON = 44,
	VISION_MSG_EVENT_RESULT_PERF_JSON=45,
	VISION_MSG_EVENT_DEWARP_STATUS=46,
	VISION_MSG_EVENT_DEWARP_DATA_MSG=47,

	VISION_MSG_EVENT_NPU_INFERENCE_DMS_RESULT = 48,	  // EVENT + ??
	VISION_MSG_EVENT_NPU_DRIVER_USAGE = 49	  // EVENT + ??
} MSGs;

#pragma pack(push, 1)
typedef struct tagVision_MSG_NpuInferenceTime
{
	uint16_t Total;
	uint16_t current;
	uint32_t time;
} Vision_MSG_NpuInferenceTime;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct tagVision_MSG_NpuUtilization
{
	uint16_t Total;
	uint16_t current;
	uint32_t util;
} Vision_MSG_NpuUtilization;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct tagVision_MSG_NpuInferenceODBoundingBox
{
	uint16_t Total;
	uint16_t current;
	uint16_t xmin;
	uint16_t ymin;
	uint16_t xmax;
	uint16_t ymax;
	uint16_t score;
	uint16_t classId;
	double distance;
	uint16_t speed;
	uint8_t label[20];
} Vision_MSG_NpuInferenceODBoundingBox;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct tagVision_MSG_NpuInferenceLDResult
{
	uint16_t TotalLane;
	uint16_t currentLane;
	uint16_t TotalPoint;
	uint16_t currentPoint;
	uint32_t x;
	uint32_t y;
} Vision_MSG_NpuInferenceLDResult;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct tagVision_MSG_NpuInferenceHBAResult
{
	uint16_t Total;
	uint16_t current;
	uint32_t EnDisable;
} Vision_MSG_NpuInferenceHBAResult;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct tagVision_MSG_NpuInferenceTSRResult
{
	uint16_t Total;
	uint16_t current;
	uint32_t classID;
	uint8_t label[20];
} Vision_MSG_NpuInferenceTSRResult;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct tagVision_MSG_CpuUtilization
{
	uint32_t utilization;
} Vision_MSG_CpuUtilization;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct tagVision_MSG_SdkMemUsage
{
	uint32_t usage;
} Vision_MSG_SdkMemUsage;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct tagVision_MSG_SdkVideoFPS
{
	float fps;
} Vision_MSG_SdkVideoFPS;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct tagVision_MSG_SdkVideoImageInfo
{
	uint16_t resolutionX;
	uint16_t resolutionY;
	uint8_t colorSpace; // to be defined
	float fps;
} Vision_MSG_SdkVideoImageInfo;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct tagVision_MSG_ImagedataHeader
{
	uint64_t length;
} Vision_MSG_ImagedataHaeder;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct tagVision_MSG_ResultEventStartStop
{
	int32_t status;
} Vision_MSG_ResultEventStartStop;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct tagVision_MSG_DewarpStatus{
	uint32_t status;
} Vision_MSG_DewarpStatus;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct tagVision_MSG_NpuUsage{
	uint32_t npu0Dma;
	uint32_t npu0Comp;
	uint32_t npu1Dma;
	uint32_t npu1Comp;
} Vision_MSG_NpuUsage;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
	int32_t x;
	int32_t y;
}PointXy;

typedef struct tagVision_MSG_NpuInferenceDMSResult
{
	PointXy pts[4];
	PointXy eyeRightLandmark[15];
	PointXy eyeLeftLandmark[15];

	int32_t ear_l;
	int32_t ear_r;
	int32_t faceDetect;
	int32_t drowsyLevel;

	int32_t longDistractLevel;
	int32_t shortDistractLevel;
	int32_t sleepLevel;
	int32_t microSleepLevel;
	int32_t unreponsiveDriverLevel;
} Vision_MSG_NpuInferenceDMSResult;
#pragma pack(pop)

typedef struct box
{
	float xMin;
	float yMin;
	float xMax;
	float yMax;
	int32_t cls;
	float score;
	double distance;
	int32_t speed;
	char label[20];
} box_t;

typedef struct point
{
	int32_t x;
	int32_t y;
} point_t;

typedef struct lane
{
	point_t points[100];
	int32_t pointNum;
} lane_t;

typedef struct boxes
{
	box_t boxes[256];
	int boxesNum;
}boxes_t;

typedef struct lanes
{
	lane_t lanes[20];
	int laneNum;
	int psudo_vp;
}lanes_t;

typedef enum _message_status
{
	MESSAGE_STATUS_IDLE = 0,
	MESSAGE_STATUS_OPENED,		// only message
	MESSAGE_STATUS_STREAMING	// message + stream
} message_status_t;

typedef enum _message_stream_status
{
	MESSAGE_STREAM_OFF = 0,
	MESSAGE_STREAM_SEND,	// projection
	MESSAGE_STREAM_RECV,	// injection
	MESSAGE_STREAM_ALL		// not supported
} message_stream_status_t;

typedef enum _message_result_type
{
	MESSAGE_TYPE_CL = 0, // classification
	MESSAGE_TYPE_OD		 // object detection
} message_result_type_t;

typedef struct _message_buffer
{
	uint32_t length;
	uint64_t phyAddr;
	uint8_t *virAddr;
	uint64_t syncStamp;
} message_buffer_t;

typedef struct _message_context
{
	message_status_t status;
	pthread_t threadId;
	bool threadStatus;
	/* vision protocol*/
	void *messageHandle;
	void *streamHandle;
	message_buffer_t *pmRecvBuffer;
	message_buffer_t *pmSendBuffer;
	int8_t recvBufferCount;
	int8_t sendBufferCount;
	queue_info_t recvStreamInfo;
	queue_info_t controlQue;
	queue_info_t streamQue;
	vision_stream_info_t *popInfo[VISION_PROTOCOL_STREAM_QUEUE_COUNT];

} message_context_t;

typedef void *MessageHandle;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */
int32_t MessageCreate(MessageHandle *pHandle);
int32_t MessageDestroy(MessageHandle handle);
int32_t MessageOpen(MessageHandle handle, message_stream_status_t streamMode, uint64_t *baseOffset, int32_t bufferCount, uint32_t bufferSize, char *pTargetipAddress, uint16_t streamPort, uint16_t messagePort);
int32_t MessageClose(MessageHandle handle);
int32_t MessagePopReceiveBuffer(MessageHandle handle, uint8_t **virtualAddr, uint64_t *baseOffset, uint64_t *syncStamp);
int32_t MessagePushReceiveBuffer(MessageHandle handle, uint64_t baseOffset);
int32_t MessagePopSendBuffer(MessageHandle handle, uint8_t **virtualAddr, uint64_t *baseOffset, uint32_t *bufferIndex);
int32_t MessagePushSendBuffer(MessageHandle handle, uint8_t *virtualAddr, uint8_t bufferIndex);
message_status_t MessageGetStatus(MessageHandle handle);

#endif // __MESSAGE_API_H__
