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

#ifndef __CPU_KERNEL_CTRL_H__
#define __CPU_KERNEL_CTRL_H__

#define USE_TLB


/////////////////////////////////////////////////////////////////////////////
#ifdef USE_TLB
/////////////////////////////////////////////////////////////////////////////
#define TLB_STR_DATA_START        typedef struct {
#define TLB_STR_DATA_END          } tlb_data;
#define TLB_GET_KEY               \
  tlb_data *tlb = (tlb_data *)pthread_getspecific(key_data);
#define TLB_GET(X)                (tlb->X)


/////////////////////////////////////////////////////////////////////////////
#else //USE_TLB
#ifdef USE_TLS
/////////////////////////////////////////////////////////////////////////////
#define TLB_STR_DATA_START  
#define TLB_STR_DATA_END    
#define TLB_GET_KEY  
#define TLB_GET(X)                (X)


/////////////////////////////////////////////////////////////////////////////
#else //USE_TLS - TLB and TLS are not used.
/////////////////////////////////////////////////////////////////////////////
#define TLB_STR_DATA_START  
#define TLB_STR_DATA_END    
#define TLB_GET_KEY  
#define TLB_GET(X)                (X)

#endif //USE_TLS
#endif //USE_TLB


/***************************************************************************/
/* Qualifiers                                                              */
/***************************************************************************/
#define __kernel
#define __global
#define __local
#define __constant
#define __private
#define __read_only
#define __write_only
#define __read_write


/***************************************************************************/
/* Types                                                                   */
/***************************************************************************/
#define CLK_LOCAL_MEM_FENCE     (1 << 0)
#define CLK_GLOBAL_MEM_FENCE    (1 << 1)
typedef int cl_mem_fence_flags;


#endif //__CPU_KERNEL_CTRL_H__
