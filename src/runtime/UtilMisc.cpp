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

#include <UtilMisc.h>
#include <string>

void getDevicesOfTypes(vector<CLDevice*> oriDevices, vector<CLDevice*>* devices, cl_device_type device_type, cl_int* err) {
	cl_bool check = false;;
	devices->clear();

  if (device_type != CL_DEVICE_TYPE_ALL && (!(device_type & (CL_DEVICE_TYPE_DEFAULT | CL_DEVICE_TYPE_CPU | CL_DEVICE_TYPE_GPU | CL_DEVICE_TYPE_ACCELERATOR | CL_DEVICE_TYPE_CUSTOM)))) {
		*err = CL_INVALID_DEVICE_TYPE;
		return;
	}

	for(vector<CLDevice*>::iterator it = oriDevices.begin(); it != oriDevices.end(); ++it) {
		if( (*it)->type == device_type ) {
			check = true;
			if ( (*it)->available )
				devices->push_back((*it));
		}
	}

	if( devices->size() == 0 ) {
		if( check )
			*err = CL_DEVICE_NOT_AVAILABLE;
		else
			*err = CL_DEVICE_NOT_FOUND;
	}
}

void CheckBuiltInKernels(cl_uint num_devices, const cl_device_id* device_list, const char* kernel_names, cl_int* err) {
	string str;
	string test (kernel_names);
	size_t s, e;

	for(uint i=0; i<num_devices; i++) {
		CLDevice* d = device_list[i]->c_obj;
		string availables (d->built_in_kernels);

		s = 0;
		while(1) {
			e = test.find(';',s);
			str = test.substr(s, e-s);

			if (availables.find(str) == test.npos){
				*err = CL_INVALID_VALUE;
				return;
			}

			if (e == test.npos)
				break;
			s = e+1;
		}
	}
}

char* GenHeaders(cl_uint num, const cl_program* headers, const char** names, const char* options) {
	char* newOptions;
	int newOptLen = 0;
	int optLen = 0;
	int dirLen;
	int prefixLen = strlen(SNUCL_BUILD_HEADER_DIR) + 4;

  mkdir(SNUCL_BUILD_DIR, 0755);
  mkdir(SNUCL_BUILD_HEADER_DIR, 0755);

	for (uint i=0; i<num; i++) {
		dirLen = CreateDirAndHeader(headers[i], names[i]);
		if (dirLen>0)
			newOptLen += (dirLen + prefixLen);
	}
	newOptLen += prefixLen;
	if(options) optLen = strlen(options);

	newOptions = (char*)malloc(optLen + newOptLen);

	if(options) strncpy(newOptions, options, optLen);

	sprintf(newOptions, " -I%s/", SNUCL_BUILD_HEADER_DIR);

	AppendHeader(&(newOptions[optLen+prefixLen]), num, names);

	return newOptions;
}

int CreateDirAndHeader(const cl_program header, const char* name) {
	string path (name);
	string str;
	string snuclDir (SNUCL_BUILD_HEADER_DIR);
	snuclDir += "/";

	size_t s, e;

	s = 0;
	while (1) {
		e = path.find('/', s);
		if (e == path.npos)
			break;

		str = snuclDir + path.substr(0, e);
		mkdir(str.c_str(), 0755);

		s = e+1;
	}

	str = snuclDir + path;
	FILE* fp = fopen(str.c_str(), "w");
	size_t size = strlen(header->c_obj->src);
	size_t ret;
	while(size) {
		ret = fwrite(header->c_obj->src, 1, size, fp);
		size -= ret;
	}
	fclose(fp);

	return s;
}

void AppendHeader(char* options, cl_uint num, const char** names) {
	int prefixLen = strlen(SNUCL_BUILD_HEADER_DIR) + 4;
	string path;
	string str;
	for (uint i=0; i<num; i++) {
		path = names[i];

		size_t s, e;

		s = 0;
		while (1) {
			e = path.find('/', s);
			if (e == path.npos)
				break;
			s = e+1;
		}

		if (s) {
			str = path.substr(0, s);
			sprintf(options, " -I%s/%s", SNUCL_BUILD_HEADER_DIR, str.c_str());

			options = (char*)((unsigned long)options + prefixLen + str.length());
		}
	}
}


void BufferRectCommon(const size_t *src_origin,
                            const size_t *dst_origin,
                            const size_t *region,
                            size_t        src_row_pitch,
                            size_t        src_slice_pitch,
                            size_t        dst_row_pitch,
                            size_t        dst_slice_pitch,
                            void *        src,
                            void *        dst)
{
  // Setup default pitches.
  if(dst_row_pitch == 0)
    dst_row_pitch = region[0];
  if(dst_slice_pitch == 0)
    dst_slice_pitch = region[1]*dst_row_pitch;
  if(src_row_pitch == 0)
    src_row_pitch = region[0];
  if(src_slice_pitch == 0)
    src_slice_pitch = region[1]*src_row_pitch;

  // Copy
  // 'x' is implicitly hided by region[0] for high performance.
  if ((region[2]!=0) && (region[1]!=0)) {
    size_t zmax = dst_origin[2] + region[2];
    size_t ymax = dst_origin[1] + region[1];
    for(size_t zd = dst_origin[2], zs = src_origin[2]; zd < zmax; ++zd, ++zs) {
      for(size_t yd = dst_origin[1], ys = src_origin[1]; yd < ymax; ++yd, ++ys) {
        size_t doff, soff;
        doff = zd*dst_slice_pitch + yd*dst_row_pitch + dst_origin[0];
        soff = zs*src_slice_pitch + ys*src_row_pitch + src_origin[0];
        memcpy((char*)dst+doff, (char*)src+soff, region[0]);
      }
    }
  }
  else if ((region[2]==0) && (region[1]!=0)) {
    size_t ymax = dst_origin[1] + region[1];
    for(size_t yd = dst_origin[1], ys = src_origin[1]; yd < ymax; ++yd, ++ys) {
      size_t doff, soff;
      doff = yd*dst_row_pitch + dst_origin[0];
      soff = ys*src_row_pitch + src_origin[0];
      memcpy((char*)dst+doff, (char*)src+soff, region[0]);
    }
  }
  else if ((region[2]==0) && (region[1]==0)) {
    memcpy((char*)dst+dst_origin[0], (char*)src+src_origin[0], region[0]);
  }
  else {
  }
}

void GetImageDesc(const cl_image_format *image_format, const cl_image_desc *image_desc, size_t *ret_size, size_t *ret_elem_size, size_t *ret_row_pitch, size_t *ret_slice_pitch, int *ret_channels, cl_int *errcode_ret) {
  cl_channel_type type = image_format->image_channel_data_type;
  size_t size = 0;
  size_t elem_size = 0;
  size_t row_pitch = 0;
  size_t slice_pitch = 0;
  cl_int err = CL_SUCCESS;

  if      (type == CL_SNORM_INT8)     elem_size = 1;
  else if (type == CL_SNORM_INT16)    elem_size = 2;
  else if (type == CL_UNORM_INT8)     elem_size = 1;
  else if (type == CL_UNORM_INT16)    elem_size = 2;
  else if (type == CL_UNORM_SHORT_565)elem_size = 2;
  else if (type == CL_UNORM_SHORT_555)elem_size = 2;
  else if (type == CL_UNORM_INT_101010)elem_size = 4;
  else if (type == CL_SIGNED_INT8)    elem_size = 1;
  else if (type == CL_SIGNED_INT16)   elem_size = 2;
  else if (type == CL_SIGNED_INT32)   elem_size = 4;
  else if (type == CL_UNSIGNED_INT8)  elem_size = 1;
  else if (type == CL_UNSIGNED_INT16) elem_size = 2;
  else if (type == CL_UNSIGNED_INT32) elem_size = 4;
  else if (type == CL_HALF_FLOAT)     elem_size = 2;
  else if (type == CL_FLOAT)          elem_size = 4;
  else err = CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;

  if (errcode_ret) *errcode_ret = err;
  if (err != CL_SUCCESS) return ;

  int CHANNELS = 4;
  cl_channel_order ch_order = image_format->image_channel_order;
  if (ch_order == CL_R) CHANNELS = 1;
  else if (ch_order == CL_A) CHANNELS = 1;
  else if (ch_order == CL_RG) CHANNELS = 2;
  else if (ch_order == CL_RA) CHANNELS = 2;
  else if (ch_order == CL_RGB) CHANNELS = 3;
  else if (ch_order == CL_RGBA) CHANNELS = 4;
  else if (ch_order == CL_BGRA) CHANNELS = 4;
  else if (ch_order == CL_ARGB) CHANNELS = 4;
  else if (ch_order == CL_INTENSITY) CHANNELS = 1;
  else if (ch_order == CL_LUMINANCE) CHANNELS = 1;
  else if (ch_order == CL_Rx) CHANNELS = 1;
  else if (ch_order == CL_RGx) CHANNELS = 2;
  else if (ch_order == CL_RGBx) CHANNELS = 3;

  elem_size *= CHANNELS;
  if (image_desc->image_row_pitch == 0) {
    row_pitch = image_desc->image_width * elem_size;
  }
  else row_pitch = image_desc->image_row_pitch;

  if (image_desc->image_slice_pitch == 0) {
    if ((image_desc->image_type == CL_MEM_OBJECT_IMAGE2D) 
        || (image_desc->image_type == CL_MEM_OBJECT_IMAGE3D)) {
      slice_pitch = image_desc->image_height * row_pitch;
    }
    else if (image_desc->image_type == CL_MEM_OBJECT_IMAGE2D_ARRAY) {
      slice_pitch = image_desc->image_height * row_pitch;
    }
    else if (image_desc->image_type == CL_MEM_OBJECT_IMAGE1D_ARRAY) {
      slice_pitch = row_pitch;
    }
  }
  else slice_pitch = image_desc->image_slice_pitch;

  if (image_desc->image_type == CL_MEM_OBJECT_IMAGE3D) {
    size = image_desc->image_depth * image_desc->image_height * image_desc->image_width * elem_size;
  }
  else if (image_desc->image_type == CL_MEM_OBJECT_IMAGE2D_ARRAY) {
    size = image_desc->image_array_size * image_desc->image_height * image_desc->image_width * elem_size;
  }
  else if (image_desc->image_type == CL_MEM_OBJECT_IMAGE2D) {
    size = image_desc->image_height * image_desc->image_width * elem_size;
  }
  else if (image_desc->image_type == CL_MEM_OBJECT_IMAGE1D_ARRAY) {
    size = image_desc->image_array_size * slice_pitch;
  }
  else if (image_desc->image_type == CL_MEM_OBJECT_IMAGE1D) {
    size = image_desc->image_width * elem_size;
  }
  else if (image_desc->image_type == CL_MEM_OBJECT_IMAGE1D_BUFFER) {
    size = image_desc->buffer->c_obj->size;
  }
  else {
  }
  if(ret_size) *ret_size = size;
  if(ret_elem_size) *ret_elem_size = elem_size;
  if(ret_row_pitch) *ret_row_pitch = row_pitch;
  if(ret_slice_pitch) *ret_slice_pitch = slice_pitch;
  if(ret_channels) *ret_channels = CHANNELS;
}
