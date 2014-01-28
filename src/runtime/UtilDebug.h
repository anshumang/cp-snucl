/*****************************************************************************/
/*                                                                           */
/* Copyright (c) 2011-2012 Seoul National University.                        */
/* All rights reserved.                                                      */
/*                                                                           */
/* Redistribution and use in source and binary forms, with or without        */
/* modification, are permitted provided that the following conditions        */
/* are met:                                                                  */
/*   1. Redistributions of source code must retain the above copyright       */
/*      notice, this list of conditions and the following disclaimer.        */
/*   2. Redistributions in binary form must reproduce the above copyright    */
/*      notice, this list of conditions and the following disclaimer in the  */
/*      documentation and/or other materials provided with the distribution. */
/*   3. Neither the name of Seoul National University nor the names of its   */
/*      contributors may be used to endorse or promote products derived      */
/*      from this software without specific prior written permission.        */
/*                                                                           */
/* THIS SOFTWARE IS PROVIDED BY SEOUL NATIONAL UNIVERSITY "AS IS" AND ANY    */
/* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED */
/* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE    */
/* DISCLAIMED. IN NO EVENT SHALL SEOUL NATIONAL UNIVERSITY BE LIABLE FOR ANY */
/* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL        */
/* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS   */
/* OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)     */
/* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,       */
/* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN  */
/* ANY WAY OUT OF THE USE OF THIS  SOFTWARE, EVEN IF ADVISED OF THE          */
/* POSSIBILITY OF SUCH DAMAGE.                                               */
/*                                                                           */
/* Contact information:                                                      */
/*   Center for Manycore Programming                                         */
/*   School of Computer Science and Engineering                              */
/*   Seoul National University, Seoul 151-744, Korea                         */
/*   http://aces.snu.ac.kr                                                   */
/*                                                                           */
/* Contributors:                                                             */
/*   Jungwon Kim, Sangmin Seo, Jun Lee, Jeongho Nah, Gangwon Jo, Jaejin Lee  */
/*                                                                           */
/*****************************************************************************/

#ifndef __UTILDEBUG_H
#define __UTILDEBUG_H

#include <stdio.h>
#include <stdlib.h>

#define SNUCL_ENABLE_FATAL
#define SNUCL_ENABLE_ERROR
//#define SNUCL_ENABLE_INFO
//#define SNUCL_ENABLE_DEBUG
//#define SNUCL_ENABLE_CHECK
#define SNUCL_ENABLE_SEPARATOR
#define SNUCL_ENABLE_ERRORCHECK

#define SNUCL_DEBUG_HEADER(x)  static const char __HEADER__[] = x

#ifndef SNUCL_ENABLE_FATAL
#define SNUCL_FATAL(fmt, ...)
#else
#define SNUCL_FATAL(fmt, ...)  fprintf(stdout, "* FTL * [%s:%d] " fmt "\n", __HEADER__, __LINE__, __VA_ARGS__)
#endif

#ifndef SNUCL_ENABLE_ERROR
#define SNUCL_ERROR(fmt, ...)
#else
#define SNUCL_ERROR(fmt, ...)  fprintf(stdout, "* ERR * [%s:%d] " fmt "\n", __HEADER__, __LINE__, __VA_ARGS__)
#endif

#ifndef SNUCL_ENABLE_INFO
#define SNUCL_INFO(fmt, ...)
#else
#define SNUCL_INFO(fmt, ...)   fprintf(stdout, "* INF * [%s:%d] " fmt "\n", __HEADER__, __LINE__, __VA_ARGS__)
#endif

#ifndef SNUCL_ENABLE_DEBUG
#define SNUCL_DEBUG(fmt, ...)
#else
#define SNUCL_DEBUG(fmt, ...)  fprintf(stdout, "* DBG * [%s:%d] " fmt "\n", __HEADER__, __LINE__, __VA_ARGS__)
#endif

#ifndef SNUCL_ENABLE_CHECK
#define SNUCL_CHECK()
#else
#define SNUCL_CHECK()          fprintf(stdout, "* CHK * [%s:%d] %s()\n", __HEADER__, __LINE__, __FUNCTION__)
#endif

#ifndef SNUCL_ENABLE_SEPARATOR
#define SNUCL_SEPARATOR()
#else
#define SNUCL_SEPARATOR()      fprintf(stdout, "* SPR * [%s:%d] ###################################################################\n", __HEADER__, __LINE__);
#endif

#ifndef SNUCL_ENABLE_ERRORCHECK
#define SNUCL_ECHECK(ERR)
#else
#define SNUCL_ECHECK(ERR)      if (ERR != CL_SUCCESS) fprintf(stdout, "* ECK * [%s:%d] ERROR[%d]\n", __HEADER__, __LINE__, ERR);
#endif

#endif
