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
* File:    mqtt_tal.c
* Summary: MQTT TCP Abstraction Layer
*
* Note:  APIs which must be implemented by the target platform
*
*/

#include <stdint.h>
#include "mqttnox.h"

#ifndef _MQTTNOX_TAL_H_
#define _MQTTNOX_TAL_H_

#ifdef __cplusplus
extern "C" {
#endif

/* APIs which must be implemented by the target platform */

typedef void (*mqttnox_tcp_rcv_t)(mqttnox_client_t* c, uint8_t * data, uint16_t len);


extern int mqttnox_tcp_init(mqttnox_client_t* c, mqttnox_tcp_rcv_t rcv_cback);
extern int mqttnox_tcp_connect(char* addr, int port);
extern int mqttnox_tcp_send(uint8_t * data, uint16_t len);
extern int mqttnox_tcp_disconnect(void);
extern void mqttnox_wait_thread(void);

#ifdef __cplusplus
}
#endif


#endif /* _MQTTNOX_TAL_H_ */