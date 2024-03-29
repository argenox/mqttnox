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
* Summary: MQTT Nox Internal
*
*/

#include <string.h>

#include "mqttnoxlib.h"

/**@brief Validates Device ID
*
* @note MQTT has specific requirements for the Device ID and this function
*       performs 
*
* @param[in]   str   Null terminated Device ID string 
*
* @return     0 if device ID valid, negative error otherwise
*/
int mqttnoxlib_validate_device_id(const char * str)
{
	int i = 0;

	if (str == NULL || strlen(str) >= MQTT_MAX_DEVICE_ID_LEN) {
		return -1;
	}

	for (i = 0; i < strlen(str); i++)
	{
		/* [MQTT-3.1.3-5] */
		if (!((str[i] >= '0' && str[i] <= '9') ||
			  (str[i] >= 'A' && str[i] <= 'Z') ||
			  (str[i] >= 'a' && str[i] <= 'z')))
		{
			return -1;
		}
	}

	return 0;
}
