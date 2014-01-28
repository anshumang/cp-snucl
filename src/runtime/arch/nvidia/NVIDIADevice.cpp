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

#include <arch/nvidia/NVIDIADevice.h>
#include <sys/stat.h>

SNUCL_DEBUG_HEADER("NVIDIADevice");

int NVIDIACLDevice::CreateDevices(vector<CLDevice*>* devices) {
  int num_devices = 0;
  cutilDrvSafeCall(cuInit(0));
  cutilDrvSafeCall(cuDeviceGetCount(&num_devices));
  SNUCL_INFO("%d NVIDIA GPUs are detected.", num_devices);

  char filepath[256];
  char* snucl_root = getenv("SNUCLROOT");
  sprintf(filepath, "%s/bin/snucl_ngpu", snucl_root);

  char* line = NULL;
  size_t len = 0;
  ssize_t read;
  FILE* fp = fopen(filepath, "r");

  if (fp != NULL) {
    if (read = getline(&line, &len, fp) != -1 && atoi(line)) {
      num_devices = atoi(line);
      SNUCL_INFO("SNUCL_NGPU [%d]", num_devices);
    }
    fclose(fp);
  }

  for (int i = 0; i < num_devices; ++i) {
    devices->push_back(new NVIDIACLDevice(i));
  }

  return num_devices;
}

NVIDIACLDevice::NVIDIACLDevice(int gpuid) : CLDevice(CL_DEVICE_TYPE_GPU) {
  this->gpuid = gpuid;

  worker = new CLDeviceWorker(this);
}

NVIDIACLDevice::~NVIDIACLDevice() {
  delete worker;
}

void NVIDIACLDevice::Init() {
  cutilDrvSafeCall(cuDeviceGet(&dev, gpuid));
  cutilDrvSafeCall(cuCtxCreate(&ctx, CU_CTX_SCHED_AUTO, dev));
  cutilDrvSafeCall(cuStreamCreate(&stream, 0));
}

void NVIDIACLDevice::InitInfo() {
  int pi;
  size_t bytes;
  cutilDrvSafeCall(cuDeviceGetAttribute(&pi, CU_DEVICE_ATTRIBUTE_PCI_DEVICE_ID, dev)); vendor_id = pi;
  cutilDrvSafeCall(cuDeviceGetAttribute(&pi, CU_DEVICE_ATTRIBUTE_MULTIPROCESSOR_COUNT, dev)); max_compute_units = pi;
  max_work_item_dimensions = 3;
  cutilDrvSafeCall(cuDeviceGetAttribute(&pi, CU_DEVICE_ATTRIBUTE_MULTIPROCESSOR_COUNT, dev)); max_compute_units = pi;
  cutilDrvSafeCall(cuDeviceGetAttribute(&pi, CU_DEVICE_ATTRIBUTE_MAX_BLOCK_DIM_X, dev)); max_work_item_sizes[0] = pi;
  cutilDrvSafeCall(cuDeviceGetAttribute(&pi, CU_DEVICE_ATTRIBUTE_MAX_BLOCK_DIM_Y, dev)); max_work_item_sizes[1] = pi;
  cutilDrvSafeCall(cuDeviceGetAttribute(&pi, CU_DEVICE_ATTRIBUTE_MAX_BLOCK_DIM_Z, dev)); max_work_item_sizes[2] = pi;
  cutilDrvSafeCall(cuDeviceGetAttribute(&pi, CU_DEVICE_ATTRIBUTE_MAX_THREADS_PER_BLOCK, dev)); max_work_group_size = pi;
  cutilDrvSafeCall(cuDeviceGetAttribute(&pi, CU_DEVICE_ATTRIBUTE_CLOCK_RATE, dev)); max_clock_frequency = pi;
  address_bits = 32;
  cutilDrvSafeCall(cuDeviceTotalMem(&bytes, dev)); max_mem_alloc_size = bytes;
  image_support = CL_FALSE;
  max_parameter_size = 1024;
  mem_base_addr_align = sizeof(cl_long16);
  min_data_type_align_size = sizeof(cl_long16);
  single_fp_config = CL_FP_DENORM | CL_FP_INF_NAN | CL_FP_ROUND_TO_NEAREST | CL_FP_ROUND_TO_ZERO | CL_FP_ROUND_TO_INF | CL_FP_FMA;
  global_mem_cache_type = CL_READ_WRITE_CACHE;
  global_mem_cacheline_size = 128;
  cutilDrvSafeCall(cuDeviceGetAttribute(&pi, CU_DEVICE_ATTRIBUTE_L2_CACHE_SIZE, dev)); global_mem_cache_size = pi;
  cutilDrvSafeCall(cuDeviceTotalMem(&bytes, dev)); global_mem_size = bytes;
  cutilDrvSafeCall(cuDeviceGetAttribute(&pi, CU_DEVICE_ATTRIBUTE_TOTAL_CONSTANT_MEMORY, dev)); max_constant_buffer_size = pi;
  max_constant_args = 9;
  local_mem_type = CL_LOCAL;
  cutilDrvSafeCall(cuDeviceGetAttribute(&pi, CU_DEVICE_ATTRIBUTE_MAX_SHARED_MEMORY_PER_BLOCK, dev)); local_mem_size = pi;
  cutilDrvSafeCall(cuDeviceGetAttribute(&pi, CU_DEVICE_ATTRIBUTE_ECC_ENABLED, dev)); error_correction_support = pi;
  cutilDrvSafeCall(cuDeviceGetAttribute(&pi, CU_DEVICE_ATTRIBUTE_UNIFIED_ADDRESSING, dev)); host_unified_memory = pi;
  profiling_timer_resolution = 1000;
  endian_little = CL_TRUE;
  available = CL_TRUE;
  compiler_available = CL_TRUE;
  execution_capabilities = CL_EXEC_KERNEL;
  queue_properties = CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_PROFILING_ENABLE;
  cutilDrvSafeCall(cuDeviceGetName(name, sizeof(name), dev));
  strcpy(vendor, "NVIDIA Corporation");
  cutilDrvSafeCall(cuDriverGetVersion(&pi));
  sprintf(driver_version, "CUDA %d", pi);
  sprintf(profile, "FULL_PROFILE");
}

void NVIDIACLDevice::LaunchKernel(CLCommand* command) {
  CLKernel* kernel = command->kernel;
  CLProgram* program = command->kernel->program;

  pthread_mutex_lock(&kernel->mutex_dev_specific);
  int k_created = kernel->dev_specific.count(this);
  pthread_mutex_unlock(&kernel->mutex_dev_specific);

  CUfunction k;   

  if (k_created == 0) {
    pthread_mutex_lock(&program->mutex_dev_specific);
    CUmodule p = (CUmodule) program->dev_specific[this];
    pthread_mutex_unlock(&program->mutex_dev_specific);

    cutilDrvSafeCall(cuModuleGetFunction(&k, p, kernel->name));

    pthread_mutex_lock(&kernel->mutex_dev_specific);
    kernel->dev_specific[this] = (void*) k;
    pthread_mutex_unlock(&kernel->mutex_dev_specific);
  } else {
    pthread_mutex_lock(&kernel->mutex_dev_specific);
    k = (CUfunction) kernel->dev_specific[this];
    pthread_mutex_unlock(&kernel->mutex_dev_specific);
  }

  map<cl_uint, CLKernelArg*>* args = command->kernel_args;
  void** kernel_params = (void**) malloc(args->size() * sizeof(void*));
  int* shared_offsets = (int*) malloc(args->size() * sizeof(int));
  CUdeviceptr* m = (CUdeviceptr*) malloc(args->size() * sizeof(CUdeviceptr));
  int kernel_arg_index = 0;
  unsigned int shared_mem_bytes = 0;
  for (cl_uint i = 0; i < (cl_uint) args->size(); ++i) {
    CLKernelArg* arg = (*args)[i];
    if (arg->mem) {
      CLMem* mem = arg->mem;
      CheckBuffer(mem);
      pthread_mutex_lock(&mem->mutex_dev_specific);
      m[i] = (CUdeviceptr) mem->dev_specific[this];
      pthread_mutex_unlock(&mem->mutex_dev_specific);
      kernel_params[kernel_arg_index++] = (void*) &m[i];
    } else if (arg->local) {
      shared_mem_bytes += arg->size;
      shared_offsets[kernel_arg_index] = shared_mem_bytes - arg->size;
      kernel_params[kernel_arg_index] = (void*) &shared_offsets[kernel_arg_index];
      kernel_arg_index++;
    } else {
      kernel_params[kernel_arg_index++] = (void*) arg->value;
    }
  }

  cutilDrvSafeCall(cuLaunchKernel(k, (unsigned int) command->nwg[0], (unsigned int) command->nwg[1], (unsigned int) command->nwg[2], (unsigned int) command->lws[0], (unsigned int) command->lws[1], (unsigned int) command->lws[2], shared_mem_bytes, stream, kernel_params, NULL));

  CUevent e;
  cutilDrvSafeCall(cuEventCreate(&e, CU_EVENT_DEFAULT));
  cutilDrvSafeCall(cuEventRecord(e, stream));
  cutilDrvSafeCall(cuEventSynchronize(e));
  cutilDrvSafeCall(cuEventDestroy(e));

  free(shared_offsets);
  free(kernel_params);
  free(m);

  for (map<cl_uint, CLKernelArg*>::const_iterator it = args->begin(); it != args->end(); ++it) {
    CLKernelArg* arg = it->second;
    CLMem* mem = arg->mem;
    if (!mem) continue;
    if (mem->flags & CL_MEM_WRITE_ONLY || mem->flags & CL_MEM_READ_WRITE) {
      mem->ClearLatest(this);
    }
  }

  command->event->Complete();
}

void NVIDIACLDevice::ReadBuffer(CLCommand* command) {
  CLMem* mem = command->mem_src;
  CheckBuffer(mem);

  pthread_mutex_lock(&mem->mutex_dev_specific);
  CUdeviceptr m = (CUdeviceptr) mem->dev_specific[this];
  pthread_mutex_unlock(&mem->mutex_dev_specific);

  if (m == 0) SNUCL_ERROR("MEM[%d] IS NULL", mem->id);

  cutilDrvSafeCall(cuMemcpyDtoH(command->ptr, m + command->off_src, command->cb));

  command->event->Complete();
}

void NVIDIACLDevice::WriteBuffer(CLCommand* command) {
  CLMem* mem_dst = command->mem_dst;
  CheckBuffer(mem_dst);

  pthread_mutex_lock(&mem_dst->mutex_dev_specific);
  CUdeviceptr m = (CUdeviceptr) mem_dst->dev_specific[this];
  pthread_mutex_unlock(&mem_dst->mutex_dev_specific);

  cutilDrvSafeCall(cuMemcpyHtoD(m + command->off_dst, (const void*) command->ptr, command->cb));

  mem_dst->ClearLatest(this);

  command->event->Complete();
}

void NVIDIACLDevice::CopyBuffer(CLCommand* command) {
  CLMem* mem_src = command->mem_src;
  CLMem* mem_dst = command->mem_dst;

  CheckBuffer(mem_src);
  CheckBuffer(mem_dst);

  pthread_mutex_lock(&mem_src->mutex_dev_specific);
  CUdeviceptr m_src = (CUdeviceptr) mem_src->dev_specific[this];
  pthread_mutex_unlock(&mem_src->mutex_dev_specific);
  if (m_src == 0) SNUCL_ERROR("SRC_MEM[%d] IS NULL", mem_src->id);

  pthread_mutex_lock(&mem_dst->mutex_dev_specific);
  CUdeviceptr m_dst = (CUdeviceptr) mem_dst->dev_specific[this];
  pthread_mutex_unlock(&mem_dst->mutex_dev_specific);
  if (m_dst == 0) SNUCL_ERROR("DST_MEM[%d] IS NULL", mem_dst->id);

  cutilDrvSafeCall(cuMemcpyDtoD(m_dst + command->off_dst, m_src + command->off_src, command->cb));

  mem_dst->ClearLatest(this);

  command->event->Complete();
}

#if 1
void NVIDIACLDevice::BuildProgram(CLCommand* command) {
  CLProgram* program = command->program;

  char cmd[256];
  char kernel_dir[128];
  char kernel_file[128];

  mkdir("snucl_kernels", 0755);
  mkdir("snucl_kernels/gpu", 0755);
  sprintf(kernel_dir, "snucl_kernels/gpu/%s", node_name);
  mkdir(kernel_dir, 0755);
  sprintf(kernel_dir, "snucl_kernels/gpu/%s/%d", node_name, gpuid);
  mkdir(kernel_dir, 0755);
  int file_idx = 0;
  while (1) {
    sprintf(kernel_file, "%s/__cl_kernel_%d.cl", kernel_dir, ++file_idx);
    if (access(kernel_file, F_OK) != 0) {
      FILE *file = fopen(kernel_file, "a");
      fprintf(file, "%s", program->src);
      fclose(file);
      break;
    }
  }

  if (strlen(program->options)) {
    SNUCL_DEBUG("OPTIONS [%s]", program->options);
    sprintf(cmd, "t-gpu.sh %d %d %s", gpuid, file_idx, program->options);
  } else {
    SNUCL_DEBUG("NO OPTIONS [%s]", "");
    sprintf(cmd, "t-gpu.sh %d %d", gpuid, file_idx);
  }

  if (system(cmd) == -1) SNUCL_ERROR("BUILD ERROR PROGRAM[%lu]", program->id);

  sprintf(kernel_file, "%s/__cl_kernel_%d.ptx", kernel_dir, file_idx);
  CUmodule p = (CUmodule) program->dev_specific[this];
  cutilDrvSafeCall(cuModuleLoad(&p, kernel_file));
  
  pthread_mutex_lock(&program->mutex_dev_specific);
  program->dev_specific[this] = (void*) p;
  pthread_mutex_unlock(&program->mutex_dev_specific);

  program->buildStatus[this] = CL_BUILD_SUCCESS;
}
#else
void NVIDIACLDevice::BuildProgram(CLCommand* command) {
  CLProgram* program = command->program;

  CUmodule p = (CUmodule) program->dev_specific[this];
  const char* ptx_file = "kernel_gpu/__cl_kernel.ptx";
  cutilDrvSafeCall(cuModuleLoad(&p, ptx_file));
  
  pthread_mutex_lock(&program->mutex_dev_specific);
  program->dev_specific[this] = (void*) p;
  pthread_mutex_unlock(&program->mutex_dev_specific);

  program->build_status = CL_BUILD_SUCCESS;
}
#endif

void NVIDIACLDevice::AllocBuffer(CLMem* mem) {
  CUdeviceptr m;
  cutilDrvSafeCall(cuMemAlloc(&m, mem->size));

  pthread_mutex_lock(&mem->mutex_dev_specific);
  mem->dev_specific[this] = (void*) m;
  pthread_mutex_unlock(&mem->mutex_dev_specific);

  mem->ClearLatest(this);
}

void NVIDIACLDevice::FreeBuffer(CLCommand* command) {
  CLMem* mem = command->mem_src;
  pthread_mutex_lock(&mem->mutex_dev_specific);
  CUdeviceptr m = (CUdeviceptr) mem->dev_specific[this];
  pthread_mutex_unlock(&mem->mutex_dev_specific);

  cutilDrvSafeCall(cuMemFree(m));

  mem->ClearLatest(NULL);
}

