#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mqttnox.h"
#include "mqttnox_tal.h"

mqttnox_client_t client = { 0 };
mqttnox_client_conf_t client_conf = { 0 };

mqttnox_topic_sub_t topics_sub[] =
{
	{"/test/val", },
	{"/test/val2", },
};

/** Callback used for async MQTT event handling. Note that this callback is called in the context
	   of the mqttnox thread, so care must be taken to avoid a stack overflow by either increasing
	   the mqttnox thread's stack, or by minimizing stack usage and passing event data to a task
	   for processing
*/

/**@brief MQTTNox Callback handler
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

			
			mqttnox_publish(&client,
				MQTTNOX_QOS0_AT_MOST_ONCE_DELIV,
				0,
				0,
				"/test/val",
				"{\"val\": 2}");
			

			//mqttnox_subscribe(&client, topics_sub, ARRAY_LEN(topics_sub));

			mqttnox_unsubscribe(&client, topics_sub, ARRAY_LEN(topics_sub));
			

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
	

	client_conf.server.addr = "test.mosquitto.org";
	client_conf.server.port = 1883;
	client_conf.clean_session = 0;

	client_conf.client_identifier = "MAMA12354";

	client_conf.callback = mqttnox_callback;

	mqttnox_init(&client);
	mqttnox_connect(&client, &client_conf);


	mqttnox_wait_thread();


	printf("MQTTNox Client");
}