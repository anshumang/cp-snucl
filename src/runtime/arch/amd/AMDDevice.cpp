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

#include <arch/amd/AMDDevice.h>
#include <CLAPI_real.h>

SNUCL_DEBUG_HEADER("AMDDevice");

#define MAX_PLATFORM  8
#define AMD_PLATFORM  "Advanced Micro Devices, Inc."
#define MAX_DEVICE    8

int AMDCLDevice::CreateDevices(vector<CLDevice*>* devices) {
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
    if (strcmp(AMD_PLATFORM, platform_name) == 0) {
      SNUCL_DEBUG("AMD PLATFORM [%d]", i);
      platform = platforms[i];
    }
  }

  if (platform == NULL) {
    SNUCL_INFO("%u AMD GPUs are detected.", 0);
    return 0;
  }

  cl_uint num_devices;
  err = __real_clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 0, NULL, &num_devices);
  SNUCL_INFO("%u AMD GPUs are detected.", num_devices);

  cl_device_id _devices[MAX_DEVICE];

  err = __real_clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, num_devices, _devices, NULL); 
  for (uint i = 0; i < num_devices; ++i) {
    devices->push_back(new AMDCLDevice((int) i, _devices[i]));
  }

  return (int) num_devices;
}

AMDCLDevice::AMDCLDevice(int gpuid, cl_device_id dev) : CLDevice(CL_DEVICE_TYPE_GPU) {
  this->gpuid = gpuid;
  this->dev = dev;

  worker = new CLDeviceWorker(this);
}

AMDCLDevice::~AMDCLDevice() {
  delete worker;
}

void AMDCLDevice::Init() {
  cl_int err;

  ctx = __real_clCreateContext(0, 1, &dev, NULL, NULL, &err);
  SNUCL_ECHECK(err);
  cmq = __real_clCreateCommandQueue(ctx, dev, 0, &err);
  SNUCL_ECHECK(err);
}

void AMDCLDevice::InitInfo() {
  cl_int err;
}

void AMDCLDevice::LaunchKernel(CLCommand* command) {
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
    } else if (arg->local) {
      err = __real_clSetKernelArg(k, index, arg->size, NULL);
    } else {
      err = __real_clSetKernelArg(k, index, arg->size, (void*) arg->value);
    }
    SNUCL_ECHECK(err);
  }

  cl_event e;
  err = __real_clEnqueueNDRangeKernel(cmq, k, command->work_dim, command->gwo, command->gws, command->lws, 0, NULL, &e);
  SNUCL_ECHECK(err);

  err = __real_clWaitForEvents(1, &e);
  SNUCL_ECHECK(err);

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

void AMDCLDevice::ReadBuffer(CLCommand* command) {
  cl_int err;

  CLMem* mem = command->mem_src;

  pthread_mutex_lock(&mem->mutex_dev_specific);
  cl_mem m = (cl_mem) mem->dev_specific[this];
  pthread_mutex_unlock(&mem->mutex_dev_specific);

  if (m == NULL) SNUCL_ERROR("MEM[%d] IS NULL", mem->id);

  err = __real_clEnqueueReadBuffer(cmq, m, CL_TRUE, command->off_src, command->cb, command->ptr, 0, NULL, NULL);
  err |= __real_clFlush(cmq);
  SNUCL_ECHECK(err);

  command->event->Complete();
}

void AMDCLDevice::WriteBuffer(CLCommand* command) {
  cl_int err;

  CLMem* mem_dst = command->mem_dst;
  CheckBuffer(mem_dst);

  pthread_mutex_lock(&mem_dst->mutex_dev_specific);
  cl_mem m = (cl_mem) mem_dst->dev_specific[this];
  pthread_mutex_unlock(&mem_dst->mutex_dev_specific);

  if (m == NULL) SNUCL_ERROR("MEM[%d] IS NULL", mem_dst->id);
  err = __real_clEnqueueWriteBuffer(cmq, m, CL_TRUE, command->off_dst, command->cb, (const void*) command->ptr, 0, NULL, NULL);
  err |= __real_clFlush(cmq);
  SNUCL_ECHECK(err);

  mem_dst->ClearLatest(this);

  command->event->Complete();
}

void AMDCLDevice::CopyBuffer(CLCommand* command) {
  cl_int err;

  CLMem* mem_src = command->mem_src;
  CLMem* mem_dst = command->mem_dst;

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

void AMDCLDevice::BuildProgram(CLCommand* command) {
  cl_int err;

  CLProgram* program = command->program;

  size_t length = strlen(program->src);
  cl_program p = __real_clCreateProgramWithSource(ctx, 1, (const char**) &program->src, &length, &err);
  SNUCL_ECHECK(err);
  err = __real_clBuildProgram(p, 1, &dev, (const char*) program->options, NULL, NULL);
  SNUCL_ECHECK(err);

  pthread_mutex_lock(&program->mutex_dev_specific);
  program->dev_specific[this] = (void*) p;
  pthread_mutex_unlock(&program->mutex_dev_specific);

  program->buildStatus[this] = CL_BUILD_SUCCESS;
}

void AMDCLDevice::AllocBuffer(CLMem* mem) {
  cl_int err;
  cl_mem m = __real_clCreateBuffer(ctx, mem->flags, mem->size, NULL, &err);
  SNUCL_ECHECK(err);

  pthread_mutex_lock(&mem->mutex_dev_specific);
  mem->dev_specific[this] = (void*) m;
  pthread_mutex_unlock(&mem->mutex_dev_specific);

  mem->ClearLatest(this);
}

void AMDCLDevice::FreeBuffer(CLCommand* command) {
  CLMem* mem = command->mem_src;
  pthread_mutex_lock(&mem->mutex_dev_specific);
  cl_mem m = (cl_mem) mem->dev_specific[this];
  pthread_mutex_unlock(&mem->mutex_dev_specific);

  cl_int err = __real_clReleaseMemObject(m);
  SNUCL_ECHECK(err);
  mem->ClearLatest(NULL);
}

