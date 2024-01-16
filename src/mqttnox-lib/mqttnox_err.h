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
* File:    mqttnox_err.h
* Summary: MQTT Nox External APIs
*
*/

#ifndef _MQTTNOX_ERR_H_
#define _MQTTNOX_ERR_H_

#ifdef __cplusplus
extern "C" {
#endif

#define ERROR_BASE 0x00000000


typedef enum 
{
    MQTTNOX_SUCCESS                   = ERROR_BASE,
    MQTTNOX_RC_ERROR                  = ERROR_BASE + 1,
    MQTTNOX_RC_ERROR_INTERNAL         = ERROR_BASE + 2,
    MQTTNOX_RC_ERROR_NOT_INIT         = ERROR_BASE + 3, /* Library object not initialized */
    MQTTNOX_RC_ERROR_BAD_CLIENT_IDENT = ERROR_BASE + 4, /* Device ID not specified specified or length / characters of ID wrong */    

} mqttnox_rc_t;



#ifdef __cplusplus
}
#endif

#endif