/*****************************************************************************
* Copyright (c) [2024] Argenox Technologies LLC
* All rights reserved.
*
*
*
* NOTICE:  All information contained herein, source code, binaries and
* derived works is, and remains the property of Argenox and its suppliers,
* if any.  The intellectual and technical concepts contained
* herein are proprietary to Argenox and its suppliers and may be covered
* by U.S. and Foreign Patents, patents in process, and are protected by
* trade secret or copyright law.
*
* Licensing of this software can be found in LICENSE
*
* THIS SOFTWARE IS PROVIDED BY ARGENOX "AS IS" AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL ARGENOX LLC BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
* CONTACT: info@argenox.com
*
* File:    mqttnox_tal_windows.h
* Summary: MQTTNox TCP Windows Implementation
*
* Note: Do not call the functions in this file directly. Use mqttnox.h
*
*/

#ifdef __cplusplus
extern "C" {
#endif

#define _WINSOCK_DEPRECATED_NO_WARNINGS


#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>

/* System Includes */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <process.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#pragma comment(lib, "ws2_32.lib")

#include "mqttnox.h"
#include "mqttnox_debug.h"
#include "mqttnox_tal.h"

struct hostent* host;

uintptr_t receive_thread;
HANDLE mqttnox_tcp_receive_thread_obj;

uint8_t server_ready = 0;

static uint8_t main_buffer[1024];

static mqttnox_tcp_rcv_t mqttnox_tcp_rcv_cback;
static mqttnox_client_t* client = NULL;

mqttnox_tcp_rcv_t data_callback = NULL;

SOCKET ClientSocket = INVALID_SOCKET;
WSADATA wsaData;


typedef struct
{
    int sock;
    struct sockaddr address;

#if defined(__linux__)
    socklen_t  addr_len;
#else
    int addr_len;
#endif
    SOCKET ClientSocket;

} connection_t;


/**@brief TCP Initialization
 * 
 * 
 * @param[in]   c    mqttnox object \see mqttnox_client_t
 * @param[in]   rcv_cback functio pointer to the receiver function
 *
 */
int mqttnox_tcp_init(mqttnox_client_t* c, mqttnox_tcp_rcv_t rcv_cback)
{
    if(rcv_cback != NULL) {
        mqttnox_tcp_rcv_cback = rcv_cback;
    }

    if (c != NULL) {
        client = c;
    }

    return 0;
}

/**@brief TCP Connect
 * 
 * @note this function provides TCP connection to the Address and Port
 *       specified
 * 
 * @param[in]   addr  
 * @param[in]   port  TCP port number used in mQTT
 *
 */
int mqttnox_tcp_connect(char * addr, int port)
{
    int rc;
    connection_t * connection;
    struct sockaddr_in server;
    void* ptr = NULL;
    char addrstr[32];

    int sock = -1;

    data_callback = mqttnox_tcp_rcv_cback;

    struct addrinfo* result = NULL;
    struct addrinfo hints;

    /* Initialize Winsock */
    rc = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (rc != 0) {
        printf("WSAStartup failed with error: %d\n", rc);
        return -1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    char port_str[32];
    snprintf(port_str, 32, "%d", port);

    /* Resolve the server address and port */
    rc = getaddrinfo(addr, port_str, &hints, &result);
    if (rc != 0) {
        printf("getaddrinfo failed with error: %d\n", rc);
        WSACleanup();
        return -1;
    }

    /* Create a SOCKET for connecting to server */
    ClientSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ClientSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return -1;
    }

    ptr = &((struct sockaddr_in*)result->ai_addr)->sin_addr;
    inet_ntop(result->ai_family, ptr, addrstr, sizeof(addrstr));

    /* Deallocate */
    freeaddrinfo(result);

    server.sin_addr.s_addr = inet_addr(addrstr);
	server.sin_family = AF_INET;
	server.sin_port = htons( port );

	/* Connect to remote server */
	if (connect(ClientSocket , (struct sockaddr *)&server , sizeof(server)) < 0)
	{
		puts("connect error");
		return -1;
	}

    printf("Ready and listening\n");

    connection = (connection_t*)malloc(sizeof(connection_t));
    if (connection == NULL) {
        return -1;
    }

    /* Create listening thread */
    mqttnox_tcp_receive_thread_obj = (HANDLE)_beginthread(&mqttnox_tcp_receive_thread, 0, (void*)connection);

    return 0;
}

int mqttnox_tcp_send(uint8_t * data, uint16_t len)
{
    int ret = 1;

     if (len > 0) {
        // Echo the buffer back to the sender
        ret = send(ClientSocket, data, len, 0);
        if (ret == SOCKET_ERROR) {
            printf("send failed with error: %d\n", WSAGetLastError());
            closesocket(ClientSocket);
            WSACleanup();
            return 1;
        }
        printf("Bytes sent: %d\n", ret);
    }

    return 0;
}

int mqttnox_tcp_disconnect(void)
{
    closesocket(ClientSocket);
    WSACleanup();

    return 0;
}


int mqttnox_tcp_receive_thread(void * ptr)
{    
    int len = 0;
    connection_t * conn;    
    int run = 1;
    int i = 0;

    if (!ptr) run = 0;

    conn = (connection_t *)ptr;

    while(run)
    {
        len = recv(ClientSocket, main_buffer, sizeof(main_buffer), 0);

        if (len > 0)
        {
            print_buffer(main_buffer, len);

            if(mqttnox_tcp_rcv_cback != NULL) {
                data_callback(client, main_buffer, len);
            }

            len = 0;            
        }
        else if (len == 0)
        {
            /* Connection has been closed */
            closesocket(ClientSocket);
            run = 0;
        }
    }

    return 0;
}

void mqttnox_wait_thread(void)
{
    WaitForSingleObject(mqttnox_tcp_receive_thread_obj, INFINITE);
}


void mqttnox_hal_debug_printf(const char* str)
{
    printf("%s", str);    
}

#ifdef __cplusplus
}
#endif