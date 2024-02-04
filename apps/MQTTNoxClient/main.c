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
* File:    main.c
* Summary: MQTTNox Client Application
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mqttnox.h"
#include "mqttnox_tal.h"

mqttnox_client_t client = { 0 };
mqttnox_client_conf_t client_conf = { 0 };

mqttnox_topic_sub_t topics_sub[] =
{
	{"/topic/device/aquairepeater/out", MQTTNOX_QOS2_EXACTLY_ONCE_DELIV},
	{"/topic/device/aquaihealthcheck/out", MQTTNOX_QOS2_EXACTLY_ONCE_DELIV},
	{"/topic/device/version/upgrade", MQTTNOX_QOS2_EXACTLY_ONCE_DELIV},
	{"/test/val", MQTTNOX_QOS2_EXACTLY_ONCE_DELIV},
	{"/test/val2", MQTTNOX_QOS2_EXACTLY_ONCE_DELIV},
};

/**@brief MQTTNox App Callback handler
*
* @note Callback used for async MQTT event handling. Note that this callback is called in the context
	   of the mqttnox thread, so care must be taken to avoid a stack overflow by either increasing
	   the mqttnox thread's stack, or by minimizing stack usage and passing event data to a task
	   for processing.
*
* @param[in]   evt   The event being handled, \see mqttnox_evt_id_t
* @param[in]   data  contains the data for the event, depending on evt type
*/
void mqttnox_callback(mqttnox_evt_data_t * data)
{
	switch (data->evt_id)
	{
		case MQTTNOX_EVT_CONNECT:
			printf("[App] MQTT Connected\n");
			
			
			/*mqttnox_publish(&client,
				MQTTNOX_QOS0_AT_MOST_ONCE_DELIV,
				0,
				0,
				"/test/val",
				"{\"val\": 2}");
				*/
				
			
			mqttnox_subscribe(&client, topics_sub, ARRAY_LEN(topics_sub));

			//mqttnox_unsubscribe(&client, topics_sub, ARRAY_LEN(topics_sub));
			
			break;
		case MQTTNOX_EVT_CONNECT_ERROR:
			printf("[App] Connection error with error %x\n", data->evt.conn_err_evt.reason);
			break;
		case MQTTNOX_EVT_PUBLISHED:
			printf("[App] MQTT Published\n");
			break;
		case MQTTNOX_EVT_SUBSCRIBED:
			printf("[App] MQTT Subscribed to Topic\n");
			break;
		case MQTTNOX_EVT_UNSUBSCRIBED:
			printf("[App] MQTT Unsubscribed to Topic\n");
			break;
		case MQTTNOX_EVT_PINGRESP:
			printf("[App] MQTT Ping Response\n");
			break;
		case MQTTNOX_EVT_PUBREL:
			break;
		case MQTTNOX_EVT_DISCONNECT:
			printf("[App] MQTT Disconnected\n");
			break;
		case MQTTNOX_EVT_ERROR:
			printf("[App] MQTT Error\n");
			break;
	}
}

int main(void)
{	
	printf("MQTTNox Version: %s\n", MQTTNOX_VERSION);

	client_conf.server.addr = "test.mosquitto.org";
	client_conf.server.port = 1883;
	client_conf.clean_session = 0;

	client_conf.client_identifier = "MAMA12354";

	client_conf.callback = mqttnox_callback;

	mqttnox_init(&client, MQTTNOX_DEBUG_LVL_ALL);
	mqttnox_connect(&client, &client_conf, 60);


	mqttnox_wait_thread();


	printf("MQTTNox Client");
}