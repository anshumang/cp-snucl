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

#ifndef __CLUSTERMESSAGE_H
#define __CLUSTERMESSAGE_H

#include <CLObject.h>

#define CLUSTER_TAG_EXIT              0
#define CLUSTER_TAG_NODE_INFO         1
#define CLUSTER_TAG_DEV_INFO          2
#define CLUSTER_TAG_COMMAND           3
#define CLUSTER_TAG_PROGRAM_BUILD     4
#define CLUSTER_TAG_KERNEL_LAUNCH     5
#define CLUSTER_TAG_MEM_ALLOC         6
#define CLUSTER_TAG_MEM_COPY          7
#define CLUSTER_TAG_MEM_SEND          8
#define CLUSTER_TAG_MEM_RECV          9
#define CLUSTER_TAG_MEM_COPY_RECT    10
#define CLUSTER_TAG_MEM_SEND_RECT    11
#define CLUSTER_TAG_MEM_RECV_RECT    12
#define CLUSTER_TAG_MEM_BCAST        13
#define CLUSTER_TAG_MEM_ALLTOALL     14
#define CLUSTER_TAG_MEM_FREE         15
#define CLUSTER_TAG_DEV_STATS        16
#define CLUSTER_TAG_MARKER           17
#define CLUSTER_TAG_NATIVE_KERNEL    18
#define CLUSTER_TAG_IMAGE_INFO       19
#define CLUSTER_TAG_IMAGE_COPY       20
#define CLUSTER_TAG_IMAGE_SEND       21
#define CLUSTER_TAG_IMAGE_RECV       22
#define CLUSTER_TAG_BUFFER_IMAGE     23
#define CLUSTER_TAG_IMAGE_BUFFER     24

#define CLUSTER_TAG_EVENT_WAIT(id)        (100 + 2 * (id) + 0)
#define CLUSTER_TAG_MEM_SEND_BODY(id)     (100 + 2 * (id) + 1)
#define CLUSTER_TAG_MEM_RECV_BODY(id)     (100 + 2 * (id) + 1)

const char CLUSTER_TAG_STR[25][32] = 
{
  "EXIT",
  "DEV_INFO",
  "NODE_INFO",
  "COMMAND",
  "PROGRAM_BUILD",
  "KERNEL_LAUNCH",
  "MEM_ALLOC",
  "MEM_COPY",
  "MEM_SEND",
  "MEM_RECV",
  "MEM_COPY_RECT",
  "MEM_SEND_RECT",
  "MEM_RECV_RECT",
  "MEM_BCAST",
  "MEM_ALLTOALL",
  "MEM_FREE",
  "DEV_STATS",
  "MARKER",
  "NATIVE_KERNEL",
  "IMAGE_INFO",
  "IMAGE_COPY",
  "IMAGE_SEND",
  "IMAGE_RECV",
  "COPY_BUFFER_TO_IMAGE",
  "COPY_IMAGE_TO_BUFFER",
};

class MessageCodec {
public:
  MessageCodec();
  MessageCodec(char* msg);
  MessageCodec(CLCommand* command);
  ~MessageCodec();

  void Init();

  void Finish(CLCommand* command);

  void Set(void* src, size_t size);
  void SetTag(int tag);
  void SetCID(unsigned long cid);
  void SetBool(void* src);
  void SetChar(void* src);
  void SetInt(void* src);
  void SetUInt(void* src);
  void SetLong(void* src);
  void SetULong(void* src);

  void Get(void* dst, int size);
  int  GetTag();
  unsigned long GetCID();
  bool GetBool();
  char GetChar();
  int  GetInt();
  unsigned int GetUInt();
  long GetLong();
  unsigned long GetULong();

  int Offset();

public:
  char* msg;

private:
  int   msg_offset;
  char  msg_internal[MESSAGE_COMMAND_SIZE];

};

#endif

