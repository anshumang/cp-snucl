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

#ifndef __UTILMISC_H
#define __UTILMISC_H

#include <stdio.h>
#include <string.h>
#include <CLObject.h>
#include <sys/stat.h>

#define SNUCL_IDIV(a, b)  ((a) % (b)) == 0 ? ((a) / (b)) : (((a) / (b)) + 1)

#define SNUCL_GetObjectInfo(param, type, value)           \
case param: {                                             \
  size_t size = sizeof(type);                             \
  if (param_value) {                                      \
    if (param_value_size < size) return CL_INVALID_VALUE; \
    memcpy(param_value, &(value), size);                  \
  }                                                       \
  if (param_value_size_ret) *param_value_size_ret = size; \
  break;                                                  \
}

#define SNUCL_GetObjectInfoV(param, type, value)          \
case param: {                                             \
  size_t size = sizeof(type);                             \
  if (param_value) {                                      \
    if (param_value_size < size) return CL_INVALID_VALUE; \
		type temp = value;                                    \
    memcpy(param_value, &temp, size);                     \
  }                                                       \
  if (param_value_size_ret) *param_value_size_ret = size; \
  break;                                                  \
}

#define SNUCL_GetObjectInfoA(param, type, value, length)  \
case param: {                                             \
  size_t size = sizeof(type) * length;                    \
  if (value == NULL) {                                    \
    if (param_value_size_ret) *param_value_size_ret = 0;  \
    break;                                                \
  }                                                       \
  if (param_value) {                                      \
    if (param_value_size < size) return CL_INVALID_VALUE; \
    memcpy(param_value, value, size);                     \
  }                                                       \
  if (param_value_size_ret) *param_value_size_ret = size; \
  break;                                                  \
}

#define SNUCL_DevicesVerification(list, numList, targets, numTargets, err); \
{                                                                           \
	bool exist = false;                                                       \
	for(int i=0; i<numTargets; i++) {                                         \
		for(int j=0; j<numList; j++) {                                          \
			if( (list)[j] == (targets)[i]->c_obj ){                               \
				exist = true;                                                       \
				break;                                                              \
			}                                                                     \
		}                                                                       \
		if(!exist) {                                                            \
			err = CL_INVALID_DEVICE;                                              \
			break;                                                                \
		}                                                                       \
	}                                                                         \
}                                                                           

void getDevicesOfTypes(vector<CLDevice*> oriDevices, vector<CLDevice*>* devices, cl_device_type device_type, cl_int* err);
void CheckBuiltInKernels(cl_uint num_devices, const cl_device_id* device_list, const char* kernel_names, cl_int* err);
void CheckOptions(CLProgram* program, const char* options, cl_int* err);
char* GenHeaders(cl_uint num, const cl_program* headers, const char** names, const char* options);
int CreateDirAndHeader(const cl_program header, const char* name);
void AppendHeader(char* options, cl_uint num, const char** name);
void BufferRectCommon(const size_t* src_origin, const size_t* dst_origin, const size_t* region, size_t src_row_pitch, size_t src_slice_pitch, size_t dst_row_pitch, size_t dst_slice_pitch, void* src, void* dst);
void GetImageDesc(const cl_image_format *image_format, const cl_image_desc *image_desc, size_t *ret_size, size_t *ret_elem_size, size_t *ret_row_pitch, size_t *ret_slice_pitch, int *ret_channels, cl_int *errcode_ret);

#endif
