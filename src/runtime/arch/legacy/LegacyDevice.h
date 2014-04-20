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

#ifndef __LEGACYDCLDEVICE_H
#define __LEGACYDCLDEVICE_H

#include <CLObject.h>
#include <Utils.h>

class LegacyCLDevice : public CLDevice {
public:
  static int CreateDevices(vector<CLDevice*>* devices, cl_device_type type, const char* platform_name);
  LegacyCLDevice(cl_device_id dev, cl_device_type type);
  virtual ~LegacyCLDevice();
  void Init();
  void InitInfo();

  void LaunchKernel(CLCommand* command);
  void ReadBuffer(CLCommand* command);
  void WriteBuffer(CLCommand* command);
  void CopyBuffer(CLCommand* command);
  void BuildProgram(CLCommand* command);
  void ReadBufferRect(CLCommand* command);
  void WriteBufferRect(CLCommand* command);
  void ReadImage(CLCommand* command);
  void WriteImage(CLCommand* command);
  void CopyImage(CLCommand* command);
  void CopyImageToBuffer(CLCommand* command);
  void CopyBufferToImage(CLCommand* command);
  void CopyBufferRect(CLCommand* command);
  void MapBuffer(CLCommand* command);
  void MapImage(CLCommand* command);
  void UnmapMemObject(CLCommand* command);

  void AllocBuffer(CLMem* mem);
  void FreeBuffer(CLCommand* mem);
  void ReadKernelInfo(CLProgram* program);

  cl_int GetSupportedImageFormats(cl_mem_flags flags, cl_mem_object_type image_type, cl_uint num_entries, cl_image_format *image_formats, cl_uint * num_image_formats);

//private:
  cl_device_id      dev;
  cl_context        ctx;
  cl_command_queue  cmq;
};

#endif

