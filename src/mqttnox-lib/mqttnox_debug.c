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

#ifdef __cplusplus
extern "C" {
#endif

/* System Includes */
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

/* Library Includes */
#include "mqttnox.h"
#include "mqttnoxlib.h"
#include "mqttnox_err.h"
#include "common.h"
#include "mqttnox_tal.h"
#include "mqttnox_debug.h"


/* MQTT Packet Types */
static const item_desc_t mqtt_packet_type[] =
{
    {"MQTT_RESERVED", MQTTNOX_CTRL_PKT_TYPE_RESERVED},
    {"MQTT_CONNECT", MQTTNOX_CTRL_PKT_TYPE_CONNECT},
    {"MQTT_CONNACK", MQTTNOX_CTRL_PKT_TYPE_CONNACK},
    {"MQTT_PUBLISH", MQTTNOX_CTRL_PKT_TYPE_PUBLISH},
    {"MQTT_PUBACK", MQTTNOX_CTRL_PKT_TYPE_PUBACK},
    {"MQTT_PUBREC", MQTTNOX_CTRL_PKT_TYPE_PUBREC},
    {"MQTT_PUBREL", MQTTNOX_CTRL_PKT_TYPE_PUBREL},
    {"MQTT_PUBCOMP", MQTTNOX_CTRL_PKT_TYPE_PUBCOMP},
    {"MQTT_SUBSCRIBE", MQTTNOX_CTRL_PKT_TYPE_SUBSCRIBE},
    {"MQTT_SUBACK", MQTTNOX_CTRL_PKT_TYPE_SUBACK},
    {"MQTT_UNSUBSCRIBE", MQTTNOX_CTRL_PKT_TYPE_UNSUBSCRIBE},
    {"MQTT_UNSUBACK", MQTTNOX_CTRL_PKT_TYPE_UNSUBACK},
    {"MQTT_PINGREQ", MQTTNOX_CTRL_PKT_TYPE_PINGREQ},
    {"MQTT_PINGRESP", MQTTNOX_CTRL_PKT_TYPE_PINGRESP},
    {"MQTT_DISCONNECT", MQTTNOX_CTRL_PKT_TYPE_DISCONNECT},
};


/**@brief Get string for packet type
* 
* @note this function is a helper function to allow us to print MQTT Packet type
*
*
* @param[in]   code is the packet ID
*
*/
char * get_mqtt_packet_type_str(int32_t code)
{
    int i = 0;
    for(i = 0; i < ARRAY_LEN(mqtt_packet_type); i++)
    {
        if(mqtt_packet_type[i].code == code){
            return (char *)mqtt_packet_type[i].name;
        }
    }

    return "";
}

/**@brief Print data buffer
*
*
* @param[in]   data   is the data to print
* @param[in]   format len are the number of bytes to print
*
*/
void print_buffer(uint8_t* data, uint16_t len)
{    
    size_t i;
    printf("-----Data[%d]: ", len);
    for (i = 0; i < len; i++)
    {
        printf("%02X ", data[i]);
    }
    printf("\n");
}

/**@brief MQTTNox Debug printf
*
*
* @param[in]   lvl    error level for the print
* @param[in]   format c string containing format specifier 
* @param[in]   ...    additional arguments
*
*/
void mqttnox_debug_printf(mqttnox_debug_lvl_t lvl, const char * format, ...)
{
    char buffer[256];

    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);    
    va_end(args);

    /* Send to system print */    
    mqttnox_hal_debug_printf(buffer);
}

#ifdef __cplusplus
}
#endif