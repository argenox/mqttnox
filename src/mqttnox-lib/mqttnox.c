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
* File:    mqttnox.h
* Summary: MQTT Nox External APIs
*
*/

#ifdef __cplusplus
extern "C" {
#endif

/* System Includes */
#include <stdint.h>
#include <string.h>
#include <stdio.h>

/* Library Includes */
#include "mqttnox.h"
#include "mqttnoxlib.h"
#include "mqttnox_err.h"
#include "common.h"
#include "mqttnox_tal.h"

#define MQTTNOX_TX_BUF_SIZE 256


static uint8_t mqttnox_tx_buf[MQTTNOX_TX_BUF_SIZE];

static int mqttnox_append_utf8_string(uint8_t* buffer, char* str);

void mqttnox_tcp_rcv_func(mqttnox_client_t* c, uint8_t * data, uint16_t len)
{
    mqttnox_hdr_t* hdr = NULL;
    mqttnox_response_var_hdr_t * var_hdr;

    do
    {
        if (data == NULL) {
            break;
        }

        hdr = (mqttnox_hdr_t*)data;
        var_hdr = (mqttnox_response_var_hdr_t*) (data + sizeof(mqttnox_hdr_t) + 2);

        switch (hdr->type) {
            
            case MQTTNOX_CTRL_PKT_TYPE_CONNACK:
                printf("MQTTNOX_CTRL_PKT_TYPE_CONNACK\n");                
                printf("Return Code %x\n", var_hdr->conn_ack.conn_return_code);
                printf("Session Present %u\n", var_hdr->conn_ack.flag_session_present);

                if (c != NULL && c->callback != NULL) {
                    c->callback(MQTTNOX_EVT_CONNECT, NULL);
                }

                break;            
            case MQTTNOX_CTRL_PKT_TYPE_PUBACK:
                printf("MQTTNOX_CTRL_PKT_TYPE_PUBACK\n");
                break;
            case MQTTNOX_CTRL_PKT_TYPE_PUBREC:
                printf("MQTTNOX_CTRL_PKT_TYPE_PUBREC\n");
                break;
            case MQTTNOX_CTRL_PKT_TYPE_PUBREL:
                printf("MQTTNOX_CTRL_PKT_TYPE_PUBREL\n");
                break;
            case MQTTNOX_CTRL_PKT_TYPE_PUBCOMP:
                printf("MQTTNOX_CTRL_PKT_TYPE_PUBCOMP\n");
                break;            
            case MQTTNOX_CTRL_PKT_TYPE_SUBACK:
                printf("MQTTNOX_CTRL_PKT_TYPE_SUBACK\n");
                break;            
            case MQTTNOX_CTRL_PKT_TYPE_UNSUBACK:
                printf("MQTTNOX_CTRL_PKT_TYPE_UNSUBACK\n");
                break;            
            case MQTTNOX_CTRL_PKT_TYPE_PINGRESP:
                printf("MQTTNOX_CTRL_PKT_TYPE_PINGRESP\n");
                break;
            case MQTTNOX_CTRL_PKT_TYPE_DISCONNECT:
                printf("MQTTNOX_CTRL_PKT_TYPE_DISCONNECT\n");
                break;

        }

        

    } while (0);


}

/**@brief Initialization of the MQTT Client
*
* @note This should be called only once and must be called prior to any
*       other library functions being used.
*
* @param[in]   c   mqttnox object \see mqttnox_client_t
*/
mqttnox_rc_t mqttnox_init(mqttnox_client_t * c)
{
    memset((void *)c, 0, sizeof(mqttnox_client_t));

    /* Set to initialized */
    c->flag_initialized = MQTTNOX_INIT_FLAG;

    return MQTTNOX_SUCCESS;
}

/**@brief Connects to an MQTT Broker
*
* @note This macro will delay concatenation until the expressions have been resolved
*
* @param[in]   c     MQTTNox Client
* @param[in]   conf  MQTT Client Configuration
*/
mqttnox_rc_t mqttnox_connect(mqttnox_client_t * c, mqttnox_client_conf_t * conf)
{
    mqttnox_rc_t rc = MQTTNOX_RC_ERROR;
    mqttnox_hdr_t hdr;
    mqttnox_connect_var_hdr_t var_hdr;
    uint16_t pkt_len = 0;
    int irc;


    do
    {
        if(c->flag_initialized != MQTTNOX_INIT_FLAG) {
            rc = MQTTNOX_RC_ERROR_NOT_INIT;
            break;
        }

        MEMZERO_S(hdr);
        MEMZERO_S(var_hdr);

        if (conf->callback != NULL) {
            c->callback = conf->callback;
        }


        hdr.type = MQTTNOX_CTRL_PKT_TYPE_CONNECT;

        /* Initialize variable header */

        var_hdr.name_len_msb = MSB(MQTT_CONN_PROTOCOL_NAME_LEN);
        var_hdr.name_len_lsb = LSB(MQTT_CONN_PROTOCOL_NAME_LEN);
        memcpy(var_hdr.name_val, MQTT_CONN_PROTOCOL_NAME, MQTT_CONN_PROTOCOL_NAME_LEN);

        /* Support for v3.1.1 Version of protocol ONLY */
        var_hdr.level_val = MQTT_PROTO_LVL_VERSION_V3_1_1;

        var_hdr.keepalive_msb = MSB(MQTT_CONN_DEFAULT_KEEPALIVE);
        var_hdr.keepalive_lsb = LSB(MQTT_CONN_DEFAULT_KEEPALIVE);

        /* Password flag must be set to 0 if user is not present in payload*/
        var_hdr.flag_user_name = 0;
        var_hdr.flag_password = 0;

        if (conf->auth.username != NULL && strlen(conf->auth.username)) {
            var_hdr.flag_user_name = 1;         

            if (conf->auth.password != NULL && strlen(conf->auth.password)) {
                var_hdr.flag_password = 1;
            }
        }

        var_hdr.flag_clean_session = 0;
        if (conf->clean_session) {
            var_hdr.flag_clean_session = 1;
        }
       
        /* If either msg or topic are specified, then we need both msg and topic */
        if (conf->will_topic.msg == NULL && conf->will_topic.topic == NULL) {
            var_hdr.flag_will = 0;
            var_hdr.flag_will_retain = 0;
            var_hdr.flag_will_qos = 0;
        }
        else
        {
            if (conf->will_topic.msg != NULL && (strlen(conf->will_topic.msg) > 0) ||
                conf->will_topic.topic != NULL && (strlen(conf->will_topic.topic) > 0)) {

                var_hdr.flag_will_qos = (conf->will_topic.qos & 0x03);
                var_hdr.flag_will_retain = (conf->will_topic.retain & 0x01);
            }
        }

        /* While the specification allows zero length client id, we don't to reduce conflicts */
        if (mqttnoxlib_validate_device_id(conf->client_identifier) != 0)
        {
            rc = MQTTNOX_RC_ERROR_BAD_CLIENT_IDENT;
            break;
        }

        int rc_i = mqttnox_tcp_init(c, mqttnox_tcp_rcv_func);
        rc_i = mqttnox_tcp_connect(conf->server.addr, conf->server.port);

        if (rc_i != 0) {
            printf("Connect failed");
        }

        MEMZERO(mqttnox_tx_buf);

        /* Copy Fixed Header */
        memcpy(mqttnox_tx_buf, (void *)&hdr, sizeof(hdr));
        pkt_len += sizeof(hdr);

        /* Skip Remaining length for now - updated later */
        pkt_len += 1;

        /* Copy Variable Header*/ 
        memcpy(&mqttnox_tx_buf[pkt_len], (void*)&var_hdr, sizeof(var_hdr));
        pkt_len += sizeof(var_hdr);

        irc = mqttnox_append_utf8_string(&mqttnox_tx_buf[pkt_len], conf->client_identifier);
        if (irc > 0) {
            pkt_len += irc;
        }

        if (var_hdr.flag_will) {

            irc = mqttnox_append_utf8_string(&mqttnox_tx_buf[pkt_len], conf->will_topic.topic);
            if (irc > 0) {
                pkt_len += irc;
            }

            irc = mqttnox_append_utf8_string(&mqttnox_tx_buf[pkt_len], conf->will_topic.msg);
            if (irc > 0) {
                pkt_len += irc;
            }
        }

        if (var_hdr.flag_user_name) {

            irc = mqttnox_append_utf8_string(&mqttnox_tx_buf[pkt_len], conf->auth.username);
            if (irc > 0) {
                pkt_len += irc;
            }

            if (var_hdr.flag_password) {

                irc = mqttnox_append_utf8_string(&mqttnox_tx_buf[pkt_len], conf->auth.password);
                if (irc > 0) {
                    pkt_len += irc;
                }
            }
        }

        mqttnox_tx_buf[MQTT_LENGTH_FIELD_OFFSET] = pkt_len - 2;

        /* Send the connect packet, response is received async */
        rc_i = mqttnox_tcp_send(mqttnox_tx_buf, pkt_len);
        

        rc = MQTTNOX_SUCCESS;
    } while(0);

    return rc;
}

static int mqttnox_append_utf8_string(uint8_t* buffer, char* str)
{
    uint16_t len = 0;
    uint16_t str_len = 0;
    uint16_t buf_offset = 0;

    if (buffer != NULL) {
        buf_offset = strlen(buffer);
    }

    if( str != NULL && (strlen(str) > 0) && buffer != NULL) {

        str_len = strlen(str);

        buffer[buf_offset + len++] = (str_len & 0xFF00) >> 8;
        buffer[buf_offset + len++] = (str_len & 0x00FF);

        memcpy(&buffer[buf_offset + len], (void*)str, str_len);

        return str_len + len;
    }

    return -1;
}

/**@brief Macro used to concatenate string using delayed macro expansion
*
* @note This macro will delay concatenation until the expressions have been resolved
*
* @param[in]   lhs   Left hand side in concatenation
* @param[in]   rhs   Right hand side in concatenation
*/
mqttnox_rc_t mqttnox_publish(mqttnox_client_t * c)
{
    mqttnox_rc_t rc = MQTTNOX_RC_ERROR;
    do
    {
        if(c->flag_initialized != MQTTNOX_INIT_FLAG) {
            rc = MQTTNOX_RC_ERROR_NOT_INIT;
            break;
        }

        rc = MQTTNOX_SUCCESS;
    } while(0);

    return rc;
}

/**@brief Macro used to concatenate string using delayed macro expansion
*
* @note This macro will delay concatenation until the expressions have been resolved
*
* @param[in]   lhs   Left hand side in concatenation
* @param[in]   rhs   Right hand side in concatenation
*/
mqttnox_rc_t mqttnox_subscribe(mqttnox_client_t * c)
{
    mqttnox_rc_t rc = MQTTNOX_RC_ERROR;
    do
    {
        if(c->flag_initialized != MQTTNOX_INIT_FLAG) {
            rc = MQTTNOX_RC_ERROR_NOT_INIT;
            break;
        }

        rc = MQTTNOX_SUCCESS;

    } while(0);

    return rc;
}

/**@brief Macro used to concatenate string using delayed macro expansion
*
* @note This macro will delay concatenation until the expressions have been resolved
*
* @param[in]   lhs   Left hand side in concatenation
* @param[in]   rhs   Right hand side in concatenation
*/
mqttnox_rc_t mqttnox_disconnect(mqttnox_client_t * c)
{
    mqttnox_rc_t rc = MQTTNOX_RC_ERROR;
    do
    {
        if(c->flag_initialized != MQTTNOX_INIT_FLAG) {
            rc = MQTTNOX_RC_ERROR_NOT_INIT;
            break;
        }

        rc = MQTTNOX_SUCCESS;
    } while(0);

    return rc;
}


#ifdef __cplusplus
}
#endif