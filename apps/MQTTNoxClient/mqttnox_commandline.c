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
* File:    mqttnox_commandline.c
* Summary: MQTT Nox Internal
*
*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <process.h>
#include <windows.h>

#include "mqttnox.h"
#include "mqttnox_commandline.h"
#include "common.h"
#include "commandline.h"

static void help_cmd_handler(char * buf, uint16_t len);
static void pub_cmd_handler(char * buf, uint16_t len);
static void sub_cmd_handler(char * buf, uint16_t len);
static void exit_cmd_handler(char* buf, uint16_t len);

static void commandline_task(void* arg);

static char read_buffer[128];
static uint16_t read_cnt = 0;

static mqttnox_client_t * client = NULL;

uintptr_t commandline_thread;
HANDLE commandline_thread_obj;

const commandline_item_t commandline_cmds[] =
{
    {"help", help_cmd_handler},
	{"pub", pub_cmd_handler},
    {"sub", sub_cmd_handler},
	{"exit", exit_cmd_handler},
};

static char* string_find_end(char* str, char delim)
{
    char* end = NULL;
    size_t len = strlen(str);
    size_t i;
    for (i = 0; len; i++) {
        if (str[i] == delim || str[i] == '\n' || str[i] == '\r' || str[i] == 0x00) {
            end = &str[i];
            break;
        }
    }
    return end;
}

void mqttnox_commandline_init(mqttnox_client_t * c)
{
	if(c != NULL) {
		client = c;
	}

    /* Create Command Line thread */
    commandline_thread_obj = (HANDLE)_beginthread(&commandline_task, 0, NULL);
}

static void commandline_process_cmd(void)
{
    size_t i;

    /* Extract command */
    char * sub_cmd_start = string_find_end(read_buffer, ' ');
    *sub_cmd_start = 0;
    sub_cmd_start++;

    /* Extract subcommand */

    /* Extract parameter */

    for(i = 0; i < ARRAY_LEN(commandline_cmds); i++)
    {
        if((memcmp(read_buffer, commandline_cmds[i].name, strlen(commandline_cmds[i].name)) == 0) &&
                strlen(commandline_cmds[i].name) == strlen(read_buffer))
        {
            if(commandline_cmds[i].handler != 0) {
                commandline_cmds[i].handler(sub_cmd_start, read_cnt - 1); // need to fix this length
            }
        }
    }
}

static void help_cmd_handler(char * buf, uint16_t len)
{
    printf("Commands:\r\n");
    size_t i;
    for(i = 0; i < ARRAY_LEN(commandline_cmds); i++)
    {
        printf("\t%s\r\n", commandline_cmds[i].name);
    }
}

static void pub_cmd_handler(char * buf, uint16_t len)
{
    char * param_start = NULL;    
	char param_topic[256];
	char param_data[256];
	int param_qos;

    MEMZERO(param_topic);
    MEMZERO(param_data);
    param_qos = 0;

    if(strlen(buf) > 0) {
        param_start= string_find_end(buf, ' ');
        //*param_start = 0;
        param_start++;
    }

    if(param_start == NULL)
    {
        printf("Please parameters topic and QOS\r\n");
    }
    else
    {
		sscanf(buf, "%s %s %u", param_topic, param_data, &param_qos);

		mqttnox_publish(client,
				param_qos,
				0,
				0,
				param_topic,
				param_data);				
    }
}

static void sub_cmd_handler(char * buf, uint16_t len)
{
    char * param_start = NULL;    
    char param_topic[256];    
    int param_qos;

    if(strlen(buf) > 0) {
        param_start= string_find_end(buf, ' ');
        //*param_start = 0;
        param_start++;
    }

    if(param_start == NULL)
    {
        printf("Please parameters topic and QOS\r\n");
    }
    else
    {
        sscanf(buf, "%s %u", param_topic, &param_qos);

		mqttnox_topic_sub_t topics_sub[] =
		{
			{param_topic, param_qos},
		};

		mqttnox_subscribe(client, topics_sub, ARRAY_LEN(topics_sub));
    }
}

static void exit_cmd_handler(char* buf, uint16_t len)
{

}


static void commandline_task(void* arg)
{
    int32_t ret = 0;
    MEMZERO(read_buffer);
    read_cnt = 0;


    printf("\r\n>");

    while (1)
    {
        ret = read(0, &read_buffer[read_cnt++], sizeof(read_buffer));

        if (ret > 0) {
            read_cnt = ret;

            if (read_cnt >= sizeof(read_buffer)) {
                read_cnt = 0;
            }


            if (ret > 0)
            {
                //printf("%c", read_buffer[read_cnt - 1]);
                if (read_buffer[read_cnt - 1] == '\n' || read_buffer[read_cnt - 1] == '\r')
                {
                    read_buffer[read_cnt - 1] = 0;
                    //printf("\r\n");
                    if (strlen(read_buffer) > 0) {
                        commandline_process_cmd();
                    }

                    printf("\r\n>");
                    MEMZERO(read_buffer);
                    read_cnt = 0;
                }
                else if (read_buffer[read_cnt - 1] == '\b')
                {
                    read_cnt--;
                    read_buffer[read_cnt] = 0;
                    printf("%c", read_buffer[0]);
                }
            }
        }
    }
}