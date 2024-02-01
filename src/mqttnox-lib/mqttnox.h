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
* File:    mqttnox.h
* Summary: MQTT Nox External APIs
*
*/

#ifndef _MQTTNOX_H_
#define _MQTTNOX_H_

#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

#include "mqttnox_err.h"
#include "mqttnoxlib.h"
#include "mqttnox_version.h"

/** MQTTNox QoS Levels */
typedef enum
{
    MQTTNOX_QOS0_AT_MOST_ONCE_DELIV  = 0,
    MQTTNOX_QOS1_AT_LEAST_ONCE_DELIV = 1,
    MQTTNOX_QOS2_EXACTLY_ONCE_DELIV  = 2,
} mqttnox_qos_t;

/** MQTTNox Events  */
typedef enum {
    MQTTNOX_EVT_CONNECT,
    MQTTNOX_EVT_CONNECT_ERROR,
    MQTTNOX_EVT_PUBLISHED,
    MQTTNOX_EVT_SUBSCRIBED,
    MQTTNOX_EVT_UNSUBSCRIBED,
    MQTTNOX_EVT_PINGRESP,
    MQTTNOX_EVT_PUBREL,
    MQTTNOX_EVT_DISCONNECT,
    MQTTNOX_EVT_ERROR,

} mqttnox_evt_id_t;

/** MQTT Subscription acknowledgement return codes */
typedef enum {

    MQTTNOX_SUBACK_RETURN_SUCCESS_QOS0 = 0x00, /* Success - Maximum QoS 0 */
    MQTTNOX_SUBACK_RETURN_SUCCESS_QOS1 = 0x01, /* Success - Maximum QoS 1 */
    MQTTNOX_SUBACK_RETURN_SUCCESS_QOS2 = 0x02, /* Success - Maximum QoS 2 */
    MQTTNOX_SUBACK_RETURN_FAILURE      = 0x80, /* Failure */

} mqttnox_suback_return_t;


/** Connection Error Reason codes */
typedef enum {
    MQTTNOX_CONN_ERR_REFUSED_UNACCP_PROT_VER = 0x01, /* Connection Refused, unacceptable protocol version */
    MQTTNOX_CONN_ERR_REFUSED_IDENT_REJECTED = 0x02,  /* Connection Refused, identifier rejected */
    MQTTNOX_CONN_ERR_REFUSED_SERVER_UNAVAIL = 0x03,  /* Connection Refused, Server Unavailable */
    MQTTNOX_CONN_ERR_REFUSED_BAD_USER_PASS = 0x04,   /* Connection Refused, Bad Username or Password */
    MQTTNOX_CONN_ERR_REFUSED_NOT_AUTH = 0x05,        /* Connection Refused, Not authorized */

} mqttnox_conn_err_reason_t;

typedef struct
{
    uint8_t session_present;

} connect_evt_t;

typedef struct
{
    mqttnox_conn_err_reason_t reason;

} connect_error_evt_t;

typedef struct
{
    uint8_t packet_identified_msb;
    uint8_t packet_identified_lsb;

} published_evt_t;


typedef struct
{
    mqttnox_suback_return_t return_code;

} subscribed_evt_t;

typedef struct
{
    uint8_t packet_identified_msb;
    uint8_t packet_identified_lsb;

} unsubscribed_evt_t;

typedef struct
{
    uint8_t test;

} pingresp_evt_t;

typedef struct
{
    uint8_t test;

} pubrel_evt_t;

typedef struct
{
    uint8_t test;

} disconnect_evt_t;

/** MQTTNox Event Data */
typedef struct
{
    mqttnox_evt_id_t evt_id; /* Indicates which event occured */

    /* Event information */
    union {
        connect_evt_t      connect_evt;
        connect_error_evt_t conn_err_evt;
        published_evt_t    published_evt;
        subscribed_evt_t   subscribed_evt;
        unsubscribed_evt_t unsubscribed_evt;
        pingresp_evt_t     pingresp_evt;
        pubrel_evt_t       pubrel_evt;
        disconnect_evt_t   disconnect_evt;
    }evt;

} mqttnox_evt_data_t;


/* Callback */
typedef void (*mqttnox_callback_t)(mqttnox_evt_data_t * evt_data);

typedef struct
{
    uint32_t flag_initialized; /** Inidiates the client object is successfully initialized */

    struct  {
        uint8_t connected : 1;
    } status;

    mqttnox_callback_t callback;
    uint16_t packet_ident;
    uint16_t keepalive;

    mqttnox_debug_lvl_t debug_lvl;

} mqttnox_client_t;

typedef struct
{
    struct {
        char* addr; /* URL or IP Address*/
        uint16_t port; /* Port to use */
    } server;

    struct {
        char* username;
        char* password;
    } auth;

    /* Will Topic */

    struct  {
        char*   topic;
        char*   msg;
        mqttnox_qos_t qos;
        uint8_t retain;
    } will_topic;

    uint8_t clean_session;

    char* client_identifier; /* Unique Client Identifier - Usually up to 23 characters */

    /** Callback used for async event handling. Note that this callback is called in the context
        of the mqttnox thread, so care must be taken to avoid a stack overflow by either increasing
        the mqttnox thread's stack, or by minimizing stack usage and passing event data to a task
        for processing
     */
    mqttnox_callback_t callback;

} mqttnox_client_conf_t;


typedef struct
{
    char* topic;
    mqttnox_qos_t qos;

} mqttnox_topic_sub_t;



extern mqttnox_rc_t mqttnox_init(mqttnox_client_t * c, mqttnox_debug_lvl_t lvl);
extern mqttnox_rc_t mqttnox_connect(mqttnox_client_t* c, mqttnox_client_conf_t* conf, uint16_t keepalive);
extern mqttnox_rc_t mqttnox_publish(mqttnox_client_t* c,
                                    mqttnox_qos_t qos,
                                    uint8_t retain,
                                    uint8_t dup,
                                    char* topic,
                                    char* msg);
extern mqttnox_rc_t mqttnox_subscribe(mqttnox_client_t* c,
                                mqttnox_topic_sub_t* topics,
                                uint8_t topic_cnt);
extern mqttnox_rc_t mqttnox_unsubscribe(mqttnox_client_t* c,
    mqttnox_topic_sub_t* topics,
    uint8_t topic_cnt);

extern mqttnox_rc_t mqttnox_disconnect(mqttnox_client_t * c);
extern uint8_t mqttnox_is_connected(mqttnox_client_t* c);

#ifdef __cplusplus
}
#endif

#endif /* _MQTTNOX_H_ */
