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

#include <arch/legacy/LegacyDevice.h>
#include <CLAPI_real.h>

SNUCL_DEBUG_HEADER("LegacyDevice");

#define MAX_PLATFORM  8
#define MAX_DEVICE    8

int LegacyCLDevice::CreateDevices(vector<CLDevice*>* devices, cl_device_type type, const char* _platform_name) {
  cl_int err;
  cl_platform_id platforms[MAX_PLATFORM];
  cl_platform_id platform = NULL;
  cl_uint num_platforms;
  err = __real_clGetPlatformIDs(0, NULL, &num_platforms);
  SNUCL_ECHECK(err);
  SNUCL_DEBUG("NUM_PLATFORMS [%u]", num_platforms);
  err = __real_clGetPlatformIDs(num_platforms, platforms, NULL);
  SNUCL_ECHECK(err);
  char platform_name[32];
  for (int i = 0; i < num_platforms; ++i) {
    err = __real_clGetPlatformInfo(platforms[i], CL_PLATFORM_VENDOR, sizeof(platform_name), platform_name, NULL);
    SNUCL_DEBUG("PLATFORM[%d] [%s]", i, platform_name);
    if (strcmp(_platform_name, platform_name) == 0) {
      SNUCL_DEBUG("Legacy PLATFORM [%d]", i);
      platform = platforms[i];
      break;
    }
  }

  if (platform == NULL) {
    SNUCL_INFO("%u Legacy GPUs are detected.", 0);
    return 0;
  }

  cl_uint num_devices;
  err = __real_clGetDeviceIDs(platform, type, 0, NULL, &num_devices);
  SNUCL_INFO("%u Legacy GPUs are detected.", num_devices);

  cl_device_id _devices[MAX_DEVICE];

  err = __real_clGetDeviceIDs(platform, type, num_devices, _devices, NULL); 
  for (uint i = 0; i < num_devices; ++i) {
    fprintf(stderr, "%s(%d) :  _devices[%d] = %p\n", "LegacyCLDevice::CreateDevices", __LINE__, i, _devices[i]);
    devices->push_back(new LegacyCLDevice(_devices[i], type));
  }

  return (int) num_devices;
}

LegacyCLDevice::LegacyCLDevice(cl_device_id dev, cl_device_type type) : CLDevice(type) {
  this->dev = dev;

  worker = new CLDeviceWorker(this);
}

LegacyCLDevice::~LegacyCLDevice() {
  delete worker;
}

void LegacyCLDevice::Init() {
  cl_int err;

  ctx = __real_clCreateContext(0, 1, &dev, NULL, NULL, &err);
  SNUCL_ECHECK(err);
  cmq = __real_clCreateCommandQueue(ctx, dev, 0, &err);
  SNUCL_ECHECK(err);
}

void LegacyCLDevice::InitInfo() {
  cl_int err;
  err = __real_clGetDeviceInfo(dev, CL_DEVICE_VENDOR_ID, sizeof(cl_uint), &vendor_id, NULL);
  SNUCL_ECHECK(err);
  err = __real_clGetDeviceInfo(dev, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint), &max_compute_units, NULL);
  SNUCL_ECHECK(err);
  err = __real_clGetDeviceInfo(dev, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(cl_uint), &max_work_item_dimensions, NULL);
  SNUCL_ECHECK(err);
  err = __real_clGetDeviceInfo(dev, CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(max_work_item_sizes), &max_work_item_sizes, NULL);
  SNUCL_ECHECK(err);
  err = __real_clGetDeviceInfo(dev, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &max_work_group_size, NULL);
  SNUCL_ECHECK(err);
  err = __real_clGetDeviceInfo(dev, CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR, sizeof(cl_uint), &preferred_vector_width_char, NULL);
  SNUCL_ECHECK(err);
  err = __real_clGetDeviceInfo(dev, CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT, sizeof(cl_uint), &preferred_vector_width_short, NULL);
  SNUCL_ECHECK(err);
  err = __real_clGetDeviceInfo(dev, CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT, sizeof(cl_uint), &preferred_vector_width_int, NULL);
  SNUCL_ECHECK(err);
  err = __real_clGetDeviceInfo(dev, CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG, sizeof(cl_uint), &preferred_vector_width_long, NULL);
  SNUCL_ECHECK(err);
  err = __real_clGetDeviceInfo(dev, CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT, sizeof(cl_uint), &preferred_vector_width_float, NULL);
  SNUCL_ECHECK(err);
  err = __real_clGetDeviceInfo(dev, CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE, sizeof(cl_uint), &preferred_vector_width_double, NULL);
  SNUCL_ECHECK(err);
  err = __real_clGetDeviceInfo(dev, CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF, sizeof(cl_uint), &preferred_vector_width_half, NULL);
  SNUCL_ECHECK(err);
  err = __real_clGetDeviceInfo(dev, CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR, sizeof(cl_uint), &native_vector_width_char, NULL);
  SNUCL_ECHECK(err);
  err = __real_clGetDeviceInfo(dev, CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT, sizeof(cl_uint), &native_vector_width_short, NULL);
  SNUCL_ECHECK(err);
  err = __real_clGetDeviceInfo(dev, CL_DEVICE_NATIVE_VECTOR_WIDTH_INT, sizeof(cl_uint), &native_vector_width_int, NULL);
  SNUCL_ECHECK(err);
  err = __real_clGetDeviceInfo(dev, CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG, sizeof(cl_uint), &native_vector_width_long, NULL);
  SNUCL_ECHECK(err);
  err = __real_clGetDeviceInfo(dev, CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT, sizeof(cl_uint), &native_vector_width_float, NULL);
  SNUCL_ECHECK(err);
  err = __real_clGetDeviceInfo(dev, CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE, sizeof(cl_uint), &native_vector_width_double, NULL);
  SNUCL_ECHECK(err);
  err = __real_clGetDeviceInfo(dev, CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF, sizeof(cl_uint), &native_vector_width_half, NULL);
  SNUCL_ECHECK(err);
  err = __real_clGetDeviceInfo(dev, CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(cl_uint), &max_clock_frequency, NULL);
  SNUCL_ECHECK(err);
  err = __real_clGetDeviceInfo(dev, CL_DEVICE_ADDRESS_BITS, sizeof(cl_uint), &address_bits, NULL);
  SNUCL_ECHECK(err);
  err = __real_clGetDeviceInfo(dev, CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(cl_ulong), &max_mem_alloc_size, NULL);
  SNUCL_ECHECK(err);
  err = __real_clGetDeviceInfo(dev, CL_DEVICE_IMAGE_SUPPORT, sizeof(cl_bool), &image_support, NULL);
  SNUCL_ECHECK(err);
  err = __real_clGetDeviceInfo(dev, CL_DEVICE_MAX_READ_IMAGE_ARGS, sizeof(cl_uint), &max_read_image_args, NULL);
  SNUCL_ECHECK(err);
  err = __real_clGetDeviceInfo(dev, CL_DEVICE_MAX_WRITE_IMAGE_ARGS, sizeof(cl_uint), &max_write_image_args, NULL);
  SNUCL_ECHECK(err);
  err = __real_clGetDeviceInfo(dev, CL_DEVICE_IMAGE2D_MAX_WIDTH, sizeof(size_t), &image2d_max_width, NULL);
  SNUCL_ECHECK(err);
  err = __real_clGetDeviceInfo(dev, CL_DEVICE_IMAGE2D_MAX_HEIGHT, sizeof(size_t), &image2d_max_height, NULL);
  SNUCL_ECHECK(err);
  err = __real_clGetDeviceInfo(dev, CL_DEVICE_IMAGE3D_MAX_WIDTH, sizeof(size_t), &image3d_max_width, NULL);
  SNUCL_ECHECK(err);
  err = __real_clGetDeviceInfo(dev, CL_DEVICE_IMAGE3D_MAX_HEIGHT, sizeof(size_t), &image3d_max_height, NULL);
  SNUCL_ECHECK(err);
  err = __real_clGetDeviceInfo(dev, CL_DEVICE_IMAGE3D_MAX_DEPTH, sizeof(size_t), &image3d_max_depth, NULL);
  SNUCL_ECHECK(err);
  //err = __real_clGetDeviceInfo(dev, CL_DEVICE_IMAGE_MAX_BUFFER_SIZE, sizeof(size_t), &image_max_buffer_size, NULL);
  SNUCL_ECHECK(err);
  //err = __real_clGetDeviceInfo(dev, CL_DEVICE_IMAGE_MAX_ARRAY_SIZE, sizeof(size_t), &image_max_array_size, NULL);
  SNUCL_ECHECK(err);
  err = __real_clGetDeviceInfo(dev, CL_DEVICE_MAX_SAMPLERS, sizeof(cl_uint), &max_samplers, NULL);
  SNUCL_ECHECK(err);
  //err = __real_clGetDeviceInfo(dev, CL_DEVICE_MAX_PARAMETER_SIZE, sizeof(size_t), &max_parameter_size, NULL);
  max_parameter_size = 1024;
  SNUCL_ECHECK(err);
  err = __real_clGetDeviceInfo(dev, CL_DEVICE_MEM_BASE_ADDR_ALIGN, sizeof(cl_uint), &mem_base_addr_align, NULL);
  SNUCL_ECHECK(err);
  err = __real_clGetDeviceInfo(dev, CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE, sizeof(cl_uint), &min_data_type_align_size, NULL);
  SNUCL_ECHECK(err);
  err = __real_clGetDeviceInfo(dev, CL_DEVICE_SINGLE_FP_CONFIG, sizeof(cl_device_fp_config), &single_fp_config, NULL);
  SNUCL_ECHECK(err);
  err = __real_clGetDeviceInfo(dev, CL_DEVICE_DOUBLE_FP_CONFIG, sizeof(cl_device_fp_config), &double_fp_config, NULL);
  SNUCL_ECHECK(err);
  err = __real_clGetDeviceInfo(dev, CL_DEVICE_GLOBAL_MEM_CACHE_TYPE, sizeof(cl_device_mem_cache_type), &global_mem_cache_type, NULL);
  SNUCL_ECHECK(err);
  err = __real_clGetDeviceInfo(dev, CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE, sizeof(cl_uint), &global_mem_cacheline_size, NULL);
  SNUCL_ECHECK(err);
  err = __real_clGetDeviceInfo(dev, CL_DEVICE_GLOBAL_MEM_CACHE_SIZE, sizeof(cl_ulong), &global_mem_cache_size, NULL);
  SNUCL_ECHECK(err);
  err = __real_clGetDeviceInfo(dev, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(cl_ulong), &global_mem_size, NULL);
  SNUCL_ECHECK(err);
  err = __real_clGetDeviceInfo(dev, CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, sizeof(cl_ulong), &max_constant_buffer_size, NULL);
  SNUCL_ECHECK(err);
  err = __real_clGetDeviceInfo(dev, CL_DEVICE_MAX_CONSTANT_ARGS, sizeof(cl_uint), &max_constant_args, NULL);
  SNUCL_ECHECK(err);
  err = __real_clGetDeviceInfo(dev, CL_DEVICE_LOCAL_MEM_TYPE, sizeof(cl_device_local_mem_type), &local_mem_type, NULL);
  SNUCL_ECHECK(err);
  err = __real_clGetDeviceInfo(dev, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(cl_ulong), &local_mem_size, NULL);
  SNUCL_ECHECK(err);
  err = __real_clGetDeviceInfo(dev, CL_DEVICE_ERROR_CORRECTION_SUPPORT, sizeof(cl_bool), &error_correction_support, NULL);
  SNUCL_ECHECK(err);
  err = __real_clGetDeviceInfo(dev, CL_DEVICE_HOST_UNIFIED_MEMORY, sizeof(cl_bool), &host_unified_memory, NULL);
  SNUCL_ECHECK(err);
  err = __real_clGetDeviceInfo(dev, CL_DEVICE_PROFILING_TIMER_RESOLUTION, sizeof(size_t), &profiling_timer_resolution, NULL);
  SNUCL_ECHECK(err);
  err = __real_clGetDeviceInfo(dev, CL_DEVICE_ENDIAN_LITTLE, sizeof(cl_bool), &endian_little, NULL);
  SNUCL_ECHECK(err);
  err = __real_clGetDeviceInfo(dev, CL_DEVICE_AVAILABLE, sizeof(cl_bool), &available, NULL);
  SNUCL_ECHECK(err);
  err = __real_clGetDeviceInfo(dev, CL_DEVICE_COMPILER_AVAILABLE, sizeof(cl_bool), &compiler_available, NULL);
  SNUCL_ECHECK(err);
  //err = __real_clGetDeviceInfo(dev, CL_DEVICE_LINKER_AVAILABLE, sizeof(cl_bool), &linker_available, NULL);
  SNUCL_ECHECK(err);
  err = __real_clGetDeviceInfo(dev, CL_DEVICE_EXECUTION_CAPABILITIES, sizeof(cl_device_exec_capabilities), &execution_capabilities, NULL);
  SNUCL_ECHECK(err);
  err = __real_clGetDeviceInfo(dev, CL_DEVICE_QUEUE_PROPERTIES, sizeof(cl_command_queue_properties), &queue_properties, NULL);
  SNUCL_ECHECK(err);
  //err = __real_clGetDeviceInfo(dev, CL_DEVICE_BUILT_IN_KERNELS, sizeof(built_in_kernels), &built_in_kernels, NULL);
  SNUCL_ECHECK(err);
  err = __real_clGetDeviceInfo(dev, CL_DEVICE_NAME, sizeof(name), &name, NULL);
  SNUCL_ECHECK(err);
  err = __real_clGetDeviceInfo(dev, CL_DEVICE_VENDOR, sizeof(vendor), &vendor, NULL);
  SNUCL_ECHECK(err);
  err = __real_clGetDeviceInfo(dev, CL_DRIVER_VERSION, sizeof(driver_version), &driver_version, NULL);
  SNUCL_ECHECK(err);
  err = __real_clGetDeviceInfo(dev, CL_DEVICE_PROFILE, sizeof(profile), &profile, NULL);
  SNUCL_ECHECK(err);
  err = __real_clGetDeviceInfo(dev, CL_DEVICE_VERSION, sizeof(device_version), &device_version, NULL);
  SNUCL_ECHECK(err);
  err = __real_clGetDeviceInfo(dev, CL_DEVICE_OPENCL_C_VERSION, sizeof(openclc_version), &openclc_version, NULL);
  SNUCL_ECHECK(err);
  err = __real_clGetDeviceInfo(dev, CL_DEVICE_EXTENSIONS, sizeof(device_extensions), &device_extensions, NULL);
  SNUCL_ECHECK(err);
  //err = __real_clGetDeviceInfo(dev, CL_DEVICE_PRINTF_BUFFER_SIZE, sizeof(size_t), &printf_buffer_size, NULL);
  SNUCL_ECHECK(err);
  //err = __real_clGetDeviceInfo(dev, CL_DEVICE_PREFERRED_INTEROP_USER_SYNC, sizeof(cl_bool), &preferred_interop_user_sync, NULL);
  SNUCL_ECHECK(err);
  //err = __real_clGetDeviceInfo(dev, CL_DEVICE_PARENT_DEVICE, sizeof(cl_device_id), &parent_device, NULL);
  SNUCL_ECHECK(err);
  //err = __real_clGetDeviceInfo(dev, CL_DEVICE_PARTITION_MAX_SUB_DEVICES, sizeof(cl_uint), &partition_max_sub_devices, NULL);
  SNUCL_ECHECK(err);
  //err = __real_clGetDeviceInfo(dev, CL_DEVICE_PARTITION_PROPERTIES, sizeof(partition_properties), &partition_properties, NULL);
  SNUCL_ECHECK(err);
  //err = __real_clGetDeviceInfo(dev, CL_DEVICE_PARTITION_AFFINITY_DOMAIN, sizeof(cl_device_affinity_domain), &affinity_domain, NULL);
  SNUCL_ECHECK(err);
  //err = __real_clGetDeviceInfo(dev, CL_DEVICE_PARTITION_TYPE, sizeof(partition_type), &partition_type, NULL);
  SNUCL_ECHECK(err);
}

void LegacyCLDevice::LaunchKernel(CLCommand* command) {
  cl_int err;

  CLKernel* kernel = command->kernel;
  CLProgram* program = command->kernel->program;

  pthread_mutex_lock(&kernel->mutex_dev_specific);
  int k_created = kernel->dev_specific.count(this);
  pthread_mutex_unlock(&kernel->mutex_dev_specific);

  cl_kernel k;

  if (k_created == 0) {
    pthread_mutex_lock(&program->mutex_dev_specific);
    cl_program p = (cl_program) program->dev_specific[this];
    pthread_mutex_unlock(&program->mutex_dev_specific);

    k = __real_clCreateKernel(p, kernel->name, &err);
    SNUCL_ECHECK(err);

    pthread_mutex_lock(&kernel->mutex_dev_specific);
    kernel->dev_specific[this] = (void*) k;
    pthread_mutex_unlock(&kernel->mutex_dev_specific);
  } else {
    pthread_mutex_lock(&kernel->mutex_dev_specific);
    k = (cl_kernel) kernel->dev_specific[this];
    pthread_mutex_unlock(&kernel->mutex_dev_specific);
  }

  map<cl_uint, CLKernelArg*>* args = command->kernel_args;
  for (map<cl_uint, CLKernelArg*>::const_iterator it = args->begin(); it != args->end(); ++it) {
    cl_uint index = it->first;
    CLKernelArg* arg = it->second;
    if (arg->mem) {
      CLMem* mem = arg->mem;
      CheckBuffer(mem);
      pthread_mutex_lock(&mem->mutex_dev_specific);
      cl_mem m = (cl_mem) mem->dev_specific[this];
      pthread_mutex_unlock(&mem->mutex_dev_specific);
      err = __real_clSetKernelArg(k, index, arg->size, (void*) &m);
    } else if (arg->sampler) {
      CLSampler* sampler = arg->sampler;
      pthread_mutex_lock(&sampler->mutex_dev_specific);
      int allocated = sampler->dev_specific.count(this);
      cl_sampler s = NULL;
      if (allocated == 0) {
        s = __real_clCreateSampler(ctx, sampler->normalized_coords, sampler->addressing_mode, sampler->filter_mode, &err);
      } else {
        s = (cl_sampler) sampler->dev_specific[this];
      }
      pthread_mutex_unlock(&sampler->mutex_dev_specific);
      SNUCL_ECHECK(err);
      err = __real_clSetKernelArg(k, index, sizeof(cl_sampler), &s);
      
    } else if (arg->local) {
      err = __real_clSetKernelArg(k, index, arg->size, NULL);
    } else {
      err = __real_clSetKernelArg(k, index, arg->size, (void*) arg->value);
    }
    SNUCL_ECHECK(err);
  }

  cl_event e;
  fprintf(stderr, "%d %s(%d) : cmq=%p, k=%p, workdim=%d, gwo[0]=%d, gws[0]=%d, lws[0]=%d, device=%p\n", getpid(), "LegacyCLDevice::LaunchKernel", __LINE__, cmq, k, command->work_dim, command->gwo[0], command->gws[0], command->lws[0], this);
  command->gwo[0] = 0; command->gwo[1] = 0; command->gwo[2] = 0;
  err = __real_clEnqueueNDRangeKernel(cmq, k, command->work_dim, command->gwo, command->gws, command->lws, 0, NULL, &e);
  fprintf(stderr, "%d %s(%d) : __real_clEnqueueNDRangeKernel done...\n", getpid(), "LegacyCLDevice::LaunchKernel", __LINE__);
  SNUCL_ECHECK(err);

  err = __real_clWaitForEvents(1, &e);
  SNUCL_ECHECK(err);

  for (map<cl_uint, CLKernelArg*>::const_iterator it = args->begin(); it != args->end(); ++it) {
    CLKernelArg* arg = it->second;
    CLMem* mem = arg->mem;
    if (!mem) continue;
    if (!(mem->flags & CL_MEM_READ_ONLY)) {
      mem->ClearLatest(this);
    }
  }

  command->event->Complete();
}

void LegacyCLDevice::ReadBuffer(CLCommand* command) {
  cl_int err;

  CLMem* mem = command->mem_src;
  CheckBuffer(mem);

  pthread_mutex_lock(&mem->mutex_dev_specific);
  cl_mem m = (cl_mem) mem->dev_specific[this];
  pthread_mutex_unlock(&mem->mutex_dev_specific);

  if (m == NULL) SNUCL_ERROR("MEM[%d] IS NULL", mem->id);

  fprintf(stderr, "%d %s(%d) : cmq=%p, m=%p, off_src=%d, cb=%d, device=%p\n", getpid(), "LegacyCLDevice::ReadBuffer", __LINE__, cmq, m, command->off_src, command->cb, this);
  /*for(int i=0; i<16; i++){
        *((int *)(command->ptr)+i) = -2;
	fprintf(stderr, "%s(%d) : command->ptr[%d] = %d\n", "LegacyCLDevice::ReadBuffer", __LINE__, i, *((int *)(command->ptr)+i));
  }*/
  //int *test_read = (int *)calloc(16, sizeof(int));
  err = __real_clEnqueueReadBuffer(cmq, m, CL_TRUE, command->off_src, command->cb, command->ptr, 0, NULL, NULL);
  //err = __real_clEnqueueReadBuffer(cmq, m, CL_TRUE, command->off_src, command->cb, test_read, 0, NULL, NULL);
  SNUCL_ECHECK(err);
  err = __real_clFlush(cmq);
  SNUCL_ECHECK(err);

  command->event->Complete();
  //fprintf(stderr, "%s(%d) : device = %p, memory = %p, command_q = %p\n", "LegacyCLDevice::ReadBuffer", __LINE__,this, m, cmq);
  //fprintf(stderr, "%s(%d) : __real_clEnqueueReadBuffer done", "LegacyCLDevice::ReadBuffer", __LINE__);
  //for(int i=0; i<16; i++)
	//fprintf(stderr, "%d %s(%d) : test_read[%d] = %d\n", getpid(), "LegacyCLDevice::ReadBuffer", __LINE__, i, test_read[i]);
}

void LegacyCLDevice::WriteBuffer(CLCommand* command) {
  cl_int err;

  CLMem* mem_dst = command->mem_dst;

  SNUCL_DEBUG("WRITE BUFFER MEM[%lu]", mem_dst->id);

  CheckBuffer(mem_dst);
  /*fprintf(stderr, "%s(%d) : offset = %d, cb = %d\n", "LegacyCLDevice::WriteBuffer", __LINE__, command->off_dst, command->cb);
  for(int i=0; i<16; i++)
	fprintf(stderr, "%s(%d) : command->ptr[%d] = %d\n", "LegacyCLDevice::WriteBuffer", __LINE__, i, *((int *)(command->ptr)+i));
  */
  //fprintf(stderr, "%s(%d) : Locking...\n", "LegacyCLDevice::WriteBuffer", __LINE__);
  pthread_mutex_lock(&mem_dst->mutex_dev_specific);
  cl_mem m = (cl_mem) mem_dst->dev_specific[this];
  pthread_mutex_unlock(&mem_dst->mutex_dev_specific);
  //fprintf(stderr, "%s(%d) : Unlocking...\n", "LegacyCLDevice::WriteBuffer", __LINE__);

  if (m == NULL) SNUCL_ERROR("MEM[%d] IS NULL", mem_dst->id);
  fprintf(stderr, "%d %s(%d) : cmq=%p, m=%p, off_dst=%d, cb=%d, device=%p\n", getpid(), "LegacyCLDevice::WriteBuffer", __LINE__, cmq, m, command->off_dst, command->cb, this);
  err = __real_clEnqueueWriteBuffer(cmq, m, CL_TRUE, command->off_dst, command->cb, (const void*) command->ptr, 0, NULL, NULL);
  err |= __real_clFlush(cmq);
  SNUCL_ECHECK(err);

  mem_dst->ClearLatest(this);

  command->event->Complete();
  err |= __real_clFlush(cmq);
  int *test_read = (int *)calloc(16, sizeof(int));
  fprintf(stderr, "%d %s(%d) : cmq=%p, m=%p, off_dst=%d, cb=%d, device=%p\n", getpid(), "LegacyCLDevice::WriteBuffer", __LINE__, cmq, m, command->off_dst, command->cb, this);
  err = __real_clEnqueueReadBuffer(cmq, m, CL_TRUE, command->off_dst, command->cb, (void*) test_read, 0, NULL, NULL);
  //fprintf(stderr, "%s(%d) : device = %p, memory = %p, command_queue = %p\n", "LegacyCLDevice::WriteBuffer", __LINE__, this, m, cmq);
  //for(int i=0; i<16; i++)
	//fprintf(stderr, "%d %s(%d) : test_read_after_write[%d] = %d\n", getpid(), "LegacyCLDevice::WriteBuffer", __LINE__, i, test_read[i]);
}

void LegacyCLDevice::CopyBuffer(CLCommand* command) {
  cl_int err;

  CLMem* mem_src = command->mem_src;
  CLMem* mem_dst = command->mem_dst;

  CheckBuffer(mem_src);
  CheckBuffer(mem_dst);

  pthread_mutex_lock(&mem_src->mutex_dev_specific);
  cl_mem m_src = (cl_mem) mem_src->dev_specific[this];
  pthread_mutex_unlock(&mem_src->mutex_dev_specific);
  if (m_src == NULL) SNUCL_ERROR("SRC_MEM[%d] IS NULL", mem_src->id);

  pthread_mutex_lock(&mem_dst->mutex_dev_specific);
  cl_mem m_dst = (cl_mem) mem_dst->dev_specific[this];
  pthread_mutex_unlock(&mem_dst->mutex_dev_specific);
  if (m_dst == NULL) SNUCL_ERROR("DST_MEM[%d] IS NULL", mem_dst->id);

  cl_event e;
  err = __real_clEnqueueCopyBuffer(cmq, m_src, m_dst, command->off_src, command->off_dst, command->cb, 0, NULL, &e);
  SNUCL_ECHECK(err);

  err = __real_clWaitForEvents(1, &e);
  SNUCL_ECHECK(err);

  mem_dst->ClearLatest(this);

  command->event->Complete();
}

void LegacyCLDevice::BuildProgram(CLCommand* command) {
  cl_int err;

  CLProgram* program = command->program;

  cl_program p = NULL;

  if (program->fromSource[this]) {
    size_t length = strlen(program->src);
    p = __real_clCreateProgramWithSource(ctx, 1, (const char**) &program->src, &length, &err);
    SNUCL_ECHECK(err);
    SNUCL_DEBUG("\n%s\n[%s]\n", program->src, program->options);
    err = __real_clBuildProgram(p, 1, &dev, (const char*) program->options, NULL, NULL);
    SNUCL_ECHECK(err);
  } else if (program->fromBinary[this]) {
    CLBinary* bin = program->bins[this];
    size_t binary_size = bin->size;
    unsigned char* binary = (unsigned char*) bin->binary;
    const unsigned char *buffers[ 1 ] = { binary };
    p = __real_clCreateProgramWithBinary(ctx, 1, &dev, &binary_size, buffers, NULL, &err);
    SNUCL_ECHECK(err);
    err = __real_clBuildProgram(p, 1, &dev, NULL, NULL, NULL);
    SNUCL_ECHECK(err);
  } else {
    SNUCL_ERROR("%s", "CANNOT REACH HERE");
  }

  pthread_mutex_lock(&program->mutex_dev_specific);
  program->dev_specific[this] = (void*) p;
  pthread_mutex_unlock(&program->mutex_dev_specific);

  ReadKernelInfo(program);

	if (command->program->buildStatus[this] != CL_BUILD_ERROR) {
		command->program->buildStatus[this] = CL_BUILD_SUCCESS;
  }

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

void LegacyCLDevice::ReadBufferRect(CLCommand* command) {
  cl_int err;

  CLMem* mem = command->buffer;
  CheckBuffer(mem);

  pthread_mutex_lock(&mem->mutex_dev_specific);
  cl_mem m = (cl_mem) mem->dev_specific[this];
  pthread_mutex_unlock(&mem->mutex_dev_specific);

  if (m == NULL) SNUCL_ERROR("MEM[%d] IS NULL", mem->id);

  SNUCL_DEBUG("SO[%lu,%lu,%lu] DO[%lu,%lu,%lu] R[%lu,%lu,%lu] P[%lu,%lu,%lu,%lu] PTR[%p]", command->src_origin[0], command->src_origin[1], command->src_origin[2], command->dst_origin[0], command->dst_origin[1], command->dst_origin[2], command->region[0], command->region[1], command->region[2], command->src_row_pitch, command->src_slice_pitch, command->dst_row_pitch, command->dst_slice_pitch, command->ptr);

  err = __real_clEnqueueReadBufferRect(cmq, m, CL_TRUE, command->src_origin, command->dst_origin, command->region, command->src_row_pitch, command->src_slice_pitch, command->dst_row_pitch, command->dst_slice_pitch, command->ptr, 0, NULL, NULL);
  err |= __real_clFlush(cmq);
  SNUCL_ECHECK(err);

  command->event->Complete();
}

void LegacyCLDevice::WriteBufferRect(CLCommand* command) {
  cl_int err;

  CLMem* mem = command->buffer;
  CheckBuffer(mem);

  pthread_mutex_lock(&mem->mutex_dev_specific);
  cl_mem m = (cl_mem) mem->dev_specific[this];
  pthread_mutex_unlock(&mem->mutex_dev_specific);

  if (m == NULL) SNUCL_ERROR("MEM[%d] IS NULL", mem->id);

  SNUCL_DEBUG("DO[%lu,%lu,%lu] SO[%lu,%lu,%lu] R[%lu,%lu,%lu] DP[%lu,%lu] SP[%lu,%lu]", command->dst_origin[0], command->dst_origin[1], command->dst_origin[2], command->src_origin[0], command->src_origin[1], command->src_origin[2], command->region[0], command->region[1], command->region[2], command->dst_row_pitch, command->dst_slice_pitch, command->src_row_pitch, command->src_slice_pitch);

  err = __real_clEnqueueWriteBufferRect(cmq, m, CL_TRUE, command->dst_origin, command->src_origin, command->region, command->dst_row_pitch, command->dst_slice_pitch, command->src_row_pitch, command->src_slice_pitch, command->ptr, 0, NULL, NULL);
  SNUCL_ECHECK(err);
  err = __real_clFlush(cmq);
  SNUCL_ECHECK(err);

  mem->ClearLatest(this);

  command->event->Complete();
}

void LegacyCLDevice::ReadImage(CLCommand* command) {
  cl_int err;

  CLMem* mem = command->mem_src;

  CheckBuffer(mem);

  pthread_mutex_lock(&mem->mutex_dev_specific);
  cl_mem m = (cl_mem) mem->dev_specific[this];
  pthread_mutex_unlock(&mem->mutex_dev_specific);

  if (m == NULL) SNUCL_ERROR("MEM[%d] IS NULL", mem->id);

  SNUCL_DEBUG("LEGACY READ IMAGE ORIGION[%lu, %lu, %lu] REGION[%lu, %lu, %lu] ROW[%lu] SLICE[%lu]", command->src_origin[0], command->src_origin[1], command->src_origin[2], command->region[0], command->region[1], command->region[2], command->dst_row_pitch, command->dst_slice_pitch);

  err = __real_clEnqueueReadImage(cmq, m, CL_TRUE, command->src_origin, command->region, command->dst_row_pitch, command->dst_slice_pitch, command->ptr, 0, NULL, NULL);
  err |= __real_clFlush(cmq);
  SNUCL_ECHECK(err);

  command->event->Complete();
}

void LegacyCLDevice::WriteImage(CLCommand* command) {
  cl_int err;

  CLMem* mem = command->mem_dst;
  CheckBuffer(mem);

  pthread_mutex_lock(&mem->mutex_dev_specific);
  cl_mem m = (cl_mem) mem->dev_specific[this];
  pthread_mutex_unlock(&mem->mutex_dev_specific);

  if (m == NULL) SNUCL_ERROR("MEM[%d] IS NULL", mem->id);

  SNUCL_DEBUG("LEGACY WRITE IMAGE ORIGION[%lu, %lu, %lu] REGION[%lu, %lu, %lu] ROW[%lu] SLICE[%lu]", command->dst_origin[0], command->dst_origin[1], command->dst_origin[2], command->region[0], command->region[1], command->region[2], command->src_row_pitch, command->src_slice_pitch);

  err = __real_clEnqueueWriteImage(cmq, m, CL_TRUE, command->dst_origin, command->region, command->src_row_pitch, command->src_slice_pitch, command->ptr, 0, NULL, NULL);
  err |= __real_clFlush(cmq);
  SNUCL_ECHECK(err);

  mem->ClearLatest(this);

  command->event->Complete();
}

void LegacyCLDevice::CopyImage(CLCommand* command) {
  cl_int err;

  CLMem* mem_src = command->mem_src;
  CLMem* mem_dst = command->mem_dst;

  CheckBuffer(mem_src);
  CheckBuffer(mem_dst);

  pthread_mutex_lock(&mem_src->mutex_dev_specific);
  cl_mem m_src = (cl_mem) mem_src->dev_specific[this];
  pthread_mutex_unlock(&mem_src->mutex_dev_specific);
  if (m_src == NULL) SNUCL_ERROR("SRC_MEM[%d] IS NULL", mem_src->id);

  pthread_mutex_lock(&mem_dst->mutex_dev_specific);
  cl_mem m_dst = (cl_mem) mem_dst->dev_specific[this];
  pthread_mutex_unlock(&mem_dst->mutex_dev_specific);
  if (m_dst == NULL) SNUCL_ERROR("DST_MEM[%d] IS NULL", mem_dst->id);

  cl_event e;
  err = __real_clEnqueueCopyImage(cmq, m_src, m_dst, command->src_origin, command->dst_origin, command->region, 0, NULL, &e);
  SNUCL_ECHECK(err);

  err = __real_clWaitForEvents(1, &e);
  SNUCL_ECHECK(err);

  mem_dst->ClearLatest(this);

  command->event->Complete();
}

void LegacyCLDevice::CopyImageToBuffer(CLCommand* command) {
  cl_int err;

  CLMem* mem_src = command->mem_src;
  CLMem* mem_dst = command->mem_dst;

  CheckBuffer(mem_src);
  CheckBuffer(mem_dst);

  pthread_mutex_lock(&mem_src->mutex_dev_specific);
  cl_mem m_src = (cl_mem) mem_src->dev_specific[this];
  pthread_mutex_unlock(&mem_src->mutex_dev_specific);
  if (m_src == NULL) SNUCL_ERROR("SRC_MEM[%d] IS NULL", mem_src->id);

  pthread_mutex_lock(&mem_dst->mutex_dev_specific);
  cl_mem m_dst = (cl_mem) mem_dst->dev_specific[this];
  pthread_mutex_unlock(&mem_dst->mutex_dev_specific);
  if (m_dst == NULL) SNUCL_ERROR("DST_MEM[%d] IS NULL", mem_dst->id);

  cl_event e;
  err = __real_clEnqueueCopyImageToBuffer(cmq, m_src, m_dst, command->src_origin, command->region, command->off_dst, 0, NULL, &e);
  SNUCL_ECHECK(err);

  err = __real_clWaitForEvents(1, &e);
  SNUCL_ECHECK(err);

  mem_dst->ClearLatest(this);

  command->event->Complete();
}

void LegacyCLDevice::CopyBufferToImage(CLCommand* command) {
  cl_int err;

  CLMem* mem_src = command->mem_src;
  CLMem* mem_dst = command->mem_dst;

  CheckBuffer(mem_src);
  CheckBuffer(mem_dst);

  pthread_mutex_lock(&mem_src->mutex_dev_specific);
  cl_mem m_src = (cl_mem) mem_src->dev_specific[this];
  pthread_mutex_unlock(&mem_src->mutex_dev_specific);
  if (m_src == NULL) SNUCL_ERROR("SRC_MEM[%d] IS NULL", mem_src->id);

  pthread_mutex_lock(&mem_dst->mutex_dev_specific);
  cl_mem m_dst = (cl_mem) mem_dst->dev_specific[this];
  pthread_mutex_unlock(&mem_dst->mutex_dev_specific);
  if (m_dst == NULL) SNUCL_ERROR("DST_MEM[%d] IS NULL", mem_dst->id);

  cl_event e;
  err = __real_clEnqueueCopyBufferToImage(cmq, m_src, m_dst, command->off_src, command->dst_origin, command->region, 0, NULL, &e);
  SNUCL_ECHECK(err);

  err = __real_clWaitForEvents(1, &e);
  SNUCL_ECHECK(err);

  mem_dst->ClearLatest(this);

  command->event->Complete();
}

void LegacyCLDevice::CopyBufferRect(CLCommand* command) {
  cl_int err;

  CLMem* mem_src = command->mem_src;
  CLMem* mem_dst = command->mem_dst;

  CheckBuffer(mem_src);
  CheckBuffer(mem_dst);

  pthread_mutex_lock(&mem_src->mutex_dev_specific);
  cl_mem m_src = (cl_mem) mem_src->dev_specific[this];
  pthread_mutex_unlock(&mem_src->mutex_dev_specific);
  if (m_src == NULL) SNUCL_ERROR("SRC_MEM[%d] IS NULL", mem_src->id);

  pthread_mutex_lock(&mem_dst->mutex_dev_specific);
  cl_mem m_dst = (cl_mem) mem_dst->dev_specific[this];
  pthread_mutex_unlock(&mem_dst->mutex_dev_specific);
  if (m_dst == NULL) SNUCL_ERROR("DST_MEM[%d] IS NULL", mem_dst->id);

  cl_event e;
  err = __real_clEnqueueCopyBufferRect(cmq, m_src, m_dst, command->src_origin, command->dst_origin, command->region, command->src_row_pitch, command->src_slice_pitch, command->dst_row_pitch, command->dst_slice_pitch, 0, NULL, &e);
  SNUCL_ECHECK(err);

  err = __real_clWaitForEvents(1, &e);
  SNUCL_ECHECK(err);

  mem_dst->ClearLatest(this);

  command->event->Complete();
}

void LegacyCLDevice::MapBuffer(CLCommand* command) {
  cl_int err;

  CLMem* mem = command->buffer;

  CheckBuffer(mem);

  pthread_mutex_lock(&mem->mutex_dev_specific);
  cl_mem m = (cl_mem) mem->dev_specific[this];
  pthread_mutex_unlock(&mem->mutex_dev_specific);

  if (m == NULL) SNUCL_ERROR("MEM[%d] IS NULL", mem->id);

  command->ptr = __real_clEnqueueMapBuffer(cmq, m, CL_TRUE, command->map_flags, command->off_src, command->cb, 0, NULL, NULL, &err);

  err |= __real_clFlush(cmq);
  SNUCL_ECHECK(err);

  command->event->Complete();
}

void LegacyCLDevice::MapImage(CLCommand* command) {
  cl_int err;

  CLMem* mem = command->buffer;

  CheckBuffer(mem);

  pthread_mutex_lock(&mem->mutex_dev_specific);
  cl_mem m = (cl_mem) mem->dev_specific[this];
  pthread_mutex_unlock(&mem->mutex_dev_specific);

  if (m == NULL) SNUCL_ERROR("MEM[%d] IS NULL", mem->id);

  command->ptr = __real_clEnqueueMapImage(cmq, m, CL_TRUE, command->map_flags, command->src_origin, command->region, &command->dst_row_pitch, &command->dst_slice_pitch, 0, NULL, NULL, &err);
  err |= __real_clFlush(cmq);
  SNUCL_ECHECK(err);

  command->event->Complete();
}

void LegacyCLDevice::UnmapMemObject(CLCommand* command) {
  cl_int err;

  CLMem* mem = command->buffer;

  pthread_mutex_lock(&mem->mutex_dev_specific);
  cl_mem m = (cl_mem) mem->dev_specific[this];
  pthread_mutex_unlock(&mem->mutex_dev_specific);

  if (m == NULL) SNUCL_ERROR("MEM[%d] IS NULL", mem->id);

  cl_event e;

  err = __real_clEnqueueUnmapMemObject(cmq, m, command->ptr, 0, NULL, &e);

  err = __real_clWaitForEvents(1, &e);
  SNUCL_ECHECK(err);

  command->event->Complete();
}

void LegacyCLDevice::AllocBuffer(CLMem* mem) {
  cl_int err;
  cl_mem m;
  if (!mem->is_image) {
    if (mem->is_sub) {
      CLMem* parent = mem->parent;
      CheckBuffer(parent);

      pthread_mutex_lock(&parent->mutex_dev_specific);
      cl_mem p = (cl_mem) parent->dev_specific[this];
      pthread_mutex_unlock(&parent->mutex_dev_specific);

      m = __real_clCreateSubBuffer(p, mem->sub_flags, mem->create_type, &mem->buf_create_info, &err);
      SNUCL_ECHECK(err);
    } else {
      void* ptr = mem->use_host ? mem->space_host : NULL;
      m = __real_clCreateBuffer(ctx, mem->flags, mem->size, ptr, &err);
      //fprintf(stderr, "%s(%d) :  m = %p\n", "LegacyCLDevice::AllocBuffer", __LINE__, m);
    }
  } else if (mem->image_desc.image_type == CL_MEM_OBJECT_IMAGE2D) {
    void* ptr = mem->use_host ? mem->space_host : NULL;
    m = __real_clCreateImage2D(ctx, mem->flags, &mem->image_format, mem->image_desc.image_width, mem->image_desc.image_height, mem->image_desc.image_row_pitch, ptr, &err);
  } else if (mem->image_desc.image_type == CL_MEM_OBJECT_IMAGE3D) {
    void* ptr = mem->use_host ? mem->space_host : NULL;
    m = __real_clCreateImage3D(ctx, mem->flags, &mem->image_format, mem->image_desc.image_width, mem->image_desc.image_height, mem->image_desc.image_depth, mem->image_desc.image_row_pitch, mem->image_desc.image_slice_pitch, ptr, &err);
  }
  SNUCL_ECHECK(err);

  pthread_mutex_lock(&mem->mutex_dev_specific);
  mem->dev_specific[this] = (void*) m;
  pthread_mutex_unlock(&mem->mutex_dev_specific);

  mem->ClearLatest(this);
}

void LegacyCLDevice::FreeBuffer(CLCommand* command) {
  CLMem* mem = command->mem_src;

  cl_mem m = (cl_mem) mem->dev_specific[this];

  if (!mem->is_sub && !mem->use_host) {
    cl_int err = __real_clReleaseMemObject(m);
    SNUCL_ECHECK(err);
  }
  mem->ClearLatest(NULL);
  command->event->Complete();
}

cl_int LegacyCLDevice::GetSupportedImageFormats(cl_mem_flags flags, cl_mem_object_type image_type, cl_uint num_entries, cl_image_format *image_formats, cl_uint* num_image_formats) {
  cl_int err = __real_clGetSupportedImageFormats(ctx, flags, image_type, num_entries, image_formats, num_image_formats);
  SNUCL_ECHECK(err);
  return err;
}

void LegacyCLDevice::ReadKernelInfo(CLProgram* program) {
  cl_int err;

  pthread_mutex_lock(&program->mutex_dev_specific);
  cl_program p = (cl_program) program->dev_specific[this];
  pthread_mutex_unlock(&program->mutex_dev_specific);

  if (program->fromSource[this]) {
    size_t binary_size;
    err = __real_clGetProgramInfo(p, CL_PROGRAM_BINARY_SIZES, sizeof(binary_size), &binary_size, NULL);
    SNUCL_ECHECK(err);

    if (binary_size == 0) SNUCL_ERROR("BUILD PROGRAM ERROR BIN SIZE [%lu]", binary_size);

    unsigned char* binary = (unsigned char*) malloc(binary_size);
    unsigned char* buffers[1] = { binary };

    err = __real_clGetProgramInfo(p, CL_PROGRAM_BINARIES, sizeof(buffers), &buffers, NULL);
    SNUCL_ECHECK(err);

    CLBinary* bin = new CLBinary((char*) binary, binary_size, CL_PROGRAM_BINARY_TYPE_COMPILED_OBJECT);
    program->SetBin(this, bin, CL_PROGRAM_BINARY_TYPE_EXECUTABLE);
  }

  cl_uint num_kernels;
  err = __real_clCreateKernelsInProgram(p, 0, NULL, &num_kernels);
  SNUCL_ECHECK(err);

  program->num_kernels = num_kernels;
	//program->kernel_attributes = (char**) malloc(sizeof(char*) * num_kernels);
	program->kernel_reqd_work_group_size = (size_t**) malloc(sizeof(unsigned int*) * num_kernels);
	//program->kernel_work_group_size_hint = (size_t**) malloc(sizeof(unsigned int*) * num_kernels);
	program->isBuiltInKernel = (bool*) malloc(sizeof(bool) * num_kernels);
	program->kernel_num_args = (cl_uint*) malloc(sizeof(cl_uint) * num_kernels);
	memset(program->isBuiltInKernel, 0, sizeof(bool) * num_kernels);

  cl_kernel* kernels = (cl_kernel*) malloc(num_kernels * sizeof(cl_kernel));
  err = __real_clCreateKernelsInProgram(p, num_kernels, kernels, NULL);
  SNUCL_ECHECK(err);

  program->kernel_names = (char**) malloc(sizeof(char*) * num_kernels);
  for (cl_uint i = 0; i < num_kernels; i++) {
    cl_kernel k = kernels[i];
    char kernel_name[512];
    size_t kernel_name_len;
    err = __real_clGetKernelInfo(k, CL_KERNEL_FUNCTION_NAME, sizeof(kernel_name), kernel_name, &kernel_name_len);
    program->kernel_names[i] = (char*) calloc(kernel_name_len + 1, 1);
    strncpy(program->kernel_names[i], kernel_name, kernel_name_len);
    SNUCL_ECHECK(err);
    err = __real_clGetKernelInfo(k, CL_KERNEL_NUM_ARGS, sizeof(program->kernel_num_args[i]), &program->kernel_num_args[i], NULL);
    SNUCL_ECHECK(err);

		program->kernel_reqd_work_group_size[i] = (size_t*) malloc(3 * sizeof(size_t));
    err = __real_clGetKernelWorkGroupInfo(k, dev, CL_KERNEL_COMPILE_WORK_GROUP_SIZE, sizeof(size_t[3]), program->kernel_reqd_work_group_size[i], NULL);
    SNUCL_ECHECK(err);
  }

}

