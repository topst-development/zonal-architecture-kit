/*
 * FreeRTOS Kernel V10.3.1
 * Copyright (C) 2017 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

/*
 * This project is a cut down version of the project described on the following
 * link.  Only the simple UDP client and server and the TCP echo clients are
 * included in the build:
 * http://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/examples_FreeRTOS_simulator.html
 */

/* Standard includes. */
#include <stdio.h>
#include <time.h>

/* FreeRTOS includes. */
#include <FreeRTOS.h>
#include "task.h"

/* Demo application includes. */
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"
#include "FreeRTOS_DNS.h"
#include "FreeRTOS_TCP_IP.h"

/* Simple UDP client and server task parameters. */
#define mainSIMPLE_UDP_CLIENT_SERVER_TASK_PRIORITY		( tskIDLE_PRIORITY )
#define mainSIMPLE_UDP_CLIENT_SERVER_PORT				( 5005UL )

/* Echo client task parameters - used for both TCP and UDP echo clients. */
#define mainECHO_CLIENT_TASK_STACK_SIZE 				( configMINIMAL_STACK_SIZE * 2 )	/* Not used in the Windows port. */
#define mainECHO_CLIENT_TASK_PRIORITY					( tskIDLE_PRIORITY + 1 )

/* Echo server task parameters. */
#define mainECHO_SERVER_TASK_STACK_SIZE					( configMINIMAL_STACK_SIZE * 2 )	/* Not used in the Windows port. */
#define mainECHO_SERVER_TASK_PRIORITY					( tskIDLE_PRIORITY + 1 )

/* Define a name that will be used for LLMNR and NBNS searches. */

/* Set the following constants to 1 or 0 to define which tasks to include and
exclude:

mainCREATE_SIMPLE_UDP_CLIENT_SERVER_TASKS:  When set to 1 two UDP client tasks
and two UDP server tasks are created.  The clients talk to the servers.  One set
of tasks use the standard sockets interface, and the other the zero copy sockets
interface.  These tasks are self checking and will trigger a configASSERT() if
they detect a difference in the data that is received from that which was sent.
As these tasks use UDP, and can therefore loose packets, they will cause
configASSERT() to be called when they are run in a less than perfect networking
environment.

mainCREATE_TCP_ECHO_TASKS_SINGLE:  When set to 1 a set of tasks are created that
send TCP echo requests to the standard echo port (port 7), then wait for and
verify the echo reply, from within the same task (Tx and Rx are performed in the
same RTOS task).  The IP address of the echo server must be configured using the
configECHO_SERVER_ADDR0 to configECHO_SERVER_ADDR3 constants in
FreeRTOSConfig.h.

mainCREATE_TCP_ECHO_SERVER_TASK:  When set to 1 a task is created that accepts
connections on the standard echo port (port 7), then echos back any data
received on that connection.
*/
#define mainCREATE_SIMPLE_UDP_CLIENT_SERVER_TASKS	1
#define mainCREATE_TCP_ECHO_TASKS_SINGLE			0
#define mainCREATE_TCP_ECHO_SERVER_TASK				0

#define mainHOST_NAME               "RTOSDemo"
#define mainDEVICE_NICK_NAME        "windows_demo"
/*-----------------------------------------------------------*/

/*
 * Just seeds the simple pseudo random number generator.
 */
static void prvSRand( UBaseType_t ulSeed );

/*
 * Miscellaneous initialisation including preparing the logging and seeding the
 * random number generator.
 */
static void prvMiscInitialisation( void );

/* The default IP and MAC address used by the demo.  The address configuration
defined here will be used if ipconfigUSE_DHCP is 0, or if ipconfigUSE_DHCP is
1 but a DHCP server could not be contacted.  See the online documentation for
more information. */

/* Set the following constant to pdTRUE to log using the method indicated by the
name of the constant, or pdFALSE to not log using the method indicated by the
name of the constant.  Options include to standard out (xLogToStdout), to a disk
file (xLogToFile), and to a UDP port (xLogToUDP).  If xLogToUDP is set to pdTRUE
then UDP messages are sent to the IP address configured as the echo server
address (see the configECHO_SERVER_ADDR0 definitions in FreeRTOSConfig.h) and
the port number set by configPRINT_PORT in FreeRTOSConfig.h. */
const BaseType_t xLogToStdout = pdTRUE, xLogToFile = pdFALSE, xLogToUDP = pdFALSE;

/* Default MAC address configuration.  The demo creates a virtual network
connection that uses this MAC address by accessing the raw Ethernet data
to and from a real network connection on the host PC.  See the
configNETWORK_INTERFACE_TO_USE definition for information on how to configure
the real network connection to use. */

/* Use by the pseudo random number generator. */
static UBaseType_t ulNextRand;

UBaseType_t uxRand( void )
{
const uint32_t ulMultiplier = 0x015a4e35UL, ulIncrement = 1UL;

	/* Utility function to generate a pseudo random number. */

	ulNextRand = ( ulMultiplier * ulNextRand ) + ulIncrement;
	return( ( int ) ( ulNextRand >> 16UL ) & 0x7fffUL );
}
/*-----------------------------------------------------------*/

static void prvSRand( UBaseType_t ulSeed )
{
	/* Utility function to seed the pseudo random number generator. */
	ulNextRand = ulSeed;
}
/*-----------------------------------------------------------*/
const char *pcApplicationHostnameHook( void )
{
    /* Assign the name "FreeRTOS" to this network node.  This function will
    be called during the DHCP: the machine will be registered with an IP
    address plus this name. */
    return mainHOST_NAME;
}
/*-----------------------------------------------------------*/

	BaseType_t xApplicationDNSQueryHook( const char *pcName )
	{
	BaseType_t xReturn;

		/* Determine if a name lookup is for this node.  Two names are given
		to this node: that returned by pcApplicationHostnameHook() and that set
		by mainDEVICE_NICK_NAME. */
		//if( _stricmp( pcName, pcApplicationHostnameHook() ) == 0 )
		if(strcmp( pcName, pcApplicationHostnameHook() ) == 0 )
		{
			xReturn = pdPASS;
		}
		//else if( _stricmp( pcName, mainDEVICE_NICK_NAME ) == 0 )
		else if(strcmp( pcName, mainDEVICE_NICK_NAME ) == 0 )
		{
			xReturn = pdPASS;
		}
		else
		{
			xReturn = pdFAIL;
		}

		return xReturn;
	}


/*
 * Callback that provides the inputs necessary to generate a randomized TCP
 * Initial Sequence Number per RFC 6528.  THIS IS ONLY A DUMMY IMPLEMENTATION
 * THAT RETURNS A PSEUDO RANDOM NUMBER SO IS NOT INTENDED FOR USE IN PRODUCTION
 * SYSTEMS.
 */
uint32_t ulApplicationGetNextSequenceNumber( uint32_t ulSourceAddress,
											 uint16_t usSourcePort,
											 uint32_t ulDestinationAddress,
											 uint16_t usDestinationPort )
{
	( void ) ulSourceAddress;
	( void ) usSourcePort;
	( void ) ulDestinationAddress;
	( void ) usDestinationPort;

	return uxRand();
}

/*
 * Supply a random number to FreeRTOS+TCP stack. 
 * THIS IS ONLY A DUMMY IMPLEMENTATION THAT RETURNS A PSEUDO RANDOM NUMBER 
 * SO IS NOT INTENDED FOR USE IN PRODUCTION SYSTEMS.
 */
BaseType_t xApplicationGetRandomNumber(uint32_t* pulNumber)
{
	*(pulNumber) = uxRand();
	return pdTRUE;
}

