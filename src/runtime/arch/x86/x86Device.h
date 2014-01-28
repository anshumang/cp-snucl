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

#ifndef __X86CLDEVICE_H
#define __X86CLDEVICE_H

#include <arch/x86/x86ComputeUnit.h>
#include <CLObject.h>
#include <Utils.h>
#include <math.h>

class x86CLDevice : public CLDevice {
public:
  static int CreateDevices(vector<CLDevice*>* devices);
  x86CLDevice(int ncore);
  ~x86CLDevice();
  void Init();
  void InitInfo();
	cl_int DivideDevices(const cl_device_partition_property* properties, cl_uint num_devices, cl_device_id* out_devices, cl_uint* num_devices_ret);

  void LaunchKernel(CLCommand* command);
  void NativeKernel(CLCommand* command);
  void ReadBuffer(CLCommand* command);
  void WriteBuffer(CLCommand* command);
  void CopyBuffer(CLCommand* command);
  void BuildProgram(CLCommand* command);
  void CompileProgram(CLCommand* command);
  void LinkProgram(CLCommand* command);
  void BuildProgramFromBinary(CLCommand* command);
  void CompileSource(CLCommand* command);
  void LinkObjects(CLCommand* command);
  void ReadBufferRect(CLCommand* command);
  void WriteBufferRect(CLCommand* command);
  void ReadImage(CLCommand* command);
  void WriteImage(CLCommand* command);
  void CopyImage(CLCommand* command);
  void CopyImageToBuffer(CLCommand* command);
  void CopyBufferToImage(CLCommand* command);
  void CopyBufferRect(CLCommand* command);
  void FillBuffer(CLCommand* command);
  void FillImage(CLCommand* command);
  void MapBuffer(CLCommand* command);
  void MapImage(CLCommand* command);
  void UnmapMemObject(CLCommand* command);
  void MigrateMemObjects(CLCommand* command);

  int GetSupportedImageFormats(cl_mem_flags flags,
                               cl_mem_object_type image_type,
                               cl_uint num_entries,
                               cl_image_format *image_formats,
                               cl_uint *num_image_formats);

  void AllocBuffer(CLMem* mem);
	void CreateBinary(char* file_idx, CLProgram* program);
	bool CheckCompileError(char* file_idx);
	void CreateCompiledBinary(char* file_idx, CLProgram* program);
	void CreateLog(char* logName, CLProgram* program);
	void ReadBin(char* file_idx, CLProgram* program);
	void ReadBinToCompiledObjects(char* file_idx, CLProgram* program);
	void ReadBinToExecutableBinary(char* file_idx, CLProgram* program);
	void ReadKernelInfo(char* kernel_info_file, CLProgram* program);
  void FreeBuffer(CLCommand* command);
	void FreeProgramInfo(CLProgram* program);
  void PrintStatistics();

private:
  void ScheduleDynamic(CLWorkGroupAssignment **wga);
  void ScheduleStatic(CLWorkGroupAssignment **wga);
	void GenFileIdx(char*);
  void BufferRectCommon(const size_t *dst_origin,
                 const size_t *src_origin,
                 const size_t *region,
                 size_t        dst_row_pitch,
                 size_t        dst_slice_pitch,
                 size_t        src_row_pitch,
                 size_t        src_slice_pitch,
                 void *        dst,
                 void *        src);
  void ReadImageCommon(CLMem*      image,
                  const size_t *origin,
                  const size_t *region,
                  size_t        row_pitch,
                  size_t        slice_pitch,
                  void *        src,
                  void *        dst);
  void WriteImageCommon(CLMem*     image,
                  const size_t *origin,
                  const size_t *region,
                  size_t        row_pitch,
                  size_t        slice_pitch,
                  void *        src,
                  void *        dst);
  void PackImagePixel(unsigned int *src, const cl_image_format *format, void *dst);
  void PackImagePixel(int *src, const cl_image_format *format, void *dst);
  void PackImagePixel(float *src, const cl_image_format *format, void *dst);
  int RoundToEven(float val);
  cl_ushort Float2Half_rte(float val);

  int               ncore;
  x86ComputeUnit**  compute_units;
};

#endif

