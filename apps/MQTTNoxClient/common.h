/*****************************************************************************
* Copyright (c) [2023] Aquai
* All rights reserved.
*
* PROPRIETARY AND CONFIDENTIAL
*
* NOTICE:  All information contained herein, source code, binaries and
* derived works is, and remains
* the property of Aquai and its suppliers,
* if any.  The intellectual and technical concepts contained
* herein are proprietary to Aquai
* and its suppliers and may be covered by U.S. and Foreign Patents,
* patents in process, and are protected by trade secret or copyright law.
* Dissemination of this information or reproduction of this material
* is strictly forbidden unless prior written permission is obtained
* from Aquai.
*
* THIS SOFTWARE IS PROVIDED BY AQUAI "AS IS" AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL AQUAI BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
* CONTACT: info@aquai.com
*
*
* File:    common_defs.h
* Summary: Common Definitions
*
*/

#ifndef __COMMON_DEFS_H_
#define __COMMON_DEFS_H_

#include <stdint.h>
#include <string.h>

#define ARRAY_LEN(X)  (sizeof(X)  / sizeof(X[0]))
#define MEMZERO(X)     memset(X, 0, sizeof(X))
#define MEMZERO_S(X)     memset(&X, 0, sizeof(X))

#define ALLOC_BASIC_UNIT 256

#endif
