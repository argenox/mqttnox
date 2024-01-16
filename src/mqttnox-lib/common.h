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
* File:    common.h
* Summary: Common Library
*
*/

#ifndef _COMMON_H_
#define _COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

#define ARRAY_LEN(X)  (sizeof(X)  / sizeof(X[0]))
#define MEMZERO(X)     memset(X, 0, sizeof(X))
#define MEMZERO_S(X)   memset(&X, 0, sizeof(X))

    /* GNU GCC Packing */
#ifdef __GNUC__
#define PACK_STRUCT( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#endif

/* IAR Packing */
#ifdef ICCARM
#define PACK_STRUCT( __Declaration__ ) __Declaration__
#endif

/* Visual Studio Packing */
#ifdef _MSC_VER
#define PACK_STRUCT( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop))
#endif

#ifndef _COMPILER_PACK
#if defined(ICCARM) || defined(__GNUC__) || defined(__ICCARM__)
#define _COMPILER_PACK  __attribute__((packed))
#else 
#define _COMPILER_PACK
#endif
#endif


#define MSB(x) ((x & 0xFF00) >> 8)
#define LSB(x) (x & 0x00FF)

#ifdef __cplusplus
}
#endif

#endif