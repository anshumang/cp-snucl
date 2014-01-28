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

#include <arch/x86/x86Device.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <malloc.h>
#include <math.h>
#include <float.h>
#include <limits.h>

SNUCL_DEBUG_HEADER("x86Device");

int x86CLDevice::CreateDevices(vector<CLDevice*>* devices) {
  char buf[8];
  FILE* fp = popen("/bin/cat /proc/cpuinfo | grep -c '^processor'", "r");
  size_t size = fread(buf, sizeof(char), sizeof(buf) - 1, fp);
  pclose(fp);
  int ncore = atoi(buf);

  SNUCL_DEBUG("%d CPU cores are detected.", ncore);

  devices->push_back(new x86CLDevice(ncore));

  return 1;
}

x86CLDevice::x86CLDevice(int ncore) : CLDevice(CL_DEVICE_TYPE_CPU) {
	srand(time(NULL));
  this->ncore = ncore;

  time_cmd_k = 0;
  time_cmd_s = 0;

  worker = new CLDeviceWorker(this);
}

x86CLDevice::~x86CLDevice() {
  delete worker;

  for (int i = 0; i < ncore; ++i) delete compute_units[i];
  free(compute_units);
}

void x86CLDevice::Init() {
  compute_units = (x86ComputeUnit**) malloc(sizeof(x86ComputeUnit*) * ncore);
  for (int i = 0; i < ncore; ++i) {
    compute_units[i] = new x86ComputeUnit(this, i);
  }
}

void x86CLDevice::InitInfo() {
  vendor_id = 201110;
  max_compute_units = ncore;
  max_work_item_dimensions = 3;
  max_work_item_sizes[0] = max_work_item_sizes[1] = max_work_item_sizes[2] = 4096;
  max_work_group_size = 4096;

  float freq = 1;
  preferred_vector_width_char = 16;
  preferred_vector_width_short = 8;
  preferred_vector_width_int = 4;
  preferred_vector_width_long = 2;
  preferred_vector_width_float = 4;
  preferred_vector_width_double = 2;
  preferred_vector_width_half = 0;
  native_vector_width_char = 16;
  native_vector_width_short = 8;
  native_vector_width_int = 4;
  native_vector_width_long = 2;
  native_vector_width_float = 4;
  native_vector_width_double = 2;
  native_vector_width_half = 0;
  FILE* fp = popen("/bin/cat /proc/cpuinfo | grep '^cpu MHz' | head -1", "r");
  if (fp) {
    char buf[256];
    size_t size = fread(buf, sizeof(char), sizeof(buf) - 1, fp);
    pclose(fp);
    char dummy[256];
    sscanf(buf, "cpu MHz %s %f", dummy, &freq);
  } else {
    SNUCL_ERROR("%s", "ERROR GET FREQ");
  }
  max_clock_frequency = (cl_uint) freq;
  address_bits = 64;
//  max_mem_alloc_size = 48 * 1024 * 1024 * 1024ULL;
  max_mem_alloc_size = 2 * 1024 * 1024 * 1024ULL;
  image_support = CL_TRUE;

	max_read_image_args = 128;
	max_write_image_args = 128;
	image2d_max_width = 8192;
	image2d_max_height = 8192;
	image3d_max_width = 2048;
	image3d_max_height = 2048;
	image3d_max_depth = 2048;
	image_max_buffer_size = 65536;
	image_max_array_size = 65536;
	max_samplers = 16;
  max_parameter_size = 1024;
  mem_base_addr_align = 1024;
  min_data_type_align_size = sizeof(cl_long16);
  single_fp_config = CL_FP_DENORM | CL_FP_INF_NAN | CL_FP_ROUND_TO_NEAREST | CL_FP_ROUND_TO_ZERO | CL_FP_ROUND_TO_INF | CL_FP_FMA;// | CL_FP_CORRECTLY_ROUNDED_DIVIDE_SQRT | CL_FP_SOFT_FLOAT;
  double_fp_config = CL_FP_DENORM | CL_FP_INF_NAN | CL_FP_ROUND_TO_NEAREST | CL_FP_ROUND_TO_ZERO | CL_FP_ROUND_TO_INF | CL_FP_FMA;
  global_mem_cache_type = CL_READ_WRITE_CACHE;
  global_mem_cacheline_size = 128;
  global_mem_cache_size = 4 * 1024 * 1024;
//  global_mem_size = 48 * 1024 * 1024 * 1024ULL;
  global_mem_size = 2 * 1024 * 1024 * 1024ULL;
  max_constant_buffer_size = 64 * 1024;
  max_constant_args = 9;
  local_mem_type = CL_GLOBAL;
  local_mem_size = 4 * 1024 * 1024;
  error_correction_support = CL_FALSE;
  host_unified_memory = CL_TRUE;
  profiling_timer_resolution = 1000;
  endian_little = CL_TRUE;
  available = CL_TRUE;
  compiler_available = CL_TRUE;
  linker_available = CL_TRUE;
  execution_capabilities = CL_EXEC_KERNEL | CL_EXEC_NATIVE_KERNEL;
  queue_properties = CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_PROFILING_ENABLE;
  strcpy(built_in_kernels, "");
  strcpy(name, node_name);
  strcpy(vendor, "GenuineIntel");
  strcpy(driver_version, "SnuCL 1.2");
  strcpy(profile, "FULL_PROFILE");
  strcpy(device_version, "OpenCL 1.2 rev00");
  strcpy(openclc_version, "OpenCL C 1.2 rev00");
  strcpy(device_extensions, "cl_khr_global_int32_base_atomics cl_khr_global_int32_extended_atomics cl_khr_local_int32_base_atomics cl_khr_local_int32_extended_atomics cl_khr_byte_addressable_store cl_khr_fp64");
	printf_buffer_size = 1024 * 1024;
	preferred_interop_user_sync = CL_FALSE;
	parent_device = NULL; 
	partition_properties[0] = CL_DEVICE_PARTITION_EQUALLY; 
	partition_properties[1] = CL_DEVICE_PARTITION_BY_COUNTS;
	num_partition_properties = 2;
	partition_max_sub_devices = ncore;
	affinity_domain = 0; 
	numSubDevices = 0;
} 

void x86CLDevice::PrintStatistics() {
  SNUCL_INFO("x86[%s] [%7.3lf / %7.3lf]", name, (double) time_cmd_k / 1000000.0, (double) time_cmd_s / 1000000.0);
  time_cmd_k = time_cmd_s = 0;
}

cl_int x86CLDevice::DivideDevices(const cl_device_partition_property* properties, cl_uint num_devices, cl_device_id* out_devices, cl_uint* num_devices_ret) {
	size_t idx;
	unsigned int numComputeUnits;
	CLDevice* subDev;

	cl_device_partition_property type = properties[0];

	switch (type) {
		case CL_DEVICE_PARTITION_EQUALLY: {
			if (properties[1] <= 0 || properties[1] > max_compute_units) return CL_DEVICE_PARTITION_FAILED;
			if (properties[2] != 0) return CL_INVALID_VALUE;

			numSubDevices = max_compute_units/properties[1];

			if (num_devices_ret) *num_devices_ret = numSubDevices;

			if (out_devices) {
				for (uint i=0; i<numSubDevices; i++) {
					subDev = new x86CLDevice(properties[1]);
					platform->devices.push_back(subDev);
					subDevices.push_back(subDev);
					subDev->Retain();
				}

				AddSubDevicesToScheduler();

				for (cl_uint i = 0; i < numSubDevices; i++) {
					subDev = subDevices[i];
					subDev->parent_device = &this->st_obj;
					subDev->partition_type = (cl_device_partition_property*)malloc(sizeof(cl_device_partition_property)*3);
					for (uint j=0; j<3; j++)
						subDev->partition_type[j] = properties[j];
					subDev->partition_type_len = 3;
					if (num_devices > i)
						out_devices[i] = &subDev->st_obj;
				}
			}
			break;
		}
		case CL_DEVICE_PARTITION_BY_COUNTS: {
			idx = 0;
			numSubDevices = 0;
			numComputeUnits = 0;

			while (1) {
				idx++;
				if (properties[idx] == CL_DEVICE_PARTITION_BY_COUNTS_LIST_END) break;
				numSubDevices++;
				numComputeUnits += properties[idx];
				if (properties[idx] <= 0 || numComputeUnits > max_compute_units) return CL_DEVICE_PARTITION_FAILED;
			}
			if (properties[++idx] != 0) return CL_INVALID_VALUE;
			if (numSubDevices > partition_max_sub_devices) return CL_INVALID_DEVICE_PARTITION_COUNT;

			if (num_devices_ret) *num_devices_ret = numSubDevices;

			if (out_devices) {
				for (uint i=0; i<numSubDevices; i++)  {
					subDev = new x86CLDevice(properties[i+1]);
					platform->devices.push_back(subDev);
					subDevices.push_back(subDev);
					subDev->Retain();
				}

				AddSubDevicesToScheduler();

				for (cl_uint i = 0; i < numSubDevices; i++) {
					subDev = subDevices[i];
					subDev->parent_device = &this->st_obj;
					subDev->partition_type = (cl_device_partition_property*)malloc(sizeof(cl_device_partition_property)*idx);
					for (uint j=0; j<idx; j++)
						subDev->partition_type[j] = properties[j];
					subDev->partition_type_len = idx;
					if (num_devices > i)
						out_devices[i] = &subDev->st_obj;
				}
			}
			break;
		}
//		case CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN:  return CL_INVALID_VALUE; //not yet supported
		default: return CL_INVALID_VALUE;
	}

	return CL_SUCCESS;
}

void x86CLDevice::LaunchKernel(CLCommand* command) {
  unsigned long t0, t1;
  unsigned long k_t0, k_t1;

  t0 = SNUCL_Timestamp();

  CLKernel* kernel = command->kernel;
  CLProgram* program = command->kernel->program;

  map<cl_uint, CLKernelArg*>* args = command->kernel_args;
  for (map<cl_uint, CLKernelArg*>::const_iterator it = args->begin(); it != args->end(); ++it) {
    CLKernelArg* arg = it->second;
    CLMem* mem = arg->mem;
    if (mem) CheckBuffer(mem);
  }

  CLWorkGroupAssignment **wga = (CLWorkGroupAssignment**) malloc(sizeof(CLWorkGroupAssignment*) * ncore);
  for (int i = 0; i < ncore; ++i) wga[i] = new CLWorkGroupAssignment(command);

  k_t0 = SNUCL_Timestamp();
  ScheduleDynamic(wga);
  //ScheduleStatic(wga);

  for (int i = 0; i < ncore; ++i) compute_units[i]->Sync();
  k_t1 = SNUCL_Timestamp();

  for (int i = 0; i < ncore; ++i) delete wga[i];
  free(wga);

  for (map<cl_uint, CLKernelArg*>::const_iterator it = args->begin(); it != args->end(); ++it) {
    CLKernelArg* arg = it->second;
    CLMem* mem = arg->mem;
    if (!mem) continue;
    if (mem->flags & CL_MEM_WRITE_ONLY || mem->flags & CL_MEM_READ_WRITE) {
      mem->ClearLatest(this);
    }
  }

  command->event->Complete();

  t1 = SNUCL_Timestamp();

  time_cmd_k += k_t1 - k_t0;
  time_cmd_s += t1 - t0;
}

void x86CLDevice::NativeKernel(CLCommand* command) {
  size_t cb_args = (*command->kernel_args)[0]->size;
  void* args = (void*) (*command->kernel_args)[0]->value;
  cl_uint num_mem_objects = (*command->kernel_args)[1]->size / sizeof(cl_mem);
  cl_mem* mem_list = (cl_mem*) (*command->kernel_args)[1]->value;
  void** args_mem_loc = (void**) (*command->kernel_args)[2]->value;

  for(int i = 0; i < num_mem_objects; ++i) {
    CLMem* mem = mem_list[i]->c_obj;
    CheckBuffer(mem);
    size_t offset = (size_t) args_mem_loc[i] - (size_t) command->ptr;
    memcpy((void*) ((size_t) args + offset), &mem->dev_specific[this], sizeof(void*));
  }
  command->user_func(args);

  command->event->Complete();
}

void x86CLDevice::ReadBuffer(CLCommand* command) {
  CLMem* mem_src = command->mem_src;
  size_t off_src = command->off_src;
  size_t size = command->cb;
  void* ptr = command->ptr;

  CheckBuffer(mem_src);

  pthread_mutex_lock(&mem_src->mutex_dev_specific);
  void* m = mem_src->dev_specific[this];
  pthread_mutex_unlock(&mem_src->mutex_dev_specific);

  memcpy(ptr, (void*) ((size_t) m + off_src), size);

  command->event->Complete();
}

void x86CLDevice::BufferRectCommon(const size_t *src_origin,
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

void x86CLDevice::ReadImageCommon(CLMem*      image,
                             const size_t *origin,
                             const size_t *region,
                             size_t        row_pitch,
                             size_t        slice_pitch,
                             void *        src,
                             void *        dst)
{
  size_t image_depth = image->image_desc.image_depth;
  size_t image_height = image->image_desc.image_height;
  size_t image_width = image->image_desc.image_width;
  size_t image_row_pitch = image->image_row_pitch;
  size_t image_slice_pitch = image->image_slice_pitch;
  size_t elem_size = image->image_elem_size;
  cl_image_desc *image_info = &image->image_desc;

  if(row_pitch == 0) {
    row_pitch = region[0]*elem_size;
  }
  if(slice_pitch == 0) {
    slice_pitch = region[1]*row_pitch;
  }

  if (image_info->image_type == CL_MEM_OBJECT_IMAGE3D) {
    size_t zmax = origin[2] + region[2];
    size_t ymax = origin[1] + region[1];
    for(size_t z=origin[2], zd=0; z<zmax; ++z, ++zd) {
      for(size_t y=origin[1], yd=0; y<ymax; ++y, ++yd) {
        size_t soff, doff;
        doff = zd*slice_pitch + yd*row_pitch;
        soff = z*image_slice_pitch + y*image_row_pitch + origin[0]*elem_size;
        memcpy((char*)dst+doff, (char*)src+soff, region[0]*elem_size);
      }
    }
  }
  else if (image_info->image_type == CL_MEM_OBJECT_IMAGE2D_ARRAY) {
    size_t zmax = origin[2] + region[2];
    size_t ymax = origin[1] + region[1];
    for(size_t z=origin[2], zd=0; z<zmax; ++z, ++zd) {
      for(size_t y=origin[1], yd=0; y<ymax; ++y, ++yd) {
        size_t soff, doff;
        doff = zd*slice_pitch + yd*row_pitch;
        soff = z*image_slice_pitch + y*image_row_pitch + origin[0]*elem_size;
        memcpy((char*)dst+doff, (char*)src+soff, region[0]*elem_size);
      }
    }
  }
  else if (image_info->image_type == CL_MEM_OBJECT_IMAGE2D) {
    size_t ymax = origin[1] + region[1];
    for(size_t y=origin[1], yd=0; y<ymax; ++y, ++yd) {
      size_t soff, doff;
      doff = yd*row_pitch;
      soff = y*image_row_pitch + origin[0]*elem_size;
      memcpy((char*)dst+doff, (char*)src+soff, region[0]*elem_size);
    }
  }
  else if (image_info->image_type == CL_MEM_OBJECT_IMAGE1D_ARRAY) {
    size_t ymax = origin[1] + region[1];
    for(size_t y=origin[1], yd=0; y<ymax; ++y, ++yd) {
      size_t soff, doff;
      doff = yd*row_pitch;
      soff = y*image_slice_pitch + origin[0]*elem_size;
      memcpy((char*)dst+doff, (char*)src+soff, region[0]*elem_size);
    }
  }
  else if (image_info->image_type == CL_MEM_OBJECT_IMAGE1D) {
    size_t soff, doff;
    doff = 0;
    soff = origin[0]*elem_size;
    memcpy((char*)dst+doff, (char*)src+soff, region[0]*elem_size);
  }
  else {
  }
}

void x86CLDevice::WriteImageCommon(CLMem*      image,
                             const size_t *origin,
                             const size_t *region,
                             size_t        row_pitch,
                             size_t        slice_pitch,
                             void *        src,
                             void *        dst)
{
  size_t image_depth = image->image_desc.image_depth;
  size_t image_height = image->image_desc.image_height;
  size_t image_width = image->image_desc.image_width;
  size_t image_row_pitch = image->image_row_pitch;
  size_t image_slice_pitch = image->image_slice_pitch;
  size_t elem_size = image->image_elem_size;
  cl_image_desc *image_info = &image->image_desc;

  if(row_pitch == 0) {
    row_pitch = region[0]*elem_size;
  }
  if(slice_pitch == 0) {
    slice_pitch = region[1]*row_pitch;
  }

  if ((image_info->image_type == CL_MEM_OBJECT_IMAGE3D)
      || (image_info->image_type == CL_MEM_OBJECT_IMAGE2D_ARRAY)) {
    size_t zmax = origin[2] + region[2];
    size_t ymax = origin[1] + region[1];
    for(size_t z=origin[2], zd=0; z<zmax; ++z, ++zd) {
      for(size_t y=origin[1], yd=0; y<ymax; ++y, ++yd) {
        size_t soff, doff;
        doff = z*image_slice_pitch + y*image_row_pitch + origin[0]*elem_size;
        soff = zd*slice_pitch + yd*row_pitch;
        memcpy((char*)dst+doff, (char*)src+soff, region[0]*elem_size);
      }
    }
  }
  else if (image_info->image_type == CL_MEM_OBJECT_IMAGE2D) {
    size_t ymax = origin[1] + region[1];
    for(size_t y=origin[1], yd=0; y<ymax; ++y, ++yd) {
      size_t soff, doff;
      doff = y*image_row_pitch + origin[0]*elem_size;
      soff = yd*row_pitch;
      memcpy((char*)dst+doff, (char*)src+soff, region[0]*elem_size);
    }
  }
  else if (image_info->image_type == CL_MEM_OBJECT_IMAGE1D_ARRAY) {
    size_t ymax = origin[1] + region[1];
    for(size_t y=origin[1], yd=0; y<ymax; ++y, ++yd) {
      size_t soff, doff;
      doff = y*image_slice_pitch + origin[0]*elem_size;
      soff = yd*row_pitch;
      memcpy((char*)dst+doff, (char*)src+soff, region[0]*elem_size);
    }
  }
  else if (image_info->image_type == CL_MEM_OBJECT_IMAGE1D) {
    size_t soff, doff;
    doff = origin[0]*elem_size;
    soff = 0;
    memcpy((char*)dst+doff, (char*)src+soff, region[0]*elem_size);
  }
  else {
		SNUCL_ERROR("%s","Cannot Reach Here\n");
  }
}

void x86CLDevice::ReadBufferRect(CLCommand* command) {
  CLMem* buffer = command->buffer;
  CheckBuffer(buffer);

  pthread_mutex_lock(&buffer->mutex_dev_specific);
  void* src = buffer->dev_specific[this];
  pthread_mutex_unlock(&buffer->mutex_dev_specific);

  BufferRectCommon(command->src_origin,
            command->dst_origin,
            command->region,
            command->src_row_pitch,
            command->src_slice_pitch,
            command->dst_row_pitch,
            command->dst_slice_pitch,
            src,
            command->ptr);

  command->event->Complete();
}

void x86CLDevice::WriteBufferRect(CLCommand* command) {
  CLMem* buffer = command->buffer;
  CheckBuffer(buffer);

  pthread_mutex_lock(&buffer->mutex_dev_specific);
  void* dst = buffer->dev_specific[this];
  pthread_mutex_unlock(&buffer->mutex_dev_specific);

  BufferRectCommon(command->src_origin,
            command->dst_origin,
            command->region,
            command->src_row_pitch,
            command->src_slice_pitch,
            command->dst_row_pitch,
            command->dst_slice_pitch,
            command->ptr,
            dst);

  command->event->Complete();
}

void x86CLDevice::ReadImage(CLCommand* command) {
  CLMem* mem_src = command->mem_src;

  CheckBuffer(mem_src);

  pthread_mutex_lock(&mem_src->mutex_dev_specific);
  void* m_src = mem_src->dev_specific[this];
  pthread_mutex_unlock(&mem_src->mutex_dev_specific);

  ReadImageCommon((CLMem*)mem_src,
              command->src_origin,
              command->region,
              command->dst_row_pitch,
              command->dst_slice_pitch,
              m_src,
              command->ptr);

  command->event->Complete();
}

void x86CLDevice::WriteImage(CLCommand* command) {
  CLMem* mem_dst = command->mem_dst;

  CheckBuffer(mem_dst);

  pthread_mutex_lock(&mem_dst->mutex_dev_specific);
  void* m_dst = mem_dst->dev_specific[this];
  pthread_mutex_unlock(&mem_dst->mutex_dev_specific);

  WriteImageCommon((CLMem*)mem_dst,
              command->dst_origin,
              command->region,
              command->src_row_pitch,
              command->src_slice_pitch,
              command->ptr,
              m_dst);

  command->event->Complete();
}

void x86CLDevice::CopyImage(CLCommand* command) {
  CLMem* mem_src = command->mem_src;
  CLMem* mem_dst = command->mem_dst;

  CheckBuffer(mem_src);
  CheckBuffer(mem_dst);

  pthread_mutex_lock(&mem_src->mutex_dev_specific);
  void* m_src = mem_src->dev_specific[this];
  pthread_mutex_unlock(&mem_src->mutex_dev_specific);
  if (m_src == NULL) SNUCL_ERROR("SRC_MEM[%d] IS NULL", mem_src->id);

  pthread_mutex_lock(&mem_dst->mutex_dev_specific);
  void* m_dst = mem_dst->dev_specific[this];
  pthread_mutex_unlock(&mem_dst->mutex_dev_specific);
  if (m_dst == NULL) SNUCL_ERROR("DST_MEM[%d] IS NULL", mem_dst->id);

  CLMem *src_image = mem_src;
  CLMem *dst_image = mem_dst;
  const size_t *src_origin = command->src_origin;
  const size_t *dst_origin = command->dst_origin;
  const size_t *region = command->region;
  void *src = m_src;
  void *dst = m_dst;
  cl_image_desc *s_image_info = &src_image->image_desc;
  cl_image_desc *d_image_info = &dst_image->image_desc;

  size_t s_image_depth =  s_image_info->image_depth;
  size_t s_image_height = s_image_info->image_height;
  size_t s_image_width =  s_image_info->image_width;
  size_t s_image_row_pitch = src_image->image_row_pitch;
  size_t s_image_slice_pitch = src_image->image_slice_pitch;
  size_t d_image_depth  = d_image_info->image_depth;
  size_t d_image_height = d_image_info->image_height;
  size_t d_image_width  = d_image_info->image_width;
  size_t d_image_row_pitch   = dst_image->image_row_pitch;
  size_t d_image_slice_pitch = dst_image->image_slice_pitch;
  size_t elem_size = dst_image->image_elem_size;

  if ((d_image_info->image_type == CL_MEM_OBJECT_IMAGE3D)
      || (d_image_info->image_type == CL_MEM_OBJECT_IMAGE2D_ARRAY)) {
    size_t zmax = dst_origin[2] + region[2];
    size_t ymax = dst_origin[1] + region[1];
    for(size_t zd=dst_origin[2], zs=src_origin[2]; zd<zmax; ++zd, ++zs) {
      for(size_t yd=dst_origin[1], ys=src_origin[1]; yd<ymax; ++yd, ++ys) {
        size_t soff, doff;
        doff = zd*d_image_slice_pitch + yd*d_image_row_pitch + dst_origin[0]*elem_size;
        soff = zs*s_image_slice_pitch + ys*s_image_row_pitch + src_origin[0]*elem_size;
        memcpy((char*)dst+doff, (char*)src+soff, region[0]*elem_size);
      }
    }
  }
  else if (d_image_info->image_type == CL_MEM_OBJECT_IMAGE2D) {
    size_t ymax = dst_origin[1] + region[1];
    for(size_t yd=dst_origin[1], ys=src_origin[1]; yd<ymax; ++yd, ++ys) {
      size_t soff, doff;
      doff = yd*d_image_row_pitch + dst_origin[0]*elem_size;
      soff = ys*s_image_row_pitch + src_origin[0]*elem_size;
      memcpy((char*)dst+doff, (char*)src+soff, region[0]*elem_size);
    }
  }
  else if (d_image_info->image_type == CL_MEM_OBJECT_IMAGE1D_ARRAY) {
    size_t ymax = dst_origin[1] + region[1];
    for(size_t yd=dst_origin[1], ys=src_origin[1]; yd<ymax; ++yd, ++ys) {
      size_t soff, doff;
      doff = yd*d_image_slice_pitch + dst_origin[0]*elem_size;
      soff = ys*s_image_slice_pitch + src_origin[0]*elem_size;
      memcpy((char*)dst+doff, (char*)src+soff, region[0]*elem_size);
    }
  }
  else if (d_image_info->image_type == CL_MEM_OBJECT_IMAGE1D) {
    size_t soff, doff;
    doff = dst_origin[0]*elem_size;
    soff = src_origin[0]*elem_size;
    memcpy((char*)dst+doff, (char*)src+soff, region[0]*elem_size);
    int *data = (int*)((char*)dst+doff);
  }
  else {
  }

  mem_dst->ClearLatest(this);
  command->event->Complete();
}

void x86CLDevice::CopyImageToBuffer(CLCommand* command) {
  CLMem* mem_src = command->mem_src;
  CLMem* mem_dst = command->mem_dst;

  CheckBuffer(mem_src);
  CheckBuffer(mem_dst);

  pthread_mutex_lock(&mem_src->mutex_dev_specific);
  void* m_src = mem_src->dev_specific[this];
  pthread_mutex_unlock(&mem_src->mutex_dev_specific);

  pthread_mutex_lock(&mem_src->mutex_dev_specific);
  void* m_dst = mem_dst->dev_specific[this];
  pthread_mutex_unlock(&mem_src->mutex_dev_specific);

  CLMem *image = mem_src;
  const size_t *origin = command->src_origin;
  const size_t *region = command->region;
  size_t offset = command->off_dst;
  void *src = m_src;
  void *dst = m_dst;

  size_t image_depth = image->image_desc.image_depth;
  size_t image_height = image->image_desc.image_height;
  size_t image_width = image->image_desc.image_width;
  size_t image_row_pitch = image->image_row_pitch;
  size_t image_slice_pitch = image->image_slice_pitch;
  size_t elem_size = image->image_elem_size;
  cl_image_desc *image_info = &image->image_desc;

  if ((image_info->image_type == CL_MEM_OBJECT_IMAGE3D)
      || (image_info->image_type == CL_MEM_OBJECT_IMAGE2D_ARRAY)) {
    size_t zmax = origin[2] + region[2];
    size_t ymax = origin[1] + region[1];
    for(size_t z=origin[2], zd=0; z<zmax; ++z, ++zd) {
      for(size_t y=origin[1], yd=0; y<ymax; ++y, ++yd) {
        size_t soff;
        soff = z*image_slice_pitch + y*image_row_pitch + origin[0]*elem_size;
        memcpy((char*)dst+offset, (char*)src+soff, region[0]*elem_size);
        offset += region[0]*elem_size;
      }
    }
  }
  else if (image_info->image_type == CL_MEM_OBJECT_IMAGE2D) {
    size_t ymax = origin[1] + region[1];
    for(size_t y=origin[1], yd=0; y<ymax; ++y, ++yd) {
      size_t soff;
      soff = y*image_row_pitch + origin[0]*elem_size;
      memcpy((char*)dst+offset, (char*)src+soff, region[0]*elem_size);
      offset += region[0]*elem_size;
    }
  }
  else if (image_info->image_type == CL_MEM_OBJECT_IMAGE1D_ARRAY) {
    size_t ymax = origin[1] + region[1];
    for(size_t y=origin[1], yd=0; y<ymax; ++y, ++yd) {
      size_t soff;
      soff = y*image_slice_pitch + origin[0]*elem_size;
      memcpy((char*)dst+offset, (char*)src+soff, region[0]*elem_size);
      offset += region[0]*elem_size;
    }
  }
  else if (image_info->image_type == CL_MEM_OBJECT_IMAGE1D) {
    memcpy((char*)dst+offset, (char*)src+origin[0]*elem_size, region[0]*elem_size);
  }
  else {
  }

  command->event->Complete();
}

void x86CLDevice::CopyBufferToImage(CLCommand* command) {
  CLMem* mem_src = command->mem_src;
  CLMem* mem_dst = (CLMem*)command->mem_dst;

  CheckBuffer(mem_src);
  CheckBuffer(mem_dst);

  pthread_mutex_lock(&mem_src->mutex_dev_specific);
  void* m_src = mem_src->dev_specific[this];
  pthread_mutex_unlock(&mem_src->mutex_dev_specific);

  pthread_mutex_lock(&mem_src->mutex_dev_specific);
  void* m_dst = mem_dst->dev_specific[this];
  pthread_mutex_unlock(&mem_src->mutex_dev_specific);

  CLMem *image = mem_dst;
  const size_t *origin = command->dst_origin;
  const size_t *region = command->region;
  size_t offset = command->off_src;
  void * src = m_src;
  void * dst = m_dst;

  size_t image_depth = image->image_desc.image_depth;
  size_t image_height = image->image_desc.image_height;
  size_t image_width = image->image_desc.image_width;
  size_t image_row_pitch = image->image_row_pitch;
  size_t image_slice_pitch = image->image_slice_pitch;
  size_t elem_size = image->image_elem_size;
  cl_image_desc *image_info = &image->image_desc;

  if ((image_info->image_type == CL_MEM_OBJECT_IMAGE3D)
      || (image_info->image_type == CL_MEM_OBJECT_IMAGE2D_ARRAY)) {
    size_t zmax = origin[2] + region[2];
    size_t ymax = origin[1] + region[1];
    for(size_t z=origin[2], zd=0; z<zmax; ++z, ++zd) {
      for(size_t y=origin[1], yd=0; y<ymax; ++y, ++yd) {
        size_t doff;
        doff = z*image_slice_pitch+ y*image_row_pitch+ origin[0]*elem_size;
        memcpy((char*)dst+doff, (char*)src+offset, region[0]*elem_size);
        offset += region[0]*elem_size;
      }
    }
  }
  else if (image_info->image_type == CL_MEM_OBJECT_IMAGE2D) {
    size_t ymax = origin[1] + region[1];
    for(size_t y=origin[1], yd=0; y<ymax; ++y, ++yd) {
      size_t doff;
      doff = y*image_row_pitch + origin[0]*elem_size;
      memcpy((char*)dst+doff, (char*)src+offset, region[0]*elem_size);
      offset += region[0]*elem_size;
    }
  }
  else if (image_info->image_type == CL_MEM_OBJECT_IMAGE1D_ARRAY) {
    size_t ymax = origin[1] + region[1];
    for(size_t y=origin[1], yd=0; y<ymax; ++y, ++yd) {
      size_t doff;
      doff = y*image_slice_pitch + origin[0]*elem_size;
      memcpy((char*)dst+doff, (char*)src+offset, region[0]*elem_size);
      offset += region[0]*elem_size;
    }
  }
  else if (image_info->image_type == CL_MEM_OBJECT_IMAGE1D) {
    memcpy((char*)dst+origin[0]*elem_size, (char*)src+offset, region[0]*elem_size);
  }
  else {
  }

  command->event->Complete();
}

void x86CLDevice::CopyBufferRect(CLCommand* command) {
  CLMem* mem_src = command->mem_src;
  CLMem* mem_dst = command->mem_dst;

  CheckBuffer(mem_src);
  CheckBuffer(mem_dst);

  pthread_mutex_lock(&mem_src->mutex_dev_specific);
  void* m_src = mem_src->dev_specific[this];
  pthread_mutex_unlock(&mem_src->mutex_dev_specific);
  if (m_src == NULL) SNUCL_ERROR("SRC_MEM[%d] IS NULL", mem_src->id);

  pthread_mutex_lock(&mem_dst->mutex_dev_specific);
  void* m_dst = mem_dst->dev_specific[this];
  pthread_mutex_unlock(&mem_dst->mutex_dev_specific);
  if (m_dst == NULL) SNUCL_ERROR("DST_MEM[%d] IS NULL", mem_dst->id);

  BufferRectCommon(command->src_origin,
            command->dst_origin,
            command->region,
            command->src_row_pitch,
            command->src_slice_pitch,
            command->dst_row_pitch,
            command->dst_slice_pitch,
            m_src,
            m_dst);

  mem_dst->ClearLatest(this);
  command->event->Complete();
}

void x86CLDevice::FillBuffer(CLCommand* command) {
  CLMem* buffer = command->buffer;
  CheckBuffer(buffer);

  pthread_mutex_lock(&buffer->mutex_dev_specific);
  char* dst = (char*)buffer->dev_specific[this];
  pthread_mutex_unlock(&buffer->mutex_dev_specific);

  size_t max = command->offset + command->size;
  size_t index = command->offset;
  while(index < max) {
    memcpy(dst+index, command->pattern, command->pattern_size);
    index += command->pattern_size;
  }

  free(command->pattern);

  command->event->Complete();
}

void x86CLDevice::FillImage(CLCommand* command) {
  CLMem* buffer = command->buffer;
  CheckBuffer(buffer);

  pthread_mutex_lock(&buffer->mutex_dev_specific);
  char* dst = (char*)buffer->dev_specific[this];
  pthread_mutex_unlock(&buffer->mutex_dev_specific);

  CLMem *image = buffer;
  const size_t *origin = command->dst_origin;
  const size_t *region = command->region;
  void * fill_color = command->pattern;
  cl_image_desc *image_info = &image->image_desc;

  size_t image_depth = image->image_desc.image_depth;
  size_t image_height = image->image_desc.image_height;
  size_t image_width = image->image_desc.image_width;
  size_t image_row_pitch = image->image_row_pitch;
  size_t image_slice_pitch = image->image_slice_pitch;
  size_t elem_size = image->image_elem_size;
  cl_image_format *image_format = &image->image_format;
  cl_channel_type image_type = image->image_format.image_channel_data_type;

  void * packed_color = malloc(command->pattern_size);

  if(image_type==CL_SIGNED_INT8||image_type==CL_SIGNED_INT16||image_type==CL_SIGNED_INT32) {
    PackImagePixel((int*)fill_color, image_format, packed_color);
  }
  else if(image_type==CL_UNSIGNED_INT8||image_type==CL_UNSIGNED_INT16||image_type==CL_UNSIGNED_INT32) {
    PackImagePixel((unsigned int*)fill_color, image_format, packed_color);
  }
  else {
    PackImagePixel((float*)fill_color, image_format, packed_color);
  }

  if ((image_info->image_type == CL_MEM_OBJECT_IMAGE3D)
      || (image_info->image_type == CL_MEM_OBJECT_IMAGE2D_ARRAY)) {
    size_t zmax = origin[2] + region[2];
    size_t ymax = origin[1] + region[1];
    size_t xmax = origin[0] + region[0];
    for(size_t z = origin[2]; z < zmax; ++z) {
      for(size_t y = origin[1]; y < ymax; ++y) {
        for(size_t x = origin[0]; x < xmax; ++x) {
          size_t off;
          off = z*image_slice_pitch + y*image_row_pitch+ x*elem_size;
          memcpy((char*)dst+off, (char*)packed_color, elem_size);
        }
      }
    }
  }
  else if (image_info->image_type == CL_MEM_OBJECT_IMAGE2D) {
    size_t ymax = origin[1] + region[1];
    size_t xmax = origin[0] + region[0];
    for(size_t y = origin[1]; y < ymax; ++y) {
      for(size_t x = origin[0]; x < xmax; ++x) {
        size_t off;
        off = y*image_row_pitch + x*elem_size;
        memcpy((char*)dst+off, (char*)packed_color, elem_size);
      }
    }
  }
  else if (image_info->image_type == CL_MEM_OBJECT_IMAGE1D_ARRAY) {
    size_t ymax = origin[1] + region[1];
    size_t xmax = origin[0] + region[0];
    for(size_t y = origin[1]; y < ymax; ++y) {
      for(size_t x = origin[0]; x < xmax; ++x) {
        size_t off;
        off = y*image_slice_pitch + x*elem_size;
        memcpy((char*)dst+off, (char*)packed_color, elem_size);
      }
    }
  }
  else if (image_info->image_type == CL_MEM_OBJECT_IMAGE1D) {
    size_t xmax = origin[0] + region[0];
    for(size_t x = origin[0]; x < xmax; ++x) {
      size_t off;
      off = x*elem_size;
      memcpy((char*)dst+off, (char*)packed_color, elem_size);
    }
  }
  else {
  }

  free(command->pattern);
  free(packed_color);

  command->event->Complete();
}

void x86CLDevice::MapBuffer(CLCommand* command) {
  CLMem* buffer = command->buffer;
  CheckBuffer(buffer);

  pthread_mutex_lock(&buffer->mutex_dev_specific);
  char* src = (char*)buffer->dev_specific[this];
  pthread_mutex_unlock(&buffer->mutex_dev_specific);

  if(!(command->map_flags & CL_MAP_WRITE_INVALIDATE_REGION)) {
    memcpy(command->ptr, src + command->off_src, command->cb);
  }

  command->event->Complete();
}

void x86CLDevice::MapImage(CLCommand* command) {
  CLMem* buffer = command->buffer;
  CheckBuffer(buffer);

  pthread_mutex_lock(&buffer->mutex_dev_specific);
  char* src = (char*)buffer->dev_specific[this];
  pthread_mutex_unlock(&buffer->mutex_dev_specific);

  if(!(command->map_flags & CL_MAP_WRITE_INVALIDATE_REGION)) {
    ReadImageCommon((CLMem*)buffer,
               command->src_origin,
               command->region,
               command->dst_row_pitch,
               command->dst_slice_pitch,
               src,
               command->ptr);
  }

  command->event->Complete();
}

void x86CLDevice::UnmapMemObject(CLCommand* command) {
  CLMem* buffer = command->buffer;

  pthread_mutex_lock(&buffer->mutex_dev_specific);
  char* dst = (char*)buffer->dev_specific[this];
  pthread_mutex_unlock(&buffer->mutex_dev_specific);

  if (buffer->is_image) {
    CLMem* img = (CLMem*)buffer;
    WriteImageCommon(img,
                img->mapped_info[command->ptr].origin,
                img->mapped_info[command->ptr].region,
                img->mapped_info[command->ptr].row_pitch,
                img->mapped_info[command->ptr].slice_pitch,
                command->ptr,
                dst);
  }
  else {
    memcpy(dst+command->off_dst, command->ptr, command->cb);
  }

  if(!buffer->use_host)
    free(command->ptr);

  command->event->Complete();
}

void x86CLDevice::MigrateMemObjects(CLCommand * command) {
  cl_uint num_mem_objects = command->num_mem_objects;
  cl_mem* mem_objects = command->mem_objects;

  for(int i=0; i < num_mem_objects; ++i) {
    // Don't care about flags. It's almost free!
    CheckBuffer(mem_objects[i]->c_obj);
  }

  free(mem_objects);

  command->event->Complete();
}

void x86CLDevice::WriteBuffer(CLCommand* command) {
  CLMem* mem_dst = command->mem_dst;
  size_t off_dst = command->off_dst;
  size_t size = command->cb;
  void* ptr = command->ptr;
  
  CheckBuffer(mem_dst);

  pthread_mutex_lock(&mem_dst->mutex_dev_specific);
  void* m = mem_dst->dev_specific[this];
  pthread_mutex_unlock(&mem_dst->mutex_dev_specific);

	if( (void*) ((size_t) m + off_dst) != ptr )
		memcpy((void*) ((size_t) m + off_dst), ptr, size);

  mem_dst->ClearLatest(this);

  command->event->Complete();
}

void x86CLDevice::CopyBuffer(CLCommand* command) {
  CLMem* mem_src = command->mem_src;
  CLMem* mem_dst = command->mem_dst;
  size_t off_src = command->off_src;
  size_t off_dst = command->off_dst;
  size_t size = command->cb;

  CheckBuffer(mem_src);
  CheckBuffer(mem_dst);

  pthread_mutex_lock(&mem_src->mutex_dev_specific);
  void* m_src = mem_src->dev_specific[this];
  pthread_mutex_unlock(&mem_src->mutex_dev_specific);
  if (m_src == NULL) SNUCL_ERROR("SRC_MEM[%d] IS NULL", mem_src->id);

  pthread_mutex_lock(&mem_dst->mutex_dev_specific);
  void* m_dst = mem_dst->dev_specific[this];
  pthread_mutex_unlock(&mem_dst->mutex_dev_specific);
  if (m_dst == NULL) SNUCL_ERROR("DST_MEM[%d] IS NULL", mem_dst->id);

  memcpy((void*) ((size_t) m_dst + off_dst), (void*) ((size_t) m_src + off_src), size);

  mem_dst->ClearLatest(this);
  command->event->Complete();
}

void x86CLDevice::BuildProgram(CLCommand* command) {
	if(command->program->fromObj[this]) {
		LinkObjects(command);
	}
	else if(command->program->fromSource[this]) {
		CompileSource(command);
		if (command->program->buildStatus[this] != CL_BUILD_ERROR)
			LinkObjects(command);
	}
	else {
		BuildProgramFromBinary(command);
	}

	if (command->program->buildStatus[this] != CL_BUILD_ERROR)
		command->program->buildStatus[this] = CL_BUILD_SUCCESS;

	command->program->buildVersion++;

	if(command->program->buildCallbackStack.size() == 0)
		command->event->Complete();
	else {
		for (vector<ProgramCallback*>::iterator it = command->program->buildCallbackStack.begin(); it != command->program->buildCallbackStack.end(); ++it) {
			(*it)->pfn_notify(&command->program->st_obj, (*it)->user_data);
		}
	}
	if (command->program->ref_cnt == 0 && command->program->kernelObjs.size() == 0)
		clReleaseProgram(&command->program->st_obj);
}

void x86CLDevice::LinkObjects(CLCommand* command) {
  CLProgram* program = command->program;

  char* cmd;
  char kernel_dir[128];
  char kernel_file[128];
  char kernel_info_file[128];

  mkdir(SNUCL_BUILD_DIR, 0755);
  mkdir(SNUCL_CPU_BUILD_DIR, 0755);
  sprintf(kernel_dir, "%s/%s", SNUCL_CPU_BUILD_DIR, node_name);
  mkdir(kernel_dir, 0755);


	if (program->linkObjects[this].size() == 0) 
		program->linkObjects[this].push_back(program->compiledObject[this]);

	int num_files = program->linkObjects[this].size();
  char file_idx[11];

	cmd = (char*) malloc (sizeof(char)*(num_files*11 + 256));

  while (1) {
		GenFileIdx(file_idx);
    sprintf(kernel_file, "%s/__cl_kernel_%s.so", kernel_dir, file_idx);
    if (access(kernel_file, F_OK) == 0) continue;
    sprintf(kernel_file, "%s/__cl_kernel_%s.cpp", kernel_dir, file_idx);
    if (access(kernel_file, F_OK) == 0) continue;
    sprintf(kernel_file, "%s/__cl_kernel_%s.cl", kernel_dir, file_idx);
    if (access(kernel_file, F_OK) == 0) continue;
		break;
  }

  sprintf(kernel_file, "%s/__cl_kernel_%s.so", kernel_dir, file_idx);
  sprintf(kernel_info_file, "%s/__cl_kernel_info_%s.so", kernel_dir, file_idx);
	sprintf(cmd, "t-cpu-link.sh %s %d %d", file_idx, ncore, num_files);

	for (int i=0; i<num_files; i++) 
		sprintf(cmd, "%s %s", cmd, program->linkObjects[this][i]);

	int optLen = 0;
	if(program->options)
		optLen = strlen(program->options);

  if (optLen) {
    SNUCL_DEBUG("OPTIONS [%s]", program->options);
    sprintf(cmd, "%s %s", cmd, program->options);
  } else 
    SNUCL_DEBUG("NO OPTIONS [%s]", "");

  if (system(cmd) == -1) SNUCL_ERROR("BUILD ERROR PROGRAM[%lu]", program->id);


	void* p = dlopen(kernel_file, RTLD_NOW);
	if (p == NULL) SNUCL_ERROR("KERNEL SO [%s] DOES NOT OPEN : %s", kernel_file,dlerror());
  pthread_mutex_lock(&program->mutex_dev_specific);
  program->dev_specific[this] = (void*) p;
  pthread_mutex_unlock(&program->mutex_dev_specific);

	//set Binary information
	CreateBinary(file_idx, program);

	ReadKernelInfo(kernel_info_file, program);

	free(cmd);
}

void x86CLDevice::BuildProgramFromBinary(CLCommand* command) {
  CLProgram* program = command->program;

  char kernel_dir[128];
  char kernel_file[128];
  char kernel_info_file[128];

  mkdir(SNUCL_BUILD_DIR, 0755);
  mkdir(SNUCL_CPU_BUILD_DIR, 0755);
  sprintf(kernel_dir, "%s/%s", SNUCL_CPU_BUILD_DIR, node_name);
  mkdir(kernel_dir, 0755);
  char file_idx[11];
  while (1) {
		GenFileIdx(file_idx);
    sprintf(kernel_file, "%s/__cl_kernel_%s.so", kernel_dir, file_idx);
    if (access(kernel_file, F_OK) == 0) continue;
    sprintf(kernel_file, "%s/__cl_kernel_%s.cpp", kernel_dir, file_idx);
    if (access(kernel_file, F_OK) == 0) continue;
    sprintf(kernel_file, "%s/__cl_kernel_%s.cl", kernel_dir, file_idx);
    if (access(kernel_file, F_OK) == 0) continue;
		break; 
	}

	//read binary
	ReadBin(file_idx, program);

  sprintf(kernel_info_file, "%s/__cl_kernel_info_%s.so", kernel_dir, file_idx);
	ReadKernelInfo(kernel_info_file, program);
}

void x86CLDevice::LinkProgram(CLCommand* command) {
	LinkObjects(command);

	command->program->buildStatus[this] = CL_BUILD_SUCCESS;
	if(command->program->linkCallbackStack.size() == 0)
		command->event->Complete();
	else {
		for (vector<ProgramCallback*>::iterator it = command->program->linkCallbackStack.begin(); it != command->program->linkCallbackStack.end(); ++it) {
			(*it)->pfn_notify(&command->program->st_obj, (*it)->user_data);
		}
	}
	if (command->program->ref_cnt == 0 && command->program->kernelObjs.size() == 0)
		clReleaseProgram(&command->program->st_obj);
}

void x86CLDevice::CompileProgram(CLCommand* command) {
	CompileSource(command);

	command->program->buildStatus[this] = CL_BUILD_SUCCESS;
	if(command->program->compileCallbackStack.size() == 0)
		command->event->Complete();
	else {
		for (vector<ProgramCallback*>::iterator it = command->program->compileCallbackStack.begin(); it != command->program->compileCallbackStack.end(); ++it) {
			(*it)->pfn_notify(&command->program->st_obj, (*it)->user_data);
		}
	}
	if (command->program->ref_cnt == 0 && command->program->kernelObjs.size() == 0)
		clReleaseProgram(&command->program->st_obj);
}

void x86CLDevice::CompileSource(CLCommand* command) {
  CLProgram* program = command->program;

  char* cmd;
  char kernel_dir[128];
  char kernel_file[128];
  char kernel_info_file[128];

  mkdir(SNUCL_BUILD_DIR, 0755);
  mkdir(SNUCL_CPU_BUILD_DIR, 0755);
  sprintf(kernel_dir, "%s/%s", SNUCL_CPU_BUILD_DIR, node_name);
  mkdir(kernel_dir, 0755);
  char file_idx[11];

  while (1) {
		GenFileIdx(file_idx);
    sprintf(kernel_file, "%s/__cl_kernel_%s.so", kernel_dir, file_idx);
    if (access(kernel_file, F_OK) == 0) continue;
    sprintf(kernel_file, "%s/__cl_kernel_%s.cpp", kernel_dir, file_idx);
    if (access(kernel_file, F_OK) == 0) continue;
    sprintf(kernel_file, "%s/__cl_kernel_%s.cl", kernel_dir, file_idx);
    if (access(kernel_file, F_OK) == 0) continue;

		FILE *file = fopen(kernel_file, "a");
		fprintf(file, "%s", program->src);
		fclose(file);
		break;
  }

	int optLen = 0;
	if(program->options)
		optLen = strlen(program->options);

	cmd = (char*) malloc (sizeof(char)*(optLen + 256));

  if (optLen) {
    SNUCL_DEBUG("OPTIONS [%s]", program->options);
    sprintf(cmd, "t-cpu-compile.sh %s %s", file_idx, program->options);
  } else {
    SNUCL_DEBUG("NO OPTIONS [%s]", "");
    sprintf(cmd, "t-cpu-compile.sh %s", file_idx);
  }

  if (system(cmd) == -1) SNUCL_ERROR("BUILD ERROR PROGRAM[%lu]", program->id);

	free(cmd);

	if (!CheckCompileError(file_idx)) {
		program->buildStatus[this] = CL_BUILD_ERROR;
		return;
	}

  sprintf(kernel_info_file, "%s/__cl_kernel_info_%s.so", kernel_dir, file_idx);

	//set Compiled Binary information
	CreateCompiledBinary(file_idx, program);

	ReadKernelInfo(kernel_info_file, program);

	program->SetObj(this, file_idx);

	//set buildLog
  char log_file[128];
  sprintf(log_file, "%s/__cl_kernel_%s.log", kernel_dir, file_idx);
	CreateLog(log_file, program);
}
bool x86CLDevice::CheckCompileError(char* file_idx) {
  char kernel_dir[128];
  char kernel_file[128];
	char kernel_file_cpp[128];

  sprintf(kernel_dir, "%s/%s", SNUCL_CPU_BUILD_DIR, node_name);
	sprintf(kernel_file, "%s/__cl_kernel_%s.o", kernel_dir, file_idx);
	sprintf(kernel_file_cpp, "%s/__cl_kernel_%s.cpp", kernel_dir, file_idx);

	if (access(kernel_file_cpp, F_OK) != 0) return false;
	if (access(kernel_file, F_OK) != 0) return false;

	return true;
}

void x86CLDevice::AllocBuffer(CLMem* mem) {
  void* m;
  if(mem->is_sub) {
    CheckBuffer(mem->parent);
    pthread_mutex_lock(&mem->mutex_dev_specific);
    m = (char*)mem->parent->dev_specific[this] + mem->buf_create_info.origin;
    pthread_mutex_unlock(&mem->mutex_dev_specific);
  }
	else if(mem->use_host)
		m = mem->space_host;
	else
		m = memalign(4096, mem->size);

  pthread_mutex_lock(&mem->mutex_dev_specific);
  mem->dev_specific[this] = (void*) m;
  pthread_mutex_unlock(&mem->mutex_dev_specific);

  mem->ClearLatest(this);
}

void x86CLDevice::FreeBuffer(CLCommand* command) {
  CLMem* mem = command->mem_src;
  void* m = mem->dev_specific[this];

  if(!mem->is_sub && !mem->use_host)
    free(m);

  mem->ClearLatest(NULL);
  command->event->Complete();
}
void x86CLDevice::FreeProgramInfo(CLProgram* program) {
  dlclose(program->dev_specific[this]);
}

void x86CLDevice::ScheduleDynamic(CLWorkGroupAssignment **wga) {
  CLCommand* command = wga[0]->command;

  size_t wg_total = command->nwg[0] * command->nwg[1] * command->nwg[2];
  size_t wg_chunk = 1;
  size_t wg_start = 0;
  size_t wg_remain = wg_total;

  while (wg_remain != 0) {
    for (int i = 0; i < ncore; ++i) {
      if (compute_units[i]->work_to_do == false) {
        wga[i]->wg_id_start = wg_start;
        wg_chunk = SNUCL_IDIV(wg_remain, 2 * ncore);
        if (wg_chunk > wg_remain) wg_chunk = wg_remain;
        wga[i]->wg_id_end = wg_start + wg_chunk;
        wg_remain -= wg_chunk;
        wg_start += wg_chunk;
        compute_units[i]->Launch(wga[i]);
        if (wg_remain == 0) break;
      }
    }
  }

}

void x86CLDevice::ScheduleStatic(CLWorkGroupAssignment **wga) {
  CLCommand* command = wga[0]->command;

  size_t wg_total = command->nwg[0] * command->nwg[1] * command->nwg[2];
  size_t wg_chunk = SNUCL_IDIV(wg_total, ncore);
  size_t wg_start = 0;
  size_t wg_remain = wg_total;

  for (int i = 0; i < ncore; ++i) {
    if (wg_remain == 0) break;

    wga[i]->wg_id_start = i * wg_chunk;
    wga[i]->wg_id_end = wg_remain < wg_chunk ? wga[i]->wg_id_start + wg_remain : wga[i]->wg_id_start + wg_chunk;

    SNUCL_DEBUG("CORE[%d] WG[%lu~%lu]", i, wga[i]->wg_id_start, wga[i]->wg_id_end);

    wg_remain -= wga[i]->wg_id_end - wga[i]->wg_id_start;

    compute_units[i]->Launch(wga[i]);
  }
}

cl_int x86CLDevice::GetSupportedImageFormats(cl_mem_flags flags,
                                cl_mem_object_type image_type,
                                cl_uint num_entries,
                                cl_image_format *image_formats,
                                cl_uint *num_image_formats)
{
  int index = 0;
  cl_image_format tmp;
  const unsigned int NUM_ORDERS = 2;
  const unsigned int NUM_TYPES = 10;

  // Minimum supported image types. See 5.3.2.1
  int order_arr[NUM_ORDERS] = { CL_RGBA, CL_BGRA };
  int type_arr[NUM_TYPES] = { CL_UNORM_INT8,
                              CL_UNORM_INT16,
                              CL_SIGNED_INT8,
                              CL_SIGNED_INT16,
                              CL_SIGNED_INT32,
                              CL_UNSIGNED_INT8,
                              CL_UNSIGNED_INT16,
                              CL_UNSIGNED_INT32,
                              CL_HALF_FLOAT,
                              CL_FLOAT
                            };

  for (int i=0; i<NUM_ORDERS; ++i) {
    tmp.image_channel_order = order_arr[i];
    for (int j=0; j<NUM_TYPES; ++j) {
      if (index >= num_entries) {
        if (num_image_formats)
          *num_image_formats = index;
        return CL_SUCCESS;
      }
      tmp.image_channel_data_type = type_arr[j];
      if(image_formats)
        image_formats[index] = tmp;
      index++;
    }
  }

  if (num_image_formats)
    *num_image_formats = index;

  return CL_SUCCESS;
}

void x86CLDevice::CreateCompiledBinary(char* file_idx, CLProgram* program) {
  char kernel_dir[128];
  char kernel_file[128];
	char kernel_file_cpp[128];
  char kernel_info_file[128];

  sprintf(kernel_dir, "%s/%s", SNUCL_CPU_BUILD_DIR, node_name);
  sprintf(kernel_file, "%s/__cl_kernel_%s.o", kernel_dir, file_idx);
	sprintf(kernel_file_cpp, "%s/__cl_kernel_%s.cpp", kernel_dir, file_idx);
  sprintf(kernel_info_file, "%s/__cl_kernel_info_%s.so", kernel_dir, file_idx);

	FILE* fpK = fopen(kernel_file, "r");
	FILE* fpCpp = fopen(kernel_file_cpp, "r");
	FILE* fpI = fopen(kernel_info_file, "r");

	struct stat statBufK;
	struct stat statBufCpp;
	struct stat statBufI;

	if (fstat(fileno(fpK), &statBufK) < 0)
		SNUCL_FATAL("%s","fstat error");
	if (fstat(fileno(fpCpp), &statBufCpp) < 0)
		SNUCL_FATAL("%s","fstat error");
	if (fstat(fileno(fpI), &statBufI) < 0)
		SNUCL_FATAL("%s","fstat error");

	off_t sizeK = statBufK.st_size;
	off_t sizeCpp = statBufCpp.st_size;
	off_t sizeI = statBufI.st_size;
	int sizeKLen = (sizeK == 0)? 1 : (int)log10(sizeK) + 1;
	int sizeCppLen = (sizeCpp == 0)? 1 : (int)log10(sizeCpp) + 1;
	int sizeILen = (sizeI == 0)? 1 : (int)log10(sizeI) + 1;

	size_t prefixSize = 1 + sizeKLen + 1 + sizeCppLen + 1 + sizeILen + 1;
	size_t totalSize = prefixSize + sizeK + sizeCpp + sizeI;

	char binType = 'C'; // Compiled

	char* binary = (char*) malloc(totalSize);
	sprintf(binary, "%c%lu.%lu.%lu.", binType,sizeK,sizeCpp,sizeI);

	size_t ret;
	size_t size;

	//read kerenl
	size = sizeK;
	while(size) {
		ret = fread(&(binary[prefixSize]), 1, size, fpK);
		size -= ret;
	}

	//read cpp
	size = sizeCpp;
	while(size) {
		ret = fread(&(binary[prefixSize+sizeK]), 1, size, fpCpp);
		size -= ret;
	}

	//read kerenl info
	size = sizeI;
	while(size) {
		ret = fread(&(binary[prefixSize+sizeK+sizeCpp]), 1, size, fpI);
		size -= ret;
	}

	fclose(fpK);
	fclose(fpCpp);
	fclose(fpI);

	CLBinary* bin = new CLBinary(binary, totalSize, CL_PROGRAM_BINARY_TYPE_COMPILED_OBJECT);
	program->SetBin(this, bin, CL_PROGRAM_BINARY_TYPE_COMPILED_OBJECT);
}
void x86CLDevice::CreateBinary(char* file_idx, CLProgram* program) {
  char kernel_dir[128];
  char kernel_file[128];
  char kernel_file_cpp[128];
  char kernel_info_file[128];

	// binary file to read
  sprintf(kernel_dir, "%s/%s", SNUCL_CPU_BUILD_DIR, node_name);
  sprintf(kernel_file, "%s/__cl_kernel_%s.so", kernel_dir, file_idx);
  sprintf(kernel_info_file, "%s/__cl_kernel_info_%s.so", kernel_dir, file_idx);

	FILE* fpK = fopen(kernel_file, "r");
	FILE* fpI = fopen(kernel_info_file, "r");

	// linked objets info
	int numLinkedObjects = program->linkObjects[this].size();
	int numLinkedObjectsLen = (int)log10(numLinkedObjects) + 1;
	FILE** fpLOs = (FILE**)malloc(sizeof(FILE*)*numLinkedObjects);
	FILE** fpLOCpps = (FILE**)malloc(sizeof(FILE*)*numLinkedObjects);

	for (int i=0; i<numLinkedObjects; i++) {
		sprintf(kernel_file, "%s/__cl_kernel_%s.o", kernel_dir, program->linkObjects[this][i]);
		sprintf(kernel_file_cpp, "%s/__cl_kernel_%s.cpp", kernel_dir, program->linkObjects[this][i]);
		fpLOs[i] = fopen(kernel_file, "r");
		fpLOCpps[i] = fopen(kernel_file_cpp, "r");
	}

	struct stat statBufK;
	struct stat statBufI;
	struct stat* statBufLOs = (struct stat*)malloc(sizeof(struct stat)*numLinkedObjects);
	struct stat* statBufLOCpps = (struct stat*)malloc(sizeof(struct stat)*numLinkedObjects);

	if (fstat(fileno(fpK), &statBufK) < 0)
		SNUCL_FATAL("%s","fstat error");
	if (fstat(fileno(fpI), &statBufI) < 0)
		SNUCL_FATAL("%s","fstat error");

	for (int i=0; i<numLinkedObjects; i++) {
		if (fstat(fileno(fpLOs[i]), &statBufLOs[i]) < 0)
			SNUCL_FATAL("%s","fstat error");
		if (fstat(fileno(fpLOCpps[i]), &statBufLOCpps[i]) < 0)
			SNUCL_FATAL("%s","fstat error");
	}

	off_t sizeK = statBufK.st_size;
	off_t sizeI = statBufI.st_size;
	int sizeKLen = (sizeK == 0)? 1 : (int)log10(sizeK) + 1;
	int sizeILen = (sizeI == 0)? 1 : (int)log10(sizeI) + 1;

	off_t* sizeLOs = (off_t*)malloc(sizeof(off_t)*numLinkedObjects);
	off_t* sizeLOCpps = (off_t*)malloc(sizeof(off_t)*numLinkedObjects);
	int* sizeLOLens = (int*)malloc(sizeof(int)*numLinkedObjects);
	int* sizeLOCppLens = (int*)malloc(sizeof(int)*numLinkedObjects);

	for (int i=0; i<numLinkedObjects; i++) {
		sizeLOs[i] = statBufLOs[i].st_size;
		sizeLOCpps[i] = statBufLOCpps[i].st_size;
		sizeLOLens[i] = (sizeLOs[i] == 0)? 1 : (int)log10(sizeLOs[i]) + 1;
		sizeLOCppLens[i] = (sizeLOCpps[i] == 0)? 1 : (int)log10(sizeLOCpps[i]) + 1;
	}

	size_t prefixSize = 1 + sizeKLen + 1 + sizeILen + 1 + numLinkedObjectsLen + 1;
	for (int i=0; i<numLinkedObjects; i++) 
		prefixSize += sizeLOLens[i] + 1 + sizeLOCppLens[i] + 1;
	
	size_t totalSize = prefixSize + sizeK + sizeI;
	for (int i=0; i<numLinkedObjects; i++) 
		totalSize += sizeLOs[i] + sizeLOCpps[i];

	char binType = 'E'; //Executable

	char* binary = (char*) malloc(totalSize);
	sprintf(binary, "%c%lu.%lu.%d.", binType,sizeK,sizeI,numLinkedObjects);
	for (int i=0; i<numLinkedObjects; i++) 
		sprintf(binary, "%s%d.%d.", binary,sizeLOs[i],sizeLOCpps[i]);

	size_t ret;
	size_t size;

	//read kerenl
	size = sizeK;
	while(size) {
		ret = fread(&(binary[prefixSize]), 1, size, fpK);
		size -= ret;
	}

	//read kerenl info
	size = sizeI;
	while(size) {
		ret = fread(&(binary[prefixSize+sizeK]), 1, size, fpI);
		size -= ret;
	}

	//read objects info
	size_t startIdx = prefixSize+sizeK+sizeI;
	for (int i=0; i<numLinkedObjects; i++) {
		size = sizeLOs[i];
		while(size) {
			ret = fread(&(binary[startIdx]), 1, size, fpLOs[i]);
			size -= ret;
		}
		startIdx += sizeLOs[i];
		size = sizeLOCpps[i];
		while(size) {
			ret = fread(&(binary[startIdx]), 1, size, fpLOCpps[i]);
			size -= ret;
		}
		startIdx += sizeLOCpps[i];
	}

	fclose(fpK);
	fclose(fpI);
	for (int i=0; i<numLinkedObjects; i++) {
		fclose(fpLOs[i]);
		fclose(fpLOCpps[i]);
	}
	free(fpLOs);
	free(fpLOCpps);
	free(statBufLOs);
	free(statBufLOCpps);
	free(sizeLOs);
	free(sizeLOCpps);
	free(sizeLOLens);
	free(sizeLOCppLens);

	CLBinary* bin = new CLBinary(binary, totalSize, CL_PROGRAM_BINARY_TYPE_EXECUTABLE);
	program->SetBin(this, bin, CL_PROGRAM_BINARY_TYPE_EXECUTABLE);
}

void x86CLDevice::ReadBin(char* file_idx, CLProgram* program) {
	switch (program->bins[this]->type) {
		case CL_PROGRAM_BINARY_TYPE_COMPILED_OBJECT: 
			ReadBinToCompiledObjects(file_idx, program);
			break;
		case CL_PROGRAM_BINARY_TYPE_EXECUTABLE:
			ReadBinToExecutableBinary(file_idx, program);
			break;
		default:
			SNUCL_FATAL("%s", "Not Supported binary type");
			exit(-1);
	}
}

void x86CLDevice::ReadBinToCompiledObjects(char* file_idx, CLProgram* program) {
	char* binary = program->bins[this]->binary;
  char kernel_dir[128];
  char kernel_file[128];
	char kernel_file_cpp[128];
  char kernel_info_file[128];

  sprintf(kernel_dir, "%s/%s", SNUCL_CPU_BUILD_DIR, node_name);
	sprintf(kernel_file, "%s/__cl_kernel_%s.o", kernel_dir, file_idx);
	sprintf(kernel_file_cpp, "%s/__cl_kernel_%s.cpp", kernel_dir, file_idx);
  sprintf(kernel_info_file, "%s/__cl_kernel_info_%s.so", kernel_dir, file_idx);

	// object size
	int idx_start=1;
	int idx_end=1;
	while (binary[idx_end++] != '.');

	int temp_size = idx_end - 1 - idx_start;
	char* temp = (char*)malloc(temp_size+1);
	temp[temp_size] = '\0';
	strncpy(temp, &(binary[idx_start]), temp_size);

	off_t sizeK = atoi(temp);
	free(temp);

	// cpp size
	idx_start = idx_end;
	while (binary[idx_end++] != '.');

	temp_size = idx_end - 1 - idx_start;
	temp = (char*)malloc(temp_size+1);
	temp[temp_size] = '\0';
	strncpy(temp, &(binary[idx_start]), temp_size);

	int sizeCpp = atoi(temp);
	free(temp);

	// info size
	idx_start = idx_end;
	while (binary[idx_end++] != '.');

	temp_size = idx_end - 1 - idx_start;
	temp = (char*)malloc(temp_size+1);
	temp[temp_size] = '\0';
	strncpy(temp, &(binary[idx_start]), temp_size);

	off_t sizeI = atoi(temp);
	free(temp);

	FILE* obj = fopen(kernel_file,"wb");
	FILE* cpp = fopen(kernel_file_cpp,"wb");
	FILE* info = fopen(kernel_info_file,"wb");

	size_t size;
	size_t ret;
	size = sizeK;
	while(size) {
		ret = fwrite(&(binary[idx_end]), 1, size, obj);
		size -= ret;
	}

	size = sizeCpp;
	while(size) {
		ret = fwrite(&(binary[idx_end + sizeK]), 1, size, cpp);
		size -= ret;
	}

	size = sizeI;
	while(size) {
		ret = fwrite(&(binary[idx_end + sizeK + sizeCpp]), 1, size, info);
		size -= ret;
	}

	fclose(obj);
	fclose(cpp);
	fclose(info);

	program->SetObj(this, file_idx);
}
void x86CLDevice::ReadBinToExecutableBinary(char* file_idx, CLProgram* program) {
	char* binary = program->bins[this]->binary;
  char kernel_dir[128];
  char kernel_file[128];
  char kernel_file_o[128];
  char kernel_file_cpp[128];
  char kernel_info_file[128];

  sprintf(kernel_dir, "%s/%s", SNUCL_CPU_BUILD_DIR, node_name);
	sprintf(kernel_file, "%s/__cl_kernel_%s.so", kernel_dir, file_idx);
  sprintf(kernel_info_file, "%s/__cl_kernel_info_%s.so", kernel_dir, file_idx);

	// object size
	int idx_start=1;
	int idx_end=1;
	while (binary[idx_end++] != '.');

	int temp_size = idx_end - 1 - idx_start;
	char* temp = (char*)malloc(temp_size+1);
	temp[temp_size] = '\0';
	strncpy(temp, &(binary[idx_start]), temp_size);

	off_t sizeK = atoi(temp);
	free(temp);

	// info size
	idx_start = idx_end;
	while (binary[idx_end++] != '.');

	temp_size = idx_end - 1 - idx_start;
	temp = (char*)malloc(temp_size+1);
	temp[temp_size] = '\0';
	strncpy(temp, &(binary[idx_start]), temp_size);

	off_t sizeI = atoi(temp);
	free(temp);

	// linked objects 
	idx_start = idx_end;
	while (binary[idx_end++] != '.');

	temp_size = idx_end - 1 - idx_start;
	temp = (char*)malloc(temp_size+1);
	temp[temp_size] = '\0';
	strncpy(temp, &(binary[idx_start]), temp_size);

	int numLinkedObjects = atoi(temp);
	free(temp);

	off_t* sizeLOs = (off_t*)malloc(sizeof(off_t)*numLinkedObjects);
	off_t* sizeLOCpps = (off_t*)malloc(sizeof(off_t)*numLinkedObjects);

	for (int i=0; i<numLinkedObjects; i++) {
		idx_start = idx_end;
		while (binary[idx_end++] != '.');

		temp_size = idx_end - 1 - idx_start;
		temp = (char*)malloc(temp_size+1);
		temp[temp_size] = '\0';
		strncpy(temp, &(binary[idx_start]), temp_size);

		sizeLOs[i] = atoi(temp);
		free(temp);

		idx_start = idx_end;
		while (binary[idx_end++] != '.');

		temp_size = idx_end - 1 - idx_start;
		temp = (char*)malloc(temp_size+1);
		temp[temp_size] = '\0';
		strncpy(temp, &(binary[idx_start]), temp_size);

		sizeLOCpps[i] = atoi(temp);
		free(temp);
	}

	//

	FILE* obj = fopen(kernel_file,"wb");
	FILE* info = fopen(kernel_info_file,"wb");

	FILE** fpLOs = (FILE**)malloc(sizeof(FILE*)*numLinkedObjects);
	FILE** fpLOCpps = (FILE**)malloc(sizeof(FILE*)*numLinkedObjects);
	for (int i=0; i<numLinkedObjects; i++) {
		char* temp_file_idx = (char*)malloc(sizeof(char)*11);
		GenFileIdx(temp_file_idx);
		sprintf(kernel_file_o, "%s/__cl_kernel_%s.o", kernel_dir, temp_file_idx);
		sprintf(kernel_file_cpp, "%s/__cl_kernel_%s.cpp", kernel_dir, temp_file_idx);
		fpLOs[i] = fopen(kernel_file_o, "wb");
		fpLOCpps[i] = fopen(kernel_file_cpp, "wb");
		program->linkObjects[this].push_back(temp_file_idx);
	}

	size_t size;
	size_t ret;
	size = sizeK;
	while(size) {
		ret = fwrite(&(binary[idx_end]), 1, size, obj);
		size -= ret;
	}

	size = sizeI;
	while(size) {
		ret = fwrite(&(binary[idx_end + sizeK]), 1, size, info);
		size -= ret;
	}

	//read objects info
	size_t startIdx = idx_end+sizeK+sizeI;
	for (int i=0; i<numLinkedObjects; i++) {
		size = sizeLOs[i];
		while(size) {
			ret = fwrite(&(binary[startIdx]), 1, size, fpLOs[i]);
			size -= ret;
		}
		startIdx += sizeLOs[i];
		size = sizeLOCpps[i];
		while(size) {
			ret = fwrite(&(binary[startIdx]), 1, size, fpLOCpps[i]);
			size -= ret;
		}
		startIdx += sizeLOCpps[i];
	}

	fclose(obj);
	fclose(info);
	for (int i=0; i<numLinkedObjects; i++) {
		fclose(fpLOs[i]);
		fclose(fpLOCpps[i]);
	}
	free(fpLOs);
	free(fpLOCpps);
	free(sizeLOs);
	free(sizeLOCpps);

	void* p = dlopen(kernel_file, RTLD_NOW);
	if (p == NULL) SNUCL_ERROR("KERNEL SO [%s] DOES NOT OPEN : %s", kernel_file,dlerror());
	SNUCL_DEBUG("KERNEL FILE[%s]", kernel_file);

	pthread_mutex_lock(&program->mutex_dev_specific);
	program->dev_specific[this] = (void*) p;
	pthread_mutex_unlock(&program->mutex_dev_specific);
}

void x86CLDevice::CreateLog(char* logName, CLProgram* program) {
	char* log;
	struct stat statBuf;
	FILE* fp = fopen(logName, "r");
	size_t size;
	size_t rsize;
	size_t ret;

	if (fstat(fileno(fp), &statBuf) < 0)
		SNUCL_FATAL("%s","fstat error");

	rsize = statBuf.st_size;

	if(rsize) {
		log = (char*)malloc(rsize+1);

		size = rsize;
		while(size) {
			ret = fread(log, 1, size, fp);
			size -= ret;
		}
		log[rsize] = '\0';
	}
	else {
		log = (char*)malloc(18+1);
		strcpy(log, "build successfully"); 
		log[18] = '\0';
	}


	fclose(fp);
	program->buildLog[this] = log;
}

void x86CLDevice::ReadKernelInfo(char* kernel_info_file, CLProgram* program) {
  char *err;
	int temp_idx;

  void* kernel_info_handle = dlopen(kernel_info_file, RTLD_NOW);
  if (kernel_info_handle == NULL) SNUCL_ERROR("KERNEL INFO SO [%s] DOES NOT EXIST", kernel_info_file);

	//_cl_kernel_num
  unsigned int* _cl_kernel_num = (unsigned int*)dlsym(kernel_info_handle, "_cl_kernel_num");
  program->num_kernels = (size_t)*_cl_kernel_num;

	//_cl_kernel_names
	char** _cl_kernel_names = (char**)dlsym(kernel_info_handle, "_cl_kernel_names");
  if ((err = dlerror()) != NULL) SNUCL_ERROR("SYMBOL [%s] DOES NOT EXIST", "_cl_kernel_names");
	program->kernel_names = (char**)malloc(sizeof(char*)*program->num_kernels);
  for (uint i = 0; i < program->num_kernels; ++i) {
		program->kernel_names[i] = (char*)calloc((strlen(_cl_kernel_names[i])+1),sizeof(char));
    strcpy(program->kernel_names[i], _cl_kernel_names[i]);
    SNUCL_DEBUG("PROGRAM[%d] KERNEL[%u] [%s]", program->id, i, program->kernel_names[i]);
  }

	//_cl_kernel_num_args
	unsigned int* _cl_kernel_num_args = (unsigned int*)dlsym(kernel_info_handle, "_cl_kernel_num_args");
	program->kernel_num_args = (cl_uint*)malloc(sizeof(cl_uint)*program->num_kernels);
	for (uint i=0; i<program->num_kernels; i++) 
		program->kernel_num_args[i] = (cl_uint)_cl_kernel_num_args[i];

	//_cl_kernel_attributes
	char** _cl_kernel_attributes = (char**)dlsym(kernel_info_handle, "_cl_kernel_attributes");
	program->kernel_attributes = (char**)malloc(sizeof(char*)*program->num_kernels);
	for (uint i=0; i<program->num_kernels; i++) {
		program->kernel_attributes[i] = (char*)calloc((strlen(_cl_kernel_attributes[i])+1),sizeof(char));
		strcpy(program->kernel_attributes[i], _cl_kernel_attributes[i]);
	}

	//_cl_kernel_work_group_size_hint
	unsigned int (*_cl_kernel_work_group_size_hint)[3] = (unsigned int (*)[3])dlsym(kernel_info_handle, "_cl_kernel_work_group_size_hint");
	program->kernel_work_group_size_hint = (size_t**)malloc(sizeof(size_t*)*program->num_kernels);
	for (uint i=0; i<program->num_kernels; i++) {
		program->kernel_work_group_size_hint[i] = (size_t*)malloc(sizeof(size_t)*3);
		program->kernel_work_group_size_hint[i][0] = (size_t)_cl_kernel_work_group_size_hint[i][0];
		program->kernel_work_group_size_hint[i][1] = (size_t)_cl_kernel_work_group_size_hint[i][1];
		program->kernel_work_group_size_hint[i][2] = (size_t)_cl_kernel_work_group_size_hint[i][2];
	}
	//_cl_kernel_reqd_work_group_size
	unsigned int (*_cl_kernel_reqd_work_group_size)[3] = (unsigned int (*)[3])dlsym(kernel_info_handle, "_cl_kernel_reqd_work_group_size");
	program->kernel_reqd_work_group_size = (size_t**)malloc(sizeof(size_t*)*program->num_kernels);
	for (uint i=0; i<program->num_kernels; i++) {
		program->kernel_reqd_work_group_size[i] = (size_t*)malloc(sizeof(size_t)*3);
		program->kernel_reqd_work_group_size[i][0] = (size_t)_cl_kernel_reqd_work_group_size[i][0];
		program->kernel_reqd_work_group_size[i][1] = (size_t)_cl_kernel_reqd_work_group_size[i][1];
		program->kernel_reqd_work_group_size[i][2] = (size_t)_cl_kernel_reqd_work_group_size[i][2];
	}
	//_cl_kernel_local_mem_size
	unsigned long long* _cl_kernel_local_mem_size = (unsigned long long*)dlsym(kernel_info_handle, "_cl_kernel_local_mem_size");
	program->kernel_local_mem_size = (cl_ulong*)malloc(sizeof(cl_ulong)*program->num_kernels);
	for (uint i=0; i<program->num_kernels; i++) {
		program->kernel_local_mem_size[i] = (cl_ulong)_cl_kernel_local_mem_size[i];
	}
	//_cl_kernel_private_mem_size
	unsigned long long* _cl_kernel_private_mem_size = (unsigned long long*)dlsym(kernel_info_handle, "_cl_kernel_private_mem_size");
	program->kernel_private_mem_size = (cl_ulong*)malloc(sizeof(cl_ulong)*program->num_kernels);
	for (uint i=0; i<program->num_kernels; i++) {
		program->kernel_private_mem_size[i] = (cl_ulong)_cl_kernel_private_mem_size[i];
	}
	//_cl_kernel_arg_address_qualifier
	char** _cl_kernel_arg_address_qualifier = (char**)dlsym(kernel_info_handle, "_cl_kernel_arg_address_qualifier");
	program->kernel_arg_address_qualifier = (cl_kernel_arg_address_qualifier**)malloc(sizeof(cl_kernel_arg_address_qualifier*)*program->num_kernels);
	for (uint i=0; i<program->num_kernels; i++) {
		program->kernel_arg_address_qualifier[i] = (cl_kernel_arg_address_qualifier*)malloc(sizeof(cl_kernel_arg_address_qualifier)*program->kernel_num_args[i]);
		for (uint j=0; j<program->kernel_num_args[i]; j++) {
			char item = _cl_kernel_arg_address_qualifier[i][j];
			switch (item) {
				case '0': program->kernel_arg_address_qualifier[i][j] = CL_KERNEL_ARG_ADDRESS_PRIVATE; break;
				case '1': program->kernel_arg_address_qualifier[i][j] = CL_KERNEL_ARG_ADDRESS_LOCAL; break;
				case '2': program->kernel_arg_address_qualifier[i][j] = CL_KERNEL_ARG_ADDRESS_CONSTANT; break;
				case '3': program->kernel_arg_address_qualifier[i][j] = CL_KERNEL_ARG_ADDRESS_GLOBAL; break;
				default: SNUCL_ERROR("%s", "Wrong kernel arg address quailifier"); break;
			}
		}
	}
	//_cl_kernel_arg_access_qualifier
	char** _cl_kernel_arg_access_qualifier = (char**)dlsym(kernel_info_handle, "_cl_kernel_arg_access_qualifier");
	program->kernel_arg_access_qualifier = (cl_kernel_arg_access_qualifier**)malloc(sizeof(cl_kernel_arg_access_qualifier*)*program->num_kernels);
	for (uint i=0; i<program->num_kernels; i++) {
		program->kernel_arg_access_qualifier[i] = (cl_kernel_arg_access_qualifier*)malloc(sizeof(cl_kernel_arg_access_qualifier)*program->kernel_num_args[i]);
		for (uint j=0; j<program->kernel_num_args[i]; j++) {
			char item = _cl_kernel_arg_access_qualifier[i][j];
			switch (item) {
				case '0': program->kernel_arg_access_qualifier[i][j] = CL_KERNEL_ARG_ACCESS_NONE; break;
				case '1': program->kernel_arg_access_qualifier[i][j] = CL_KERNEL_ARG_ACCESS_READ_WRITE; break;
				case '2': program->kernel_arg_access_qualifier[i][j] = CL_KERNEL_ARG_ACCESS_WRITE_ONLY; break;
				case '3': program->kernel_arg_access_qualifier[i][j] = CL_KERNEL_ARG_ACCESS_READ_ONLY; break;
				default: SNUCL_ERROR("%s", "Wrong kernel arg access quailifier"); break;
			}
		}
	}
	//_cl_kernel_arg_type_name
	char** _cl_kernel_arg_type_name = (char**)dlsym(kernel_info_handle, "_cl_kernel_arg_type_name");
	temp_idx = 0;
	program->kernel_arg_type_name = (char***)malloc(sizeof(char*)*program->num_kernels);
	for (uint i=0; i<program->num_kernels; i++) {
		program->kernel_arg_type_name[i] = (char**)malloc(sizeof(char*)*program->kernel_num_args[i]);
		for (uint j=0; j<program->kernel_num_args[i]; j++) {
			program->kernel_arg_type_name[i][j] = (char*)calloc((strlen(_cl_kernel_arg_type_name[temp_idx])+1),sizeof(char));
			strcpy(program->kernel_arg_type_name[i][j], _cl_kernel_arg_type_name[temp_idx++]);
		}
	}
	//_cl_kernel_arg_type_qualifier
	unsigned int* _cl_kernel_arg_type_qualifier = (unsigned int*)dlsym(kernel_info_handle, "_cl_kernel_arg_type_qualifier");
	temp_idx = 0;
	program->kernel_arg_type_qualifier = (cl_kernel_arg_type_qualifier**)malloc(sizeof(cl_kernel_arg_type_qualifier*)*program->num_kernels);
	for (uint i=0; i<program->num_kernels; i++) {
		program->kernel_arg_type_qualifier[i] = (cl_kernel_arg_type_qualifier*)malloc(sizeof(cl_kernel_arg_type_qualifier)*program->kernel_num_args[i]);
		for (uint j=0; j<program->kernel_num_args[i]; j++) {
			program->kernel_arg_type_qualifier[i][j] = (cl_kernel_arg_type_qualifier)_cl_kernel_arg_type_qualifier[temp_idx++];
		}
	}
	//_cl_kernel_arg_name
	char** _cl_kernel_arg_name = (char**)dlsym(kernel_info_handle, "_cl_kernel_arg_name");
	temp_idx = 0;
	program->kernel_arg_name = (char***)malloc(sizeof(char**)*program->num_kernels);
	for (uint i=0; i<program->num_kernels; i++) {
		program->kernel_arg_name[i] = (char**)malloc(sizeof(char*)*program->kernel_num_args[i]);
		for (uint j=0; j<program->kernel_num_args[i]; j++) {
			program->kernel_arg_name[i][j] = (char*)calloc((strlen(_cl_kernel_arg_name[temp_idx])+1),sizeof(char));
			strcpy(program->kernel_arg_name[i][j], _cl_kernel_arg_name[temp_idx++]);
		}
	}

	//preferred_work_group_size_multiple
	program->preferred_work_group_size_multiple = 64;

	//isBuiltInKernel
	program->isBuiltInKernel = (bool*)malloc(sizeof(bool)*program->num_kernels);
	memset(program->isBuiltInKernel, 0, sizeof(bool)*program->num_kernels);

  if (dlclose(kernel_info_handle) != 0) SNUCL_ERROR("CANNOT CLOSE KERNEL INFO SO [%s]", kernel_info_file);
}

void x86CLDevice::GenFileIdx(char* file_idx) {
	file_idx[0] = rand()%10 + '0';
	file_idx[1] = rand()%10 + '0';
	file_idx[2] = rand()%10 + '0';
	file_idx[3] = rand()%26 + 'a';
	file_idx[4] = rand()%26 + 'A';
	file_idx[5] = rand()%26 + 'a';
	file_idx[6] = rand()%10 + '0';
	file_idx[7] = rand()%10 + '0';
	file_idx[8] = rand()%26 + 'A';
	file_idx[9] = rand()%10 + '0';
	file_idx[10] = '\0';
}

#define SWIZZLE_VEC(vec, type)                  \
  if (format->image_channel_order == CL_BGRA) { \
    type tmp;                                   \
    tmp = vec[0];                               \
    vec[0] = vec[2];                            \
    vec[2] = tmp;                               \
  }

#define MAKE_HEX_FLOAT(x,y,z)  ((float)ldexp( (float)(y), z))

int x86CLDevice::RoundToEven(float val) {
  if(val >= -(float)INT_MIN) return INT_MAX;
  if(val <= (float)INT_MIN)  return INT_MIN;

  if(fabsf(val) < MAKE_HEX_FLOAT(0x1.0p23f, 0x1L, 23)) {
    static const float magic[2] = { MAKE_HEX_FLOAT(0x1.0p23f, 0x1L, 23), MAKE_HEX_FLOAT(-0x1.0p23f, -0x1L, 23) };
    float magicVal = magic[val<0.0f];
    val += magicVal;
    val -= magicVal;
  }
  return (int)val;
}

cl_ushort x86CLDevice::Float2Half_rte(float val) {
  union { uint32_t u; float f; } fv, ft;
  fv.f = val;

  uint16_t sign     = (fv.u >> 16) & 0x8000;
  uint16_t exponent = (fv.u >> 23) & 0xFF;
  uint32_t mantissa = fv.u & 0x007FFFFF;

  fv.u = fv.u & 0x7FFFFFFF;

  if (exponent == 0xFF)
    return (mantissa == 0) ? (sign|0x7C00) : (sign|0x7E00|(mantissa>>13));

  if (fv.f >= 0x1.FFEp15f) return (sign|0x7C00);

  if (fv.f <= 0x1.0p-25f) return sign;

  if (fv.f < 0x1.8p-24f) return (sign|1);

  if (fv.f < 0x1.0p-14f) {
    fv.f = fv.f * 0x1.0p-125f;
    return (sign | fv.u);
  }

  ft.f = val * 0x1.0p13f;
  ft.u = ft.u & 0x7F800000;
  ft.f = ((fv.f + ft.f) - ft.f) * 0x1.0p-112f;
  return sign | (uint16_t)(ft.u >> 13);
}

#define SATURATE( v, min, max ) ( v < min ? min : ( v > max ? max : v ) )
void x86CLDevice::PackImagePixel(unsigned int *src, const cl_image_format *format, void *dst) {
  SWIZZLE_VEC(src, unsigned);
  const int CHANNELS = 4; // CL_RGBA, CL_BGRA
  if (format->image_channel_data_type == CL_UNSIGNED_INT8) {
    unsigned char *p = (unsigned char*)dst;
    for (unsigned int i=0; i<CHANNELS; i++) {
      p[i] = (unsigned char)SATURATE(src[i], 0, 255);
    }
  }
  else if (format->image_channel_data_type == CL_UNSIGNED_INT16) {
    unsigned short *p = (unsigned short *)dst;
    for (unsigned int i=0; i<CHANNELS; i++) {
      p[i] = (unsigned short)SATURATE(src[i], 0, 65535);
    }
  }
  else if (format->image_channel_data_type == CL_UNSIGNED_INT32) {
    unsigned int *p = (unsigned int *)dst;
    for (unsigned int i=0; i<CHANNELS; i++) {
      p[i] = (unsigned int)src[i];
    }
  }
  else {
    // Nothing.
  }
}

void x86CLDevice::PackImagePixel(int *src, const cl_image_format *format, void *dst) {
  SWIZZLE_VEC(src, int);
  const int CHANNELS = 4; // CL_RGBA, CL_BGRA
  if (format->image_channel_data_type == CL_SIGNED_INT8) {
    char *p = (char*)dst;
    for (unsigned int i=0; i<CHANNELS; i++) {
      p[i] = (char)SATURATE(src[i], -128, 127);
    }
  }
  else if (format->image_channel_data_type == CL_SIGNED_INT16) {
    short *p = (short *)dst;
    for (unsigned int i=0; i<CHANNELS; i++) {
      p[i] = (short)SATURATE(src[i], -32768, 32767);
    }
  }
  else if (format->image_channel_data_type == CL_SIGNED_INT32) {
    int *p = (int *)dst;
    for (unsigned int i=0; i<CHANNELS; i++) {
      p[i] = (int)src[i];
    }
  }
  else {
    // Nothing.
  }
}

#define NORMALIZE( v, max ) ( v < 0 ? 0 : ( v > 1.f ? max : RoundToEven( v * max ) ) )
#define NORMALIZE_UNROUNDED( v, max ) ( v < 0 ? 0 : ( v > 1.f ? max :  v * max ) )
#define NORMALIZE_SIGNED( v, min, max ) ( v  < -1.0f ? min : ( v > 1.f ? max : RoundToEven( v * max ) ) )
#define NORMALIZE_SIGNED_UNROUNDED( v, min, max ) ( v  < -1.0f ? min : ( v > 1.f ? max : v * max ) )
#define CONVERT_INT( v, min, max, max_val)  ( v < min ? min : ( v > max ? max_val : RoundToEven( v ) ) )
#define CONVERT_UINT( v, max, max_val)  ( v < 0 ? 0 : ( v > max ? max_val : RoundToEven( v ) ) )
void x86CLDevice::PackImagePixel(float *src, const cl_image_format *format, void *dst) {
  SWIZZLE_VEC(src, float);
  const int CHANNELS = 4; // CL_RGBA, CL_BGRA

  if (format->image_channel_data_type == CL_HALF_FLOAT) {
    cl_ushort *p = (cl_ushort *)dst;
    for (unsigned int i=0; i<CHANNELS; i++) {
      p[i] = Float2Half_rte(src[i]);
    }
  }
  else if (format->image_channel_data_type == CL_FLOAT) {
    cl_float *p = (cl_float*)dst;
    for (unsigned int i=0; i<CHANNELS; i++) {
      p[i] = src[i];
    }
  }
  else if (format->image_channel_data_type == CL_UNORM_INT8) {
    cl_uchar *p = (cl_uchar*)dst;
    for (unsigned int i=0; i<CHANNELS; i++) {
      p[i] = (unsigned char)NORMALIZE(src[i], 255.f);
    }
  }
  else if (format->image_channel_data_type == CL_UNORM_INT16) {
    cl_ushort *p = (cl_ushort*)dst;
    for (unsigned int i=0; i<CHANNELS; i++) {
      p[i] = (unsigned short)NORMALIZE(src[i], 65535.f);
    }
  }
  else if (format->image_channel_data_type == CL_SIGNED_INT8) {
    cl_char *p = (cl_char*)dst;
    for (unsigned int i=0; i<CHANNELS; i++) {
      p[i] = (char)CONVERT_INT(src[i], -127.0f, 127.f, 127);
    }
  }
  else if (format->image_channel_data_type == CL_SIGNED_INT16) {
    cl_short *p = (cl_short*)dst;
    for (unsigned int i=0; i<CHANNELS; i++) {
      p[i] = (short)CONVERT_INT(src[i], -32767.0f, 32767.f, 32767);
    }
  }
  else if (format->image_channel_data_type == CL_SIGNED_INT32) {
    cl_int *p = (cl_int*)dst;
    for (unsigned int i=0; i<CHANNELS; i++) {
      p[i] = (int)CONVERT_INT(src[i], 
          MAKE_HEX_FLOAT(-0x1.0p31f, -1, 31), MAKE_HEX_FLOAT(0x1.fffffep30f, 0x1fffffe, 30-23), CL_INT_MAX);
    }
  }
  else if (format->image_channel_data_type == CL_UNSIGNED_INT8) {
    cl_uchar *p = (cl_uchar*)dst;
    for (unsigned int i=0; i<CHANNELS; i++) {
      p[i] = (unsigned char)CONVERT_UINT(src[i], 255.f, CL_UCHAR_MAX);
    }
  }
  else if (format->image_channel_data_type == CL_UNSIGNED_INT16) {
    cl_ushort *p = (cl_ushort*)dst;
    for (unsigned int i=0; i<CHANNELS; i++) {
      p[i] = (unsigned short)CONVERT_UINT(src[i], 32767.f, CL_USHRT_MAX);
    }
  }
  else if (format->image_channel_data_type == CL_UNSIGNED_INT32) {
    cl_uint *p = (cl_uint*)dst;
    for (unsigned int i=0; i<CHANNELS; i++) {
      p[i] = (unsigned int)CONVERT_UINT(src[i], 
          MAKE_HEX_FLOAT(0x1.fffffep31f, 0x1fffffe, 31-23), CL_UINT_MAX);
    }
  }
  else {
    // Nothing.
  }
}
