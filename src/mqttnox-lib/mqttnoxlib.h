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
* File:    mqttnoxlib.h
* Summary: MQTT Nox Internal Library definitions
*
* Note: Do not call the functions in this file directly. Use mqttnox.h
*
*/

#ifndef _MQTTNOXLIB_H_
#define _MQTTNOXLIB_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "mqttnox_err.h"
#include "common.h"

/** Flag inidicating initialization */
#define MQTTNOX_INIT_FLAG 0x41474E58

#define MQTT_CONN_PROTOCOL_NAME_LEN   4
#define MQTT_CONN_DEFAULT_KEEPALIVE   60
#define MQTT_CONN_PROTOCOL_NAME       "MQTT"
#define MQTT_PROTO_LVL_VERSION_V3_1_1 4
#define MQTT_MAX_DEVICE_ID_LEN        23
#define MAX_STR_LEN                   64
#define MQTT_LENGTH_FIELD_OFFSET      1

typedef enum
{
    MQTTNOX_CONNECTION_RC_ACCEPTED                = 0x00,
    MQTTNOX_CONNECTION_RC_REFUSED_UNACCP_PROT_VER = 0x01, /* Connection Refused, unacceptable protocol version */
    MQTTNOX_CONNECTION_RC_REFUSED_IDENT_REJECTED  = 0x02, /* Connection Refused, identifier rejected */
    MQTTNOX_CONNECTION_RC_REFUSED_SERVER_UNAVAIL  = 0x03, /* Connection Refused, Server Unavailable */
    MQTTNOX_CONNECTION_RC_REFUSED_BAD_USER_PASS   = 0x04, /* Connection Refused, Bad Username or Password */
    MQTTNOX_CONNECTION_RC_REFUSED_NOT_AUTH        = 0x05, /* Connection Refused, Not authorized */

} mqttnox_connect_rc_t;



#pragma pack(push, 1)
typedef struct {

    uint8_t name_len_msb;
    uint8_t name_len_lsb;
    char     name_val[MQTT_CONN_PROTOCOL_NAME_LEN];

    uint8_t level_val;

    uint8_t flag_user_name : 1;
    uint8_t flag_password  : 1;
    uint8_t flag_will_retain : 1;
    uint8_t flag_will_qos : 2;
    uint8_t flag_will : 1;
    uint8_t flag_clean_session : 1;
    uint8_t flag_reserved : 1;

    uint8_t keepalive_msb;
    uint8_t keepalive_lsb;

} mqttnox_connect_var_hdr_t;

#pragma pack(pop)


#pragma pack(push, 1)
typedef struct {

    uint8_t flag_session_present: 1;
    uint8_t flag_user_name : 7;
    uint8_t conn_return_code;

} mqttnox_connack_var_hdr_t;

#pragma pack(pop)



typedef union
{
    mqttnox_connack_var_hdr_t conn_ack;


} mqttnox_response_var_hdr_t;



#pragma pack(push, 1)
typedef enum {

    MQTTNOX_CTRL_PKT_TYPE_RESERVED    = 0,  /* Reserved and must not be used */
    MQTTNOX_CTRL_PKT_TYPE_CONNECT     = 1,  /* MQTT CONNECT */
    MQTTNOX_CTRL_PKT_TYPE_CONNACK     = 2,  /* MQTT CONNACK */
    MQTTNOX_CTRL_PKT_TYPE_PUBLISH     = 3,  /* MQTT PUBLISH */
    MQTTNOX_CTRL_PKT_TYPE_PUBACK      = 4,  /* MQTT PUBACK */
    MQTTNOX_CTRL_PKT_TYPE_PUBREC      = 5,  /* MQTT PUBREC */
    MQTTNOX_CTRL_PKT_TYPE_PUBREL      = 6,  /* MQTT PUBREL */
    MQTTNOX_CTRL_PKT_TYPE_PUBCOMP     = 7,  /* MQTT PUBCOMP */
    MQTTNOX_CTRL_PKT_TYPE_SUBSCRIBE   = 8,  /* MQTT SUBSCRIBE */
    MQTTNOX_CTRL_PKT_TYPE_SUBACK      = 9,  /* MQTT SUBACK */
    MQTTNOX_CTRL_PKT_TYPE_UNSUBSCRIBE = 10, /* MQTT UNSUBSCRIBE */
    MQTTNOX_CTRL_PKT_TYPE_UNSUBACK    = 11, /* MQTT UNSUBACK */
    MQTTNOX_CTRL_PKT_TYPE_PINGREQ     = 12, /* MQTT PINGREQ */
    MQTTNOX_CTRL_PKT_TYPE_PINGRESP    = 13, /* MQTT PINGRESP */
    MQTTNOX_CTRL_PKT_TYPE_DISCONNECT  = 14, /* MQTT DISCONNECT */

} mqttnox_ctrl_pkt_type_t;
#pragma pack(pop)

/* Although the C standard does not enfoce bit fields, on platforms */
#pragma pack(push, 1)

typedef struct
{
    uint8_t retain               : 1;
    uint8_t qos                  : 2;
    uint8_t dup                  : 1;
    uint8_t type : 4;

} _COMPILER_PACK mqttnox_hdr_t;

#pragma pack(pop)



extern int mqttnoxlib_validate_device_id(char* str);


#ifdef __cplusplus
}
#endif


#endif /* _MQTTNOXLIB_H_ */