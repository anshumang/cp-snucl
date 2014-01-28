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

#include <arch/cluster/ClusterMessage.h>
#include <string.h>

MessageCodec::MessageCodec() {
  msg_offset = 0;
  msg = msg_internal;
}

MessageCodec::MessageCodec(char* msg) {
  msg_offset = 0;
  this->msg = msg;
}

MessageCodec::MessageCodec(CLCommand* command) {
  msg_offset = 0;
  msg = command->msg;
}

MessageCodec::~MessageCodec() {
}

void MessageCodec::Init() {
  msg_offset = 0;
}

void MessageCodec::Finish(CLCommand* command) {
  command->msg_size = msg_offset;
}

void MessageCodec::Set(void* src, size_t size) {
  memcpy((void*) ((size_t) msg + msg_offset), src, size);
  msg_offset += size;
}

void MessageCodec::SetTag(int tag) {
  Set((void*) &tag, sizeof(int));
}

void MessageCodec::SetCID(unsigned long cid) {
  Set((void*) &cid, sizeof(unsigned long));
}

void MessageCodec::SetBool(void* src) {
  Set(src, sizeof(bool));
}

void MessageCodec::SetChar(void* src) {
  Set(src, sizeof(char));
}

void MessageCodec::SetInt(void* src) {
  Set(src, sizeof(int));
}

void MessageCodec::SetUInt(void* src) {
  Set(src, sizeof(unsigned int));
}

void MessageCodec::SetLong(void* src) {
  Set(src, sizeof(long));
}

void MessageCodec::SetULong(void* src) {
  Set(src, sizeof(unsigned long));
}

void MessageCodec::Get(void* dst, int size) {
  memcpy(dst, (void*) ((size_t) msg + msg_offset), size);
  msg_offset += size;
}

int MessageCodec::GetTag() {
  return GetInt();
}

unsigned long MessageCodec::GetCID() {
  return GetULong();
}

bool MessageCodec::GetBool() {
  bool ret;
  Get(&ret, sizeof(bool));
  return ret;
}

char MessageCodec::GetChar() {
  char ret;
  Get(&ret, sizeof(char));
  return ret;
}

int MessageCodec::GetInt() {
  int ret;
  Get(&ret, sizeof(int));
  return ret;
}

unsigned int MessageCodec::GetUInt() {
  unsigned int ret;
  Get(&ret, sizeof(unsigned int));
  return ret;
}

long MessageCodec::GetLong() {
  long ret;
  Get(&ret, sizeof(long));
  return ret;
}

unsigned long MessageCodec::GetULong() {
  unsigned long ret;
  Get(&ret, sizeof(unsigned long));
  return ret;
}

int MessageCodec::Offset() {
  return msg_offset;
}

