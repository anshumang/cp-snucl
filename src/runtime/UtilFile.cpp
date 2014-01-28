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

#include <UtilFile.h>

#include <string.h>

size_t SNUCL_GetStringsLength(unsigned int count, const char** strings, const size_t* lengths) {
  size_t ret = 0;

  if (lengths == NULL) {
    for (unsigned int i = 0; i < count; i++) {
      ret += strlen(strings[i]);
    }
  } else {
    for (unsigned int i = 0; i < count; i++) {
      if (lengths[i] == 0) ret += strlen(strings[i]);
      else ret += lengths[i];
    }
  }

  return ++ret;
}

size_t SNUCL_CopyStringsToBuf(char* pbuf, unsigned int count, const char** strings, const size_t* lengths) {
  size_t copy_size = 0;
  size_t length;
  char* buf = pbuf;

  if (lengths == NULL) {
    for (unsigned int i = 0; i < count; i++) {
      length = strlen(strings[i]);
      strcpy(buf, strings[i]);
      buf += length;
      copy_size += length;
    }
  } else {
    for (unsigned int i = 0; i < count; i++) {
      if (lengths[i] == 0) length = strlen(strings[i]);
      else length = lengths[i];
      strncpy(buf, strings[i], length);
      buf += length;
      copy_size += length;
    }
  }

  return copy_size;
}

