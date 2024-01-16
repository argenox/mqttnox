/*****************************************************************************
* Copyright (c) [2024] Argenox Technologies LLC
* All rights reserved.
*
* PROPRIETARY AND CONFIDENTIAL
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

    // Need to link with Ws2_32.lib
    //#pragma comment (lib, "Ws2_32.lib")


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

//#pragma comment (lib, "Mswsock.lib")
#pragma comment(lib, "ws2_32.lib")

#include "mqttnox.h"
#include "mqttnox_tal.h"

HANDLE  queueMutex;
HANDLE  queueCond;

int port_outgoing = 0;

struct hostent* host;

uintptr_t receive_thread;
HANDLE connection_thread_obj;

uint8_t server_ready = 0;
int port_incoming = 0;

static uint8_t received_data[1024];
static uint8_t received_length = 0;


static mqttnox_tcp_rcv_t mqttnox_tcp_rcv_cback;
static mqttnox_client_t* client= NULL;

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

int connection_thread(void* ptr);

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

int mqttnox_tcp_connect(char * addr, int port)
{
    int ret_val_t1 = 0;

    port_outgoing = port;

    // host = gethostbyname("127.0.0.1");
    // if (!host)
    // {
    //     fprintf(stderr, "error: unknown host\n");
    //     return -1;
    // }

    printf("Starting Outgoing Client at port_outgoing %d\n", port);

    data_callback = mqttnox_tcp_rcv_cback;
    port_incoming = port;

    queueCond = CreateEvent(NULL, false, false, NULL);

    queueMutex = CreateMutex(
        NULL,              // default security attributes
        FALSE,             // initially not owned
        NULL);             // unnamed mutex

//    receive_thread = _beginthread((HANDLE)&server_incoming_thread_handler, 0, NULL);


    // printf("Starting Receiver port %d\n", port);


    // /* Create server_thread */
    // ret_val_t1 = pthread_create(&client_thread, NULL, client_outgoing_thread_handler, NULL);
    // if (ret_val_t1)
    // {
    //     fprintf(stderr, "Error - pthread_create() return value: %d\n", ret_val_t1);
    //     exit(EXIT_FAILURE);
    // }    

    int iResult;
    struct sockaddr_in server;
    void* ptr = NULL;
    char addrstr[32];

    int sock = -1;
    
    connection_t * connection;
    
    printf("Client TCP thread\n");

    struct addrinfo* result = NULL;
    struct addrinfo hints;
        
    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return -1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    char port_str[32];
    snprintf(port_str, 32, "%d", port_incoming);

    // Resolve the server address and port
    iResult = getaddrinfo(addr, port_str, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return -1;
    }

    // Create a SOCKET for connecting to server
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
	server.sin_port = htons( port_incoming );

	//Connect to remote server
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
    connection_thread_obj = (HANDLE)_beginthread(&connection_thread, 0, (void*)connection);

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




uint8_t main_buffer[1024];

int connection_thread(void * ptr)
{
    //char * buffer;
    DWORD dwCount = 0, dwWaitResult;
    int iResult;
    int len = 0;
    connection_t * conn;
    //long addr = 0;
    int run = 1;

    if (!ptr) run = 0;

    printf("Here");
    conn = (connection_t *)ptr;

    printf("Running");

    while(run)
    {

        /* read length of message */
        //read(conn->sock, &len, sizeof(char));
        //read(conn->sock, main_buffer, sizeof(main_buffer));


        iResult = recv(ClientSocket, main_buffer, sizeof(main_buffer), 0);
#if 0  
        if (iResult > 0)
            printf("Bytes received: %d\n", iResult);
        else
            printf("Nothing received\n");
#endif
        len = iResult;

//        len = recvfrom(conn->sock, main_buffer, sizeof(main_buffer), 0, NULL, NULL);
        if (len > 0)
        {
            //memset(main_buffer, 0, sizeof(main_buffer));
            //addr = (long)((struct sockaddr_in *)&conn->address)->sin_addr.s_addr;
            //buffer = (char *)malloc((len+1)*sizeof(char));
            //buffer[len] = 0;

            /* Received data */
            //read(conn->sock, main_buffer, len);

            /* Lock the queue mutex to make sure that adding data to the queue happens correctly */

            dwWaitResult = WaitForSingleObject(
                queueMutex,    // handle to mutex
                INFINITE);  // no time-out interval

            int i = 0;
            printf("Here Received %d\n", len);

            printf("-----Data[%d]: ", len);
            
            for(i = 0; i < len; i++)
            {
                printf("%02X ", main_buffer[i]);
            }

            printf("\n");


            memcpy(received_data, main_buffer, len);
            received_length = len;

            if(mqttnox_tcp_rcv_cback != NULL) {
                data_callback(client, received_data, received_length);
            }

            ReleaseMutex(queueMutex);

            /* Wait for response from stack */


            printf("%d Going to lock\n", __LINE__);

            /* Start by locking the queue mutex */
#if 0
            dwWaitResult = WaitForSingleObject(
                queueMutex,    // handle to mutex
                INFINITE);  // no time-out interval
#endif

            printf("%d Server ready\n", __LINE__);

            server_ready = 1;

            //As long as the queue is empty,
            //while(send_length == 0) {
                // - wait for the condition variable to be signalled
                //Note: This call unlocks the mutex when called and
                //relocks it before returning!
                //pthread_cond_wait(&queueCond, &queueMutex);

                dwWaitResult = WaitForSingleObject(
                    queueCond, // event handle
                    INFINITE);    // indefinite wait



            //}


                //printf("%d Here about to send %d bytes\n", __LINE__, send_length);

#if DEBUG_ENABLED
            printf("Received some data to send of length %d   ", send_length);

            for(i = 0; i < send_length; i++)
            {
                printf("%x ", send_data[i]);
            }

            printf("\n");
#endif
            /* Send data */
             //write(sock, &send_length, sizeof(send_length));

           

            //Now unlock the mutex
            //pthread_mutex_unlock(&queueMutex);
            ReleaseMutex(queueMutex);

            /* Signal the condition variable that new data is available in the queue */
            //pthread_cond_signal(&queueCond);

            SetEvent(queueCond);


            /* Done, unlock the mutex */
            //pthread_mutex_unlock(&queueMutex);
            #if 0
            dwWaitResult = WaitForSingleObject(
                queueMutex,    // handle to mutex
                INFINITE);  // no time-out interval
#endif
            ReleaseMutex(queueMutex);


            len = 0;
            //free(buffer);
        }
        else if (len == 0)
        {
            /* Connection has been closed */
            closesocket(ClientSocket);
            run = 0;
        }

    }
    /* close socket and clean up */
    //close(conn->sock);
    
    //free(conn);
    //pthread_exit(0);

    return 0;
}

void mqttnox_wait_thread(void)
{
    WaitForSingleObject(connection_thread_obj, INFINITE);
}



#ifdef __cplusplus
}
#endif