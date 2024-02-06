/*****************************************************************************
* Copyright (c) [2024] Argenox Technologies LLC
* All rights reserved.
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
#include "mqttnox_config.h"
#include "mqttnox_debug.h"


static uint8_t mqttnox_tx_buf[MQTTNOX_TX_BUF_SIZE];

/* Intrnal Helper Functions */
static int mqttnox_append_utf8_string(uint8_t* buffer, const char* str, uint8_t add_len);
static void mqttnox_send_event(mqttnox_client_t* c, mqttnox_evt_data_t* data);

/* MQTT Response Handlers */
static void mqttnox_handler_connack(mqttnox_client_t* c, uint8_t * data, uint16_t len);
static void mqttnox_handler_publish(mqttnox_client_t* c, uint8_t* data, uint16_t len);
static void mqttnox_handler_puback(mqttnox_client_t* c, uint8_t * data, uint16_t len);
static void mqttnox_handler_pubrec(mqttnox_client_t* c, uint8_t * data, uint16_t len);
static void mqttnox_handler_pubrel(mqttnox_client_t* c, uint8_t * data, uint16_t len);
static void mqttnox_handler_pubcomp(mqttnox_client_t* c, uint8_t * data, uint16_t len);
static void mqttnox_handler_suback(mqttnox_client_t* c, uint8_t * data, uint16_t len);
static void mqttnox_handler_unsuback(mqttnox_client_t* c, uint8_t * data, uint16_t len);
static void mqttnox_handler_pingresp(mqttnox_client_t* c, uint8_t * data, uint16_t len);
static mqttnox_rc_t mqttnox_puback(mqttnox_client_t* c, uint16_t identifier);

static mqttnox_rc_t mqttnox_pubrec(mqttnox_client_t* c, uint16_t identifier);
static mqttnox_rc_t mqttnox_pubcomp(mqttnox_client_t* c, uint16_t identifier);
static mqttnox_rc_t mqttnox_pubrel(mqttnox_client_t* c, uint16_t identifier);

static int mqttnox_set_remain_len(uint8_t* buffer, uint32_t len);
static int mqttnox_decode_remain_len(uint8_t* buffer, uint32_t* len);

/**@brief Initialization of the MQTT Client
*
* @note This should be called only once and must be called prior to any
*       other library functions being used.
*
* @param[in]   c   mqttnox object \see mqttnox_client_t
*/
mqttnox_rc_t mqttnox_init(mqttnox_client_t* c, mqttnox_debug_lvl_t lvl)
{
    memset((void*)c, 0, sizeof(mqttnox_client_t));

    c->packet_ident = 17;
    c->debug_lvl = lvl;
    c->keepalive = MQTT_CONN_DEFAULT_KEEPALIVE;

    /* Set to initialized */
    c->flag_initialized = MQTTNOX_INIT_FLAG;

    return MQTTNOX_SUCCESS;
}

/**@brief TCP callback for data reception
*
*
* @param[in]   c    mqttnox object \see mqttnox_client_t
* @param[in]   data pointer to buffer with the incoming data from the server
* @param[in]   len  length of the data from the server
*
*/
void mqttnox_tcp_rcv_func(mqttnox_client_t* c, uint8_t * data, uint16_t len)
{
    mqttnox_hdr_t* hdr = NULL;

    mqttnox_debug_printf(c, MQTTNOX_DEBUG_LVL_DEBUG, "TCP Receive Function\n");
    
    do
    {
        if (c == NULL) {
            mqttnox_debug_printf(c, MQTTNOX_DEBUG_LVL_ERROR, "Client NULL %s Line %d\n", __FILE__, __LINE__);
            break;
        }

        if(c->flag_initialized != MQTTNOX_INIT_FLAG) {
            mqttnox_debug_printf(c, MQTTNOX_DEBUG_LVL_ERROR, "Not initialized\n");
            break;
        }

        if (data == NULL || len == 0) {
            mqttnox_debug_printf(c, MQTTNOX_DEBUG_LVL_ERROR, "Data NULL or zero length %s Line %d\n", __FILE__, __LINE__);
            break;
        }

        hdr = (mqttnox_hdr_t*)data;

        switch (hdr->type) {

            case MQTTNOX_CTRL_PKT_TYPE_CONNACK:
                c->status.connected = 1;
                mqttnox_handler_connack(c, data, len);
                break;
            case MQTTNOX_CTRL_PKT_TYPE_PUBLISH:
                mqttnox_handler_publish(c, data, len);
                break;
            case MQTTNOX_CTRL_PKT_TYPE_PUBACK:                
                mqttnox_handler_puback(c, data, len);
                break;
            case MQTTNOX_CTRL_PKT_TYPE_PUBREC:
                mqttnox_debug_printf(c, MQTTNOX_DEBUG_LVL_DEBUG, "MQTTNOX_CTRL_PKT_TYPE_PUBREC\n");
                mqttnox_handler_pubrec(c, data, len);
                break;
            case MQTTNOX_CTRL_PKT_TYPE_PUBREL:
                mqttnox_debug_printf(c, MQTTNOX_DEBUG_LVL_DEBUG, "MQTTNOX_CTRL_PKT_TYPE_PUBREL\n");
                mqttnox_handler_pubrel(c, data, len);
                break;
            case MQTTNOX_CTRL_PKT_TYPE_PUBCOMP:
                mqttnox_debug_printf(c, MQTTNOX_DEBUG_LVL_DEBUG, "MQTTNOX_CTRL_PKT_TYPE_PUBCOMP\n");
                mqttnox_handler_pubcomp(c, data, len);
                break;
            case MQTTNOX_CTRL_PKT_TYPE_SUBACK:
                mqttnox_debug_printf(c, MQTTNOX_DEBUG_LVL_DEBUG, "MQTTNOX_CTRL_PKT_TYPE_SUBACK\n");
                mqttnox_handler_suback(c, data, len);
                break;
            case MQTTNOX_CTRL_PKT_TYPE_UNSUBACK:
                mqttnox_debug_printf(c, MQTTNOX_DEBUG_LVL_DEBUG, "MQTTNOX_CTRL_PKT_TYPE_UNSUBACK\n");
                mqttnox_handler_unsuback(c, data, len);
                break;
            case MQTTNOX_CTRL_PKT_TYPE_PINGRESP:
                mqttnox_debug_printf(c, MQTTNOX_DEBUG_LVL_DEBUG, "MQTTNOX_CTRL_PKT_TYPE_PINGRESP\n");
                mqttnox_handler_pingresp(c, data, len);
                break;
            default:
                mqttnox_debug_printf(c, MQTTNOX_DEBUG_LVL_ERROR, "Packet Type Error\n");
                break;
        }
    } while (0);
}

/**@brief MQTT ConnACK Handler
*
* @note Connection Acknowledgement Handler
*
* @param[in]   c    mqttnox object \see mqttnox_client_t
* @param[in]   data pointer to buffer with the incoming data from the server
* @param[in]   len  length of the data from the server
*
* @return     None
*/
static void mqttnox_handler_connack(mqttnox_client_t* c, uint8_t * data, uint16_t len)
{    
    uint8_t data_buffer[64];
    mqttnox_evt_data_t* evt_data = (mqttnox_evt_data_t*)data_buffer;
    mqttnox_response_var_hdr_t* var_hdr = (mqttnox_response_var_hdr_t*)(data + sizeof(mqttnox_hdr_t) + 2);

    mqttnox_debug_printf(c, MQTTNOX_DEBUG_LVL_DEBUG, "MQTTNOX_CTRL_PKT_TYPE_CONNACK\n");
    mqttnox_debug_printf(c, MQTTNOX_DEBUG_LVL_DEBUG, "Return Code %x\n", var_hdr->conn_ack.conn_return_code);
    mqttnox_debug_printf(c, MQTTNOX_DEBUG_LVL_DEBUG, "Session Present %u\n", var_hdr->conn_ack.flag_session_present);

    switch (var_hdr->conn_ack.conn_return_code)
    {
        case MQTTNOX_CONNECTION_RC_ACCEPTED:
            mqttnox_debug_printf(c, MQTTNOX_DEBUG_LVL_DEBUG, "Connection Successful\n");        
            evt_data->evt_id = MQTTNOX_EVT_CONNECT;
            evt_data->evt.connect_evt.session_present = var_hdr->conn_ack.flag_session_present;
            mqttnox_send_event(c, evt_data);

            break;
        case MQTTNOX_CONNECTION_RC_REFUSED_UNACCP_PROT_VER:
            mqttnox_debug_printf(c, MQTTNOX_DEBUG_LVL_DEBUG, "Connection Refused, unacceptable protocol version\n");
            evt_data->evt_id = MQTTNOX_EVT_CONNECT_ERROR;
            evt_data->evt.conn_err_evt.reason = MQTTNOX_CONN_ERR_REFUSED_UNACCP_PROT_VER;
            mqttnox_send_event(c, evt_data);            
            break;
        case MQTTNOX_CONNECTION_RC_REFUSED_IDENT_REJECTED:
            mqttnox_debug_printf(c, MQTTNOX_DEBUG_LVL_DEBUG, "Connection Refused, identifier rejected\n");
            evt_data->evt_id = MQTTNOX_EVT_CONNECT_ERROR;
            evt_data->evt.conn_err_evt.reason = MQTTNOX_CONN_ERR_REFUSED_IDENT_REJECTED;
            mqttnox_send_event(c, evt_data);            
            break;
        case MQTTNOX_CONNECTION_RC_REFUSED_SERVER_UNAVAIL:
            mqttnox_debug_printf(c, MQTTNOX_DEBUG_LVL_DEBUG, "Connection Refused, Server Unavailable\n");
            evt_data->evt_id = MQTTNOX_EVT_CONNECT_ERROR;
            evt_data->evt.conn_err_evt.reason = MQTTNOX_CONN_ERR_REFUSED_SERVER_UNAVAIL;
            mqttnox_send_event(c, evt_data);            
            break;
        case MQTTNOX_CONNECTION_RC_REFUSED_BAD_USER_PASS:
            mqttnox_debug_printf(c, MQTTNOX_DEBUG_LVL_DEBUG, "Connection Refused, Bad Username or Password\n");
            evt_data->evt_id = MQTTNOX_EVT_CONNECT_ERROR;
            evt_data->evt.conn_err_evt.reason = MQTTNOX_CONN_ERR_REFUSED_BAD_USER_PASS;
            mqttnox_send_event(c, evt_data);            
            break;
        case MQTTNOX_CONNECTION_RC_REFUSED_NOT_AUTH:
            mqttnox_debug_printf(c, MQTTNOX_DEBUG_LVL_DEBUG, "Connection Refused, Not authorized\n");
            evt_data->evt_id = MQTTNOX_EVT_CONNECT_ERROR;
            evt_data->evt.conn_err_evt.reason = MQTTNOX_CONN_ERR_REFUSED_NOT_AUTH;
            mqttnox_send_event(c, evt_data);            
            break;
        default:
            mqttnox_debug_printf(c, MQTTNOX_DEBUG_LVL_ERROR, 
                                "Unknown error - invalid connection code 0x%x\n", 
                                var_hdr->conn_ack.conn_return_code);
    }
}

/**@brief MQTT Publish Handler
*
* @note Publish Ack Handler
*
* @param[in]   c    mqttnox object \see mqttnox_client_t
* @param[in]   data pointer to buffer with the incoming data from the server
* @param[in]   len  length of the data from the server
*
* @return     None
*/
static void mqttnox_handler_publish(mqttnox_client_t* c, uint8_t* data, uint16_t len)
{
    
    mqttnox_rc_t rc;
    mqttnox_hdr_t* hdr = (mqttnox_hdr_t*)data;    
    mqttnox_evt_data_t  evt_data;    
    uint32_t remain_length = 0;
    int remain_len_byte = 0;
    size_t offset = 0;

    do
    {

        remain_len_byte = mqttnox_decode_remain_len(&data[sizeof(mqttnox_hdr_t)], &remain_length);
        if (remain_len_byte < 0 || remain_len_byte > 4) {
            /* Remain length is wrong */
            mqttnox_debug_printf(c, MQTTNOX_DEBUG_LVL_ERROR, "Remain Length is wrong: %u bytes \n", remain_len_byte);
        }

        mqttnox_debug_printf(c, MQTTNOX_DEBUG_LVL_DEBUG, "MQTTNOX_CTRL_PKT_TYPE_PUBLISH with QoS: %u\n", hdr->qos);


        /* Decode Topic Length */
        uint16_t topic_len = (data[sizeof(mqttnox_hdr_t) + remain_len_byte] << 8) | data[sizeof(mqttnox_hdr_t) + remain_len_byte + 1];
#if 0
        if (topic_len < sizeof(data_buffer)) {
            //memcpy(data_buffer, data[1], topic_len);
            evt_data.evt.received_evt.topic = data
        }
#endif

        offset = sizeof(mqttnox_hdr_t) + remain_len_byte + 2;

        evt_data.evt_id = MQTTNOX_EVT_RECEIVED;
        evt_data.evt.received_evt.topic = &data[offset];
        offset += topic_len;

        /* Packet identifier only present in packets with QoS of 1 or 2 */
        if (hdr->qos != MQTTNOX_QOS0_AT_MOST_ONCE_DELIV) {
            evt_data.evt.received_evt.packet_identifier = (data[offset] << 8) | data[offset + 1];
            offset += 2;
        }
        evt_data.evt.received_evt.payload = &data[offset];
        evt_data.evt.received_evt.payload_len = remain_length - MQTTNOX_PACKET_IDENT_BYTE_LEN - MQTTNOX_LENGTH_BYTE_LEN - topic_len;

        switch (hdr->qos)
        {
            case MQTTNOX_QOS0_AT_MOST_ONCE_DELIV:
                /* Nothing to do for QoS 0*/
                break;
            case MQTTNOX_QOS1_AT_LEAST_ONCE_DELIV:
                /* Send Puback */
                rc = mqttnox_puback(c, evt_data.evt.received_evt.packet_identifier);

                break;
            case MQTTNOX_QOS2_EXACTLY_ONCE_DELIV:
                /* Send PubRec */
                rc = mqttnox_pubrec(c, evt_data.evt.received_evt.packet_identifier);
                break;
        }

    } while (0);
    
    mqttnox_send_event(c, &evt_data);
}

/**@brief MQTT Pub Ack Handler
*
* @note Publish Ack Handler
*
* @param[in]   c    mqttnox object \see mqttnox_client_t
* @param[in]   data pointer to buffer with the incoming data from the server
* @param[in]   len  length of the data from the server
*
* @return     None
*/
static void mqttnox_handler_puback(mqttnox_client_t* c, uint8_t * data, uint16_t len)
{
    mqttnox_debug_printf(c, MQTTNOX_DEBUG_LVL_DEBUG, "MQTTNOX_CTRL_PKT_TYPE_PUBACK\n");

    uint8_t data_buffer[64];
    mqttnox_evt_data_t* evt_data = (mqttnox_evt_data_t*)data_buffer;

    mqttnox_response_var_hdr_t* var_hdr = (mqttnox_response_var_hdr_t*)(data + sizeof(mqttnox_hdr_t) + 1);

    evt_data->evt_id = MQTTNOX_EVT_PUBLISHED;
    evt_data->evt.published_evt.packet_identified_msb = var_hdr->unsub_ack.msb;
    evt_data->evt.published_evt.packet_identified_lsb = var_hdr->unsub_ack.lsb;

    mqttnox_send_event(c, evt_data);
}

/**@brief MQTT Pub Rec Handler
*
* @note Publish Rec Handler
*
* @param[in]   c    mqttnox object \see mqttnox_client_t
* @param[in]   data pointer to buffer with the incoming data from the server
* @param[in]   len  length of the data from the server
*
* @return     None
*/
static void mqttnox_handler_pubrec(mqttnox_client_t* c, uint8_t * data, uint16_t len)
{
    mqttnox_rc_t rc;

    do
    {
        uint16_t packet_identifier = (data[sizeof(mqttnox_hdr_t)] << 8) | data[sizeof(mqttnox_hdr_t) + 1];

        rc = mqttnox_pubrel(c, packet_identifier);

    } while (0);
}

/**@brief MQTT Pub Rel Handler
*
* @note Publish Rec Handler
*
* @param[in]   c    mqttnox object \see mqttnox_client_t
* @param[in]   data pointer to buffer with the incoming data from the server
* @param[in]   len  length of the data from the server
*
* @return     None
*/
static void mqttnox_handler_pubrel(mqttnox_client_t* c, uint8_t * data, uint16_t len)
{    
    mqttnox_rc_t rc;
    
    do
    {
        uint16_t packet_identifier = (data[sizeof(mqttnox_hdr_t)] << 8) | data[sizeof(mqttnox_hdr_t) + 1];

        rc = mqttnox_pubcomp(c, packet_identifier);       

    } while (0);
}

/**@brief MQTT Pub Comp Handler
*
* @note Publish Rec Handler
*
* @param[in]   c    mqttnox object \see mqttnox_client_t
* @param[in]   data pointer to buffer with the incoming data from the server
* @param[in]   len  length of the data from the server
*
* @return     None
*/
static void mqttnox_handler_pubcomp(mqttnox_client_t* c, uint8_t * data, uint16_t len)
{
    /* Not implemented yet */
}

/**@brief MQTT Sub ACK Handler
*
* @note Publish Subscription ACK Handler
*
* @param[in]   c    mqttnox object \see mqttnox_client_t
* @param[in]   data pointer to buffer with the incoming data from the server
* @param[in]   len  length of the data from the server
*
* @return     None
*/
static void mqttnox_handler_suback(mqttnox_client_t* c, uint8_t * data, uint16_t len)
{
    uint8_t data_buffer[64];
    mqttnox_evt_data_t* evt_data = (mqttnox_evt_data_t*)data_buffer;

    evt_data->evt_id = MQTTNOX_EVT_SUBSCRIBED;
    evt_data->evt.subscribed_evt.return_code = *(mqttnox_suback_return_t *)(data + sizeof(mqttnox_hdr_t) + 2);

    mqttnox_send_event(c, evt_data);
}

/**@brief MQTT Unsub ACK Handler
*
* @note Unsubscribe ACK Handler
*
* @param[in]   c    mqttnox object \see mqttnox_client_t
* @param[in]   data pointer to buffer with the incoming data from the server
* @param[in]   len  length of the data from the server
*
* @return     None
*/
static void mqttnox_handler_unsuback(mqttnox_client_t* c, uint8_t * data, uint16_t len)
{
    uint8_t data_buffer[64];
    mqttnox_evt_data_t* evt_data = (mqttnox_evt_data_t*)data_buffer;

    mqttnox_response_var_hdr_t* var_hdr = (mqttnox_response_var_hdr_t*)(data + sizeof(mqttnox_hdr_t) + 1);

    evt_data->evt_id = MQTTNOX_EVT_UNSUBSCRIBED;
    evt_data->evt.unsubscribed_evt.packet_identified_msb = var_hdr->unsub_ack.msb;
    evt_data->evt.unsubscribed_evt.packet_identified_lsb = var_hdr->unsub_ack.lsb;

    mqttnox_send_event(c, evt_data);
}

/**@brief MQTT Ping Response Handler
*
* @note Publish Rec Handler
*
* @param[in]   c    mqttnox object \see mqttnox_client_t
* @param[in]   data pointer to buffer with the incoming data from the server
* @param[in]   len  length of the data from the server
*
* @return     None
*/
static void mqttnox_handler_pingresp(mqttnox_client_t* c, uint8_t * data, uint16_t len)
{
    uint8_t data_buffer[64];
    mqttnox_evt_data_t* evt_data = (mqttnox_evt_data_t*)data_buffer;
    evt_data->evt_id = MQTTNOX_EVT_PINGRESP;

    mqttnox_send_event(c, evt_data);
}

/**@brief Send Event to callback
*
* @note Internal function
*
* @param[in]   c    mqttnox object \see mqttnox_client_t
* @param[in]   data pointer to buffer with the incoming data from the server
* @param[in]   len  length of the data from the server
*
* @return     None
*/
static void mqttnox_send_event(mqttnox_client_t* c, mqttnox_evt_data_t * data)
{
    /* Ensure callback is valid */
    if (c != NULL && c->callback != NULL) {
        c->callback(data);
    }
}




/**@brief Connects to an MQTT Broker
*
* @note This macro will delay concatenation until the expressions have been resolved
*
* @param[in]   c     MQTTNox Client
* @param[in]   conf  MQTT Client Configuration
*/
mqttnox_rc_t mqttnox_connect(mqttnox_client_t * c, mqttnox_client_conf_t * conf, uint16_t keepalive)
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

        c->keepalive = keepalive;
        var_hdr.keepalive_msb = MSB(keepalive);
        var_hdr.keepalive_lsb = LSB(keepalive);

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

                var_hdr.flag_will = 1;
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
            mqttnox_debug_printf(c, MQTTNOX_DEBUG_LVL_DEBUG, "Connect failed");
            break;
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

        irc = mqttnox_append_utf8_string(&mqttnox_tx_buf[pkt_len], conf->client_identifier, 1);
        if (irc > 0) {
            pkt_len += irc;
        }

        if (var_hdr.flag_will) {

            irc = mqttnox_append_utf8_string(&mqttnox_tx_buf[pkt_len], conf->will_topic.topic, 1);
            if (irc > 0) {
                pkt_len += irc;
            }

            irc = mqttnox_append_utf8_string(&mqttnox_tx_buf[pkt_len], conf->will_topic.msg, 1);
            if (irc > 0) {
                pkt_len += irc;
            }
        }

        if (var_hdr.flag_user_name) {

            irc = mqttnox_append_utf8_string(&mqttnox_tx_buf[pkt_len], conf->auth.username, 1);
            if (irc > 0) {
                pkt_len += irc;
            }

            if (var_hdr.flag_password) {

                irc = mqttnox_append_utf8_string(&mqttnox_tx_buf[pkt_len], conf->auth.password, 1);
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

#define MAX_REMAIN_LEN_BYTES (4)

/**@brief MQTT Publish
*
* @note This must be called when client has successfully connected
*
* @param[in]   c      MQTTNox Client object
* @param[in]   qos    Quality of Service for Delivery \see mqttnox_qos_t
* @param[in]   retain retain to send to future subscribers
* @param[in]   dup    Indicates this is the first sending of data (0) or a duplicate (1)
*/
mqttnox_rc_t mqttnox_publish(mqttnox_client_t * c, 
                             mqttnox_qos_t qos, 
                             uint8_t retain, 
                             uint8_t dup, 
                             char * topic,
                             char * msg)
{
    mqttnox_rc_t rc = MQTTNOX_RC_ERROR;
    mqttnox_hdr_t hdr;
    mqttnox_connect_var_hdr_t var_hdr;
    uint16_t pkt_len = 0;
    uint8_t remain_bytes = 0;
    int irc;

    /* We start at an offset because length is determined later.
       Leave enough space for max remaining length bytes and header */
    uint16_t offset = MAX_REMAIN_LEN_BYTES + sizeof(hdr);
    
    do
    {
        if (c->flag_initialized != MQTTNOX_INIT_FLAG) {
            rc = MQTTNOX_RC_ERROR_NOT_INIT;
            break;
        }

        MEMZERO_S(hdr);
        MEMZERO_S(var_hdr);

        /* Initialize fixed header */
        hdr.type = MQTTNOX_CTRL_PKT_TYPE_PUBLISH;
        hdr.dup = dup & 1;
        hdr.qos = qos & 0x03;
        hdr.retain = retain;

        MEMZERO(mqttnox_tx_buf);
                
        /* Setup Variable Header*/
        irc = mqttnox_append_utf8_string(&mqttnox_tx_buf[pkt_len + offset], topic, 1);
        if (irc > 0) {
            pkt_len += irc;
        }

        if (qos == MQTTNOX_QOS1_AT_LEAST_ONCE_DELIV || qos == MQTTNOX_QOS2_EXACTLY_ONCE_DELIV) {
            /* Add packet identifier */
            mqttnox_tx_buf[pkt_len + offset] = MSB(c->packet_ident);
            pkt_len++;
            mqttnox_tx_buf[pkt_len + offset] = LSB(c->packet_ident);
            pkt_len++;
            c->packet_ident++;
        }

        /* Msg does not have length */
        irc = mqttnox_append_utf8_string(&mqttnox_tx_buf[pkt_len + offset], msg, 0);
        if (irc > 0) {
            pkt_len += irc;
        }

        /* Set length backwards behind the data */
        remain_bytes = mqttnox_set_remain_len(&mqttnox_tx_buf[sizeof(hdr)], pkt_len);

        pkt_len += remain_bytes;

        /* Copy Fixed Header */
        memcpy(&mqttnox_tx_buf[offset - remain_bytes - 1], (void*)&hdr, sizeof(hdr));
        pkt_len += sizeof(hdr);

        /* Send the connect packet, response is received async */
        irc = mqttnox_tcp_send(&mqttnox_tx_buf[offset - remain_bytes - 1], pkt_len);


        rc = MQTTNOX_SUCCESS;
    } while (0);

    return rc;
}

/**@brief MQTT Subscribe
*
* @note This function can subscribe to one or more topics
*
* @param[in]   lhs   Left hand side in concatenation
* @param[in]   rhs   Right hand side in concatenation
*/
mqttnox_rc_t mqttnox_subscribe(mqttnox_client_t * c,
                               mqttnox_topic_sub_t * topics,
                               uint8_t topic_cnt)
{
    mqttnox_rc_t rc = MQTTNOX_RC_ERROR;
    mqttnox_hdr_t hdr;
    mqttnox_connect_var_hdr_t var_hdr;
    uint16_t pkt_len = 0;
    size_t i = 0;
    uint8_t remain_bytes = 0;
    int irc;

    /* We start at an offset because length is determined later.
       Leave enough space for max remaining length bytes and header */
    uint16_t offset = MAX_REMAIN_LEN_BYTES + sizeof(hdr);

    do
    {
        if (c->flag_initialized != MQTTNOX_INIT_FLAG) {
            rc = MQTTNOX_RC_ERROR_NOT_INIT;
            break;
        }

        MEMZERO_S(hdr);
        MEMZERO_S(var_hdr);
        MEMZERO(mqttnox_tx_buf);

        /* Initialize fixed header */
        hdr.type = MQTTNOX_CTRL_PKT_TYPE_SUBSCRIBE;
 
        /* Add packet identifier */
        mqttnox_tx_buf[pkt_len + offset] = MSB(c->packet_ident);
        pkt_len++;
        mqttnox_tx_buf[pkt_len + offset] = LSB(c->packet_ident);
        pkt_len++;
        c->packet_ident++;

        for (i = 0; i < topic_cnt; i++) {
            irc = mqttnox_append_utf8_string(&mqttnox_tx_buf[pkt_len + offset], topics[i].topic, 1);
            if (irc > 0) {
                pkt_len += irc;
            }

            /* Add qos */
            mqttnox_tx_buf[pkt_len + offset] = topics[i].qos & 0x03;
            pkt_len++;
        }

        /* Set length backwards behind the data */
        remain_bytes = mqttnox_set_remain_len(&mqttnox_tx_buf[sizeof(hdr)], pkt_len);
        pkt_len += remain_bytes;

        /* Copy Fixed Header */
        memcpy(&mqttnox_tx_buf[offset - remain_bytes - 1], (void*)&hdr, sizeof(hdr));
        mqttnox_tx_buf[offset - remain_bytes - 1] |= 0x2; /* Required */
        pkt_len += sizeof(hdr);

        /* Send the connect packet, response is received async */
        irc = mqttnox_tcp_send(&mqttnox_tx_buf[offset - remain_bytes - 1], pkt_len);


        rc = MQTTNOX_SUCCESS;
    } while (0);

    return rc;
}

static int mqttnox_set_remain_len(uint8_t * buffer, uint32_t len)
{        
    if (len <= 127) {
        buffer[3] = (len & 0x0000007F);
        return 1;
    }
    else if (len <= 16383) {
        buffer[2] = (len & 0x0000007F) | (1 << 7);
        buffer[3] = (len & 0x00007FF0) >> 7;
        return 2;
    }
    else if (len <= 2097151) {
        buffer[1] = (len & 0x0000007F) | (1 << 7);
        buffer[2] = ((len & 0x00007FF0) >> 7) | (1 << 7);
        buffer[3] = (len & 0x007FF000) >> 14;
        return 3;
    }
    else if (len <= 268435455) {
        buffer[0] = (len & 0x0000007F) | (1 << 7);
        buffer[1] = ((len & 0x00007FF0) >> 7) | (1 << 7);
        buffer[2] = ((len & 0x007FF000) >> 14) | (1 << 7);
        buffer[3] = ((len & 0x7FF00000) >> 21);
        return 4;
    }

    return 0;
}
static int mqttnox_decode_remain_len(uint8_t* buffer, uint32_t * len)
{
    uint8_t byte;
    uint32_t multiplier = 1;
    uint32_t value = 0;
    size_t index = 0;
    uint8_t byte_cnt = 0;
    do
    {
        byte = buffer[index];
        value += (byte & 127) * multiplier;
        multiplier *= 128;
        if (multiplier > 128 * 128 * 128) {
            return -1;
        }

        byte_cnt++;

    } while ((byte & 128) != 0);

    if (len != NULL) {
        *len = value;
    }   

    return byte_cnt;
}


/**@brief MQTT Unsubscribe
*
* @note This macro will delay concatenation until the expressions have been resolved
*
* @param[in]   lhs   Left hand side in concatenation
* @param[in]   rhs   Right hand side in concatenation
*/
mqttnox_rc_t mqttnox_unsubscribe(mqttnox_client_t* c,
                               mqttnox_topic_sub_t* topics,
                               uint8_t topic_cnt)
{
    mqttnox_rc_t rc = MQTTNOX_RC_ERROR;
    mqttnox_hdr_t hdr;
    mqttnox_connect_var_hdr_t var_hdr;
    uint16_t pkt_len = 0;
    size_t i = 0;
    int irc;

    do
    {
        if (c->flag_initialized != MQTTNOX_INIT_FLAG) {
            rc = MQTTNOX_RC_ERROR_NOT_INIT;
            break;
        }

        MEMZERO_S(hdr);
        MEMZERO_S(var_hdr);

        /* Initialize fixed header */
        hdr.type = MQTTNOX_CTRL_PKT_TYPE_UNSUBSCRIBE;


        MEMZERO(mqttnox_tx_buf);

        /* Copy Fixed Header */
        memcpy(mqttnox_tx_buf, (void*)&hdr, sizeof(hdr));
        mqttnox_tx_buf[0] |= 0x2; /* Required */
        pkt_len += sizeof(hdr);

        /* Skip Remaining length for now - updated later */
        pkt_len += 1;

        /* Add packet identifier */
        mqttnox_tx_buf[pkt_len++] = MSB(c->packet_ident);
        mqttnox_tx_buf[pkt_len++] = LSB(c->packet_ident);
        c->packet_ident++;

        for (i = 0; i < topic_cnt; i++) {
            irc = mqttnox_append_utf8_string(&mqttnox_tx_buf[pkt_len], topics[i].topic, 1);
            if (irc > 0) {
                pkt_len += irc;
            }
        }

        mqttnox_tx_buf[MQTT_LENGTH_FIELD_OFFSET] = pkt_len - 2;

        /* Send the connect packet, response is received async */
        irc = mqttnox_tcp_send(mqttnox_tx_buf, pkt_len);


        rc = MQTTNOX_SUCCESS;
    } while (0);

    return rc;
}

/**@brief MQTT PubAck
*
* @note This function sends Pub Ack
*
* @param[in]   lhs   Left hand side in concatenation
* @param[in]   rhs   Right hand side in concatenation
*/
static mqttnox_rc_t mqttnox_puback(mqttnox_client_t* c,
                            uint16_t identifier)
{
    mqttnox_rc_t rc = MQTTNOX_RC_ERROR;
    mqttnox_hdr_t hdr;
    mqttnox_connect_var_hdr_t var_hdr;
    uint16_t pkt_len = 0;    
    uint8_t remain_bytes = 0;
    int irc;

    /* We start at an offset because length is determined later.
       Leave enough space for max remaining length bytes and header */
    uint16_t offset = MAX_REMAIN_LEN_BYTES + sizeof(hdr);

    do
    {
        if (c->flag_initialized != MQTTNOX_INIT_FLAG) {
            rc = MQTTNOX_RC_ERROR_NOT_INIT;
            break;
        }

        MEMZERO_S(hdr);
        MEMZERO_S(var_hdr);
        MEMZERO(mqttnox_tx_buf);

        /* Initialize fixed header */
        hdr.type = MQTTNOX_CTRL_PKT_TYPE_PUBACK;

        /* Add packet identifier */
        mqttnox_tx_buf[pkt_len + offset] = MSB(identifier);
        pkt_len++;
        mqttnox_tx_buf[pkt_len + offset] = LSB(identifier);
        pkt_len++;

        /* Set length backwards behind the data */
        remain_bytes = mqttnox_set_remain_len(&mqttnox_tx_buf[sizeof(hdr)], pkt_len);
        pkt_len += remain_bytes;

        /* Copy Fixed Header */
        memcpy(&mqttnox_tx_buf[offset - remain_bytes - 1], (void*)&hdr, sizeof(hdr));        
        pkt_len += sizeof(hdr);

        /* Send the connect packet, response is received async */
        irc = mqttnox_tcp_send(&mqttnox_tx_buf[offset - remain_bytes - 1], pkt_len);


        rc = MQTTNOX_SUCCESS;
    } while (0);

    return rc;
}

/**@brief MQTT PubRec
*
* @note This function sends Pub Rec
*
* @param[in]   c   Left hand side in concatenation
* @param[in]   rhs   Right hand side in concatenation
*/
static mqttnox_rc_t mqttnox_pubrec(mqttnox_client_t* c, uint16_t identifier)
{
    mqttnox_rc_t rc = MQTTNOX_RC_ERROR;
    mqttnox_hdr_t hdr;
    mqttnox_connect_var_hdr_t var_hdr;
    uint16_t pkt_len = 0;
    size_t i = 0;
    uint8_t remain_bytes = 0;
    int irc;

    mqttnox_debug_printf(c, MQTTNOX_DEBUG_LVL_DEBUG, "Sending Pub Rec\n");

    /* We start at an offset because length is determined later.
       Leave enough space for max remaining length bytes and header */
    uint16_t offset = MAX_REMAIN_LEN_BYTES + sizeof(hdr);

    do
    {
        if (c->flag_initialized != MQTTNOX_INIT_FLAG) {
            rc = MQTTNOX_RC_ERROR_NOT_INIT;
            break;
        }

        MEMZERO_S(hdr);
        MEMZERO_S(var_hdr);
        MEMZERO(mqttnox_tx_buf);

        /* Initialize fixed header */
        hdr.type = MQTTNOX_CTRL_PKT_TYPE_PUBREC;

        /* Add packet identifier */
        mqttnox_tx_buf[pkt_len + offset] = MSB(identifier);
        pkt_len++;
        mqttnox_tx_buf[pkt_len + offset] = LSB(identifier);
        pkt_len++;

        /* Set length backwards behind the data */
        remain_bytes = mqttnox_set_remain_len(&mqttnox_tx_buf[sizeof(hdr)], pkt_len);
        pkt_len += remain_bytes;

        /* Copy Fixed Header */
        memcpy(&mqttnox_tx_buf[offset - remain_bytes - 1], (void*)&hdr, sizeof(hdr));        
        pkt_len += sizeof(hdr);

        /* Send the connect packet, response is received async */
        irc = mqttnox_tcp_send(&mqttnox_tx_buf[offset - remain_bytes - 1], pkt_len);

        rc = MQTTNOX_SUCCESS;
    } while (0);

    return rc;
}

/**@brief MQTT PubRel
*
* @note This function sends Pub Rec
*
* @param[in]   c   Left hand side in concatenation
* @param[in]   rhs   Right hand side in concatenation
*/
static mqttnox_rc_t mqttnox_pubrel(mqttnox_client_t* c, uint16_t identifier)
{
    mqttnox_rc_t rc = MQTTNOX_RC_ERROR;
    mqttnox_hdr_t hdr;
    mqttnox_connect_var_hdr_t var_hdr;
    uint16_t pkt_len = 0;
    size_t i = 0;
    uint8_t remain_bytes = 0;
    int irc;

    mqttnox_debug_printf(c, MQTTNOX_DEBUG_LVL_DEBUG, "Sending Pub Rel\n");

    /* We start at an offset because length is determined later.
       Leave enough space for max remaining length bytes and header */
    uint16_t offset = MAX_REMAIN_LEN_BYTES + sizeof(hdr);

    do
    {
        if (c->flag_initialized != MQTTNOX_INIT_FLAG) {
            rc = MQTTNOX_RC_ERROR_NOT_INIT;
            break;
        }

        MEMZERO_S(hdr);
        MEMZERO_S(var_hdr);
        MEMZERO(mqttnox_tx_buf);

        /* Initialize fixed header */
        hdr.type = MQTTNOX_CTRL_PKT_TYPE_PUBREL;

        /* Add packet identifier */
        mqttnox_tx_buf[pkt_len + offset] = LSB(identifier);
        pkt_len++;
        mqttnox_tx_buf[pkt_len + offset] = MSB(identifier);
        pkt_len++;

        /* Set length backwards behind the data */
        remain_bytes = mqttnox_set_remain_len(&mqttnox_tx_buf[sizeof(hdr)], pkt_len);
        pkt_len += remain_bytes;

        /* Copy Fixed Header */
        memcpy(&mqttnox_tx_buf[offset - remain_bytes - 1], (void*)&hdr, sizeof(hdr));
        mqttnox_tx_buf[offset - remain_bytes - 1] |= 0x2; /* Required */
        pkt_len += sizeof(hdr);

        /* Send the connect packet, response is received async */
        irc = mqttnox_tcp_send(&mqttnox_tx_buf[offset - remain_bytes - 1], pkt_len);

        rc = MQTTNOX_SUCCESS;
    } while (0);

    return rc;
}

/**@brief MQTT PubComp
*
* @note This function sends Pub Rec
*
* @param[in]   c   Left hand side in concatenation
* @param[in]   rhs   Right hand side in concatenation
*/
static mqttnox_rc_t mqttnox_pubcomp(mqttnox_client_t* c, uint16_t identifier)
{
    mqttnox_rc_t rc = MQTTNOX_RC_ERROR;
    mqttnox_hdr_t hdr;
    mqttnox_connect_var_hdr_t var_hdr;
    uint16_t pkt_len = 0;
    size_t i = 0;
    uint8_t remain_bytes = 0;
    int irc;

    mqttnox_debug_printf(c, MQTTNOX_DEBUG_LVL_DEBUG, "Sending Pub Comp\n");

    /* We start at an offset because length is determined later.
       Leave enough space for max remaining length bytes and header */
    uint16_t offset = MAX_REMAIN_LEN_BYTES + sizeof(hdr);

    do
    {
        if (c->flag_initialized != MQTTNOX_INIT_FLAG) {
            rc = MQTTNOX_RC_ERROR_NOT_INIT;
            break;
        }

        MEMZERO_S(hdr);
        MEMZERO_S(var_hdr);
        MEMZERO(mqttnox_tx_buf);

        /* Initialize fixed header */
        hdr.type = MQTTNOX_CTRL_PKT_TYPE_PUBCOMP;

        /* Add packet identifier */
        mqttnox_tx_buf[pkt_len + offset] = LSB(identifier);
        pkt_len++;
        mqttnox_tx_buf[pkt_len + offset] = MSB(identifier);
        pkt_len++;

        /* Set length backwards behind the data */
        remain_bytes = mqttnox_set_remain_len(&mqttnox_tx_buf[sizeof(hdr)], pkt_len);
        pkt_len += remain_bytes;

        /* Copy Fixed Header */
        memcpy(&mqttnox_tx_buf[offset - remain_bytes - 1], (void*)&hdr, sizeof(hdr));        
        pkt_len += sizeof(hdr);

        /* Send the connect packet, response is received async */
        irc = mqttnox_tcp_send(&mqttnox_tx_buf[offset - remain_bytes - 1], pkt_len);

        rc = MQTTNOX_SUCCESS;
    } while (0);

    return rc;
}

/**@brief MQTT Disconnect
*
* @note This function initiates a clean disconnection from the MQTT
*
* @param[in]   lhs   Left hand side in concatenation
* @param[in]   rhs   Right hand side in concatenation
*/
mqttnox_rc_t mqttnox_disconnect(mqttnox_client_t * c)
{
    mqttnox_rc_t rc = MQTTNOX_RC_ERROR;
    mqttnox_hdr_t hdr;
    mqttnox_connect_var_hdr_t var_hdr;
    uint16_t pkt_len = 0;
    int irc;

    do
    {
        if (c->flag_initialized != MQTTNOX_INIT_FLAG) {
            rc = MQTTNOX_RC_ERROR_NOT_INIT;
            break;
        }

        MEMZERO_S(hdr);
        MEMZERO_S(var_hdr);

        /* Initialize fixed header */
        hdr.type = MQTTNOX_CTRL_PKT_TYPE_DISCONNECT;


        MEMZERO(mqttnox_tx_buf);

        /* Copy Fixed Header */
        memcpy(mqttnox_tx_buf, (void*)&hdr, sizeof(hdr));        
        pkt_len += sizeof(hdr);

        /* Zero remaining length */
        mqttnox_tx_buf[pkt_len++] = 0;

        /* Send the connect packet, response is received async */
        irc = mqttnox_tcp_send(mqttnox_tx_buf, pkt_len);
        if(irc != 0) {
            break;
        }

        /* Disconnect the TCP */
        mqttnox_tcp_disconnect();

        c->status.connected = 0;

        rc = MQTTNOX_SUCCESS;
    } while (0);

    return rc;
}

/**@brief Appends UTF8 strings to buffer
*
* @note Internal function
*
* @param[in]   buffer    the buffer to which append the string
* @param[in]   str    the string to append
* @param[in]   add_len  indicates whether length should be prepended to string
*
* @return      length of string appended, or -1 if failed
*/
static int mqttnox_append_utf8_string(uint8_t* buffer, const char* str, uint8_t add_len)
{
    uint16_t len = 0;
    size_t str_len = 0;
    size_t buf_offset = 0;

    if (buffer != NULL) {
        buf_offset = strlen((const char *)buffer);
    }

    if (str != NULL && (strlen(str) > 0) && buffer != NULL) {

        str_len = strlen(str);

        if (add_len) {
            buffer[buf_offset + len] = ((uint16_t)str_len & 0xFF00) >> 8;
            len++;
            buffer[buf_offset + len] = ((uint16_t)str_len & 0x00FF);
            len++;
        }

        memcpy(&buffer[buf_offset + len], (void*)str, str_len);

        return (uint16_t)str_len + len;
    }

    return -1;
}

/**@brief MQTT check if connected
*
* @note Checks whether the MQTT is currently connected
*
* @param[in]   c    mqttnox object \see mqttnox_client_t
* @param[in]   data pointer to buffer with the incoming data from the server
* @param[in]   len  length of the data from the server
*
* @return     None
*/
uint8_t mqttnox_is_connected(mqttnox_client_t* c)
{
    if (c->status.connected) {
        return 1;
    }
    else
    {
        return 0;
    }
}

#ifdef __cplusplus
}
#endif
