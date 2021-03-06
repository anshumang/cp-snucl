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
/*   Sangmin Seo, Jungwon Kim, Jun Lee, Jeongho Nah, Gangwon Jo,             */
/*   and Jaejin Lee                                                          */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/* This file is based on the SNU-SAMSUNG OpenCL Compiler and is distributed  */
/* under the same license, GNU General Public License.                       */
/* See LICENSE.SNU-SAMSUNG_OpenCL_C_Compiler.TXT for details.                */
/*****************************************************************************/

#ifndef __CL_BUILTINS_CONVERT_DOUBLE4_H__
#define __CL_BUILTINS_CONVERT_DOUBLE4_H__

#include "cl_types.h"

double4 convert_double4(char4) __attribute__((overloadable));
double4 convert_double4(uchar4) __attribute__((overloadable));
double4 convert_double4(short4) __attribute__((overloadable));
double4 convert_double4(ushort4) __attribute__((overloadable));
double4 convert_double4(int4) __attribute__((overloadable));
double4 convert_double4(uint4) __attribute__((overloadable));
double4 convert_double4(long4) __attribute__((overloadable));
double4 convert_double4(ulong4) __attribute__((overloadable));
double4 convert_double4(float4) __attribute__((overloadable));
double4 convert_double4(double4) __attribute__((overloadable));
double4 convert_double4_rte(char4) __attribute__((overloadable));
double4 convert_double4_rte(uchar4) __attribute__((overloadable));
double4 convert_double4_rte(short4) __attribute__((overloadable));
double4 convert_double4_rte(ushort4) __attribute__((overloadable));
double4 convert_double4_rte(int4) __attribute__((overloadable));
double4 convert_double4_rte(uint4) __attribute__((overloadable));
double4 convert_double4_rte(long4) __attribute__((overloadable));
double4 convert_double4_rte(ulong4) __attribute__((overloadable));
double4 convert_double4_rte(float4) __attribute__((overloadable));
double4 convert_double4_rte(double4) __attribute__((overloadable));
double4 convert_double4_rtz(char4) __attribute__((overloadable));
double4 convert_double4_rtz(uchar4) __attribute__((overloadable));
double4 convert_double4_rtz(short4) __attribute__((overloadable));
double4 convert_double4_rtz(ushort4) __attribute__((overloadable));
double4 convert_double4_rtz(int4) __attribute__((overloadable));
double4 convert_double4_rtz(uint4) __attribute__((overloadable));
double4 convert_double4_rtz(long4) __attribute__((overloadable));
double4 convert_double4_rtz(ulong4) __attribute__((overloadable));
double4 convert_double4_rtz(float4) __attribute__((overloadable));
double4 convert_double4_rtz(double4) __attribute__((overloadable));
double4 convert_double4_rtp(char4) __attribute__((overloadable));
double4 convert_double4_rtp(uchar4) __attribute__((overloadable));
double4 convert_double4_rtp(short4) __attribute__((overloadable));
double4 convert_double4_rtp(ushort4) __attribute__((overloadable));
double4 convert_double4_rtp(int4) __attribute__((overloadable));
double4 convert_double4_rtp(uint4) __attribute__((overloadable));
double4 convert_double4_rtp(long4) __attribute__((overloadable));
double4 convert_double4_rtp(ulong4) __attribute__((overloadable));
double4 convert_double4_rtp(float4) __attribute__((overloadable));
double4 convert_double4_rtp(double4) __attribute__((overloadable));
double4 convert_double4_rtn(char4) __attribute__((overloadable));
double4 convert_double4_rtn(uchar4) __attribute__((overloadable));
double4 convert_double4_rtn(short4) __attribute__((overloadable));
double4 convert_double4_rtn(ushort4) __attribute__((overloadable));
double4 convert_double4_rtn(int4) __attribute__((overloadable));
double4 convert_double4_rtn(uint4) __attribute__((overloadable));
double4 convert_double4_rtn(long4) __attribute__((overloadable));
double4 convert_double4_rtn(ulong4) __attribute__((overloadable));
double4 convert_double4_rtn(float4) __attribute__((overloadable));
double4 convert_double4_rtn(double4) __attribute__((overloadable));
double4 convert_double4_sat(char4) __attribute__((overloadable));
double4 convert_double4_sat(uchar4) __attribute__((overloadable));
double4 convert_double4_sat(short4) __attribute__((overloadable));
double4 convert_double4_sat(ushort4) __attribute__((overloadable));
double4 convert_double4_sat(int4) __attribute__((overloadable));
double4 convert_double4_sat(uint4) __attribute__((overloadable));
double4 convert_double4_sat(long4) __attribute__((overloadable));
double4 convert_double4_sat(ulong4) __attribute__((overloadable));
double4 convert_double4_sat(float4) __attribute__((overloadable));
double4 convert_double4_sat(double4) __attribute__((overloadable));
double4 convert_double4_sat_rte(char4) __attribute__((overloadable));
double4 convert_double4_sat_rte(uchar4) __attribute__((overloadable));
double4 convert_double4_sat_rte(short4) __attribute__((overloadable));
double4 convert_double4_sat_rte(ushort4) __attribute__((overloadable));
double4 convert_double4_sat_rte(int4) __attribute__((overloadable));
double4 convert_double4_sat_rte(uint4) __attribute__((overloadable));
double4 convert_double4_sat_rte(long4) __attribute__((overloadable));
double4 convert_double4_sat_rte(ulong4) __attribute__((overloadable));
double4 convert_double4_sat_rte(float4) __attribute__((overloadable));
double4 convert_double4_sat_rte(double4) __attribute__((overloadable));
double4 convert_double4_sat_rtz(char4) __attribute__((overloadable));
double4 convert_double4_sat_rtz(uchar4) __attribute__((overloadable));
double4 convert_double4_sat_rtz(short4) __attribute__((overloadable));
double4 convert_double4_sat_rtz(ushort4) __attribute__((overloadable));
double4 convert_double4_sat_rtz(int4) __attribute__((overloadable));
double4 convert_double4_sat_rtz(uint4) __attribute__((overloadable));
double4 convert_double4_sat_rtz(long4) __attribute__((overloadable));
double4 convert_double4_sat_rtz(ulong4) __attribute__((overloadable));
double4 convert_double4_sat_rtz(float4) __attribute__((overloadable));
double4 convert_double4_sat_rtz(double4) __attribute__((overloadable));
double4 convert_double4_sat_rtp(char4) __attribute__((overloadable));
double4 convert_double4_sat_rtp(uchar4) __attribute__((overloadable));
double4 convert_double4_sat_rtp(short4) __attribute__((overloadable));
double4 convert_double4_sat_rtp(ushort4) __attribute__((overloadable));
double4 convert_double4_sat_rtp(int4) __attribute__((overloadable));
double4 convert_double4_sat_rtp(uint4) __attribute__((overloadable));
double4 convert_double4_sat_rtp(long4) __attribute__((overloadable));
double4 convert_double4_sat_rtp(ulong4) __attribute__((overloadable));
double4 convert_double4_sat_rtp(float4) __attribute__((overloadable));
double4 convert_double4_sat_rtp(double4) __attribute__((overloadable));
double4 convert_double4_sat_rtn(char4) __attribute__((overloadable));
double4 convert_double4_sat_rtn(uchar4) __attribute__((overloadable));
double4 convert_double4_sat_rtn(short4) __attribute__((overloadable));
double4 convert_double4_sat_rtn(ushort4) __attribute__((overloadable));
double4 convert_double4_sat_rtn(int4) __attribute__((overloadable));
double4 convert_double4_sat_rtn(uint4) __attribute__((overloadable));
double4 convert_double4_sat_rtn(long4) __attribute__((overloadable));
double4 convert_double4_sat_rtn(ulong4) __attribute__((overloadable));
double4 convert_double4_sat_rtn(float4) __attribute__((overloadable));
double4 convert_double4_sat_rtn(double4) __attribute__((overloadable));

#endif //__CL_BUILTINS_CONVERT_DOUBLE4_H__

