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

#include <arch/cluster/ClusterCompute.h>
#include <malloc.h>

#define MAX_DEVICE  8

SNUCL_DEBUG_HEADER("ClusterCompute");

extern MPI_Comm MPI_COMM_NODE;

ClusterCompute::ClusterCompute(int rank, char* name) {
  this->rank = rank;
  this->name = name;

  devs = &g_platform.devices;

  mc = new MessageCodec();
}

ClusterCompute::~ClusterCompute() {
  delete mc;
}

void ClusterCompute::InitContext() {
  cl_uint num_devices = (cl_uint) devs->size();
  cl_device_id devices[MAX_DEVICE];
  for (unsigned int i = 0; i < num_devices; ++i) {
    devices[i] = &((*devs)[i]->st_obj);
  }
  ctx = new CLContext(num_devices, devices, false);
  cids = (unsigned long*) calloc(num_devices, sizeof(unsigned long));
}

int ClusterCompute::Execute(int argc, char** argv) {
  InitContext();
  running = true;

  while (running) {
    int flag = 0;
    mpi_ret = MPI_Irecv(msg, MESSAGE_COMMAND_SIZE, MPI_CHAR, MPI_ANY_SOURCE, CLUSTER_TAG_COMMAND, MPI_COMM_WORLD, &mpi_request);
    do {
      CheckCompleteEvents();
      CheckCompleteEventsCS();
      mpi_ret = MPI_Test(&mpi_request, &flag, &mpi_status);
    } while (!flag);
    HandleMsg(msg);
  }
  return 0;
}

void ClusterCompute::HandleMsg(char* msg) {
  int tag = ((int*) msg)[0];
#ifdef MESSAGE_CHUNKING
  unsigned long cid;
  if (tag != CLUSTER_TAG_EXIT && tag != CLUSTER_TAG_NODE_INFO && tag != CLUSTER_TAG_CHUNK) {
    cid = ((unsigned long*) (msg + sizeof(int)))[0];
    int dev_id = ((int*) (msg + sizeof(int) + sizeof(unsigned long)))[0];
    if (cids[dev_id] + 1 == cid) {
      cids[dev_id]++;
    } else {
      SNUCL_DEBUG("NODE[%d] HANDLE MESSAGE[%s] FROM[%d] CID[%lu] DEV[%d]", rank, CLUSTER_TAG_STR[tag], mpi_status.MPI_SOURCE, cid, dev_id);
      char* d_msg = (char*) malloc(MESSAGE_COMMAND_SIZE);
      memcpy(d_msg, msg, MESSAGE_COMMAND_SIZE);
      cid_msgs[cid] = d_msg;
      return;
    }
  } else {
    SNUCL_DEBUG("NODE[%d] HANDLE MESSAGE[%s] FROM[%d]", rank, CLUSTER_TAG_STR[tag], mpi_status.MPI_SOURCE);
  }
#endif
  SNUCL_DEBUG("NODE[%d] TAG[%d] HANDLE MESSAGE[%s] FROM[%d]", rank, tag, CLUSTER_TAG_STR[tag], mpi_status.MPI_SOURCE);
  switch (tag) {
    case CLUSTER_TAG_EXIT:          HandleMsgExit();             break;
    case CLUSTER_TAG_NODE_INFO:     HandleMsgNodeInfo();         break;
    case CLUSTER_TAG_DEV_INFO:      HandleMsgDevInfo(msg);       break;
    case CLUSTER_TAG_PROGRAM_BUILD: HandleMsgProgramBuild(msg);  break;
    case CLUSTER_TAG_KERNEL_LAUNCH: HandleMsgKernelLaunch(msg);  break;
    case CLUSTER_TAG_MEM_ALLOC:     HandleMsgMemAlloc(msg);      break;
    case CLUSTER_TAG_MEM_COPY:      HandleMsgMemCopy(msg);       break;
    case CLUSTER_TAG_MEM_SEND:      HandleMsgMemSend(msg);       break;
    case CLUSTER_TAG_MEM_RECV:      HandleMsgMemRecv(msg);       break;
    case CLUSTER_TAG_MEM_COPY_RECT: HandleMsgMemCopyRect(msg);   break;
    case CLUSTER_TAG_MEM_SEND_RECT: HandleMsgMemSendRect(msg);   break;
    case CLUSTER_TAG_MEM_RECV_RECT: HandleMsgMemRecvRect(msg);   break;
    case CLUSTER_TAG_MEM_BCAST:     HandleMsgMemBcast(msg);      break;
    case CLUSTER_TAG_MEM_ALLTOALL:  HandleMsgMemAlltoall(msg);   break;
    case CLUSTER_TAG_MEM_FREE:      HandleMsgMemFree(msg);       break;
    case CLUSTER_TAG_DEV_STATS:     HandleMsgDevStats(msg);      break;
    case CLUSTER_TAG_MARKER:        HandleMsgMarker(msg);        break;
    case CLUSTER_TAG_NATIVE_KERNEL: HandleMsgNativeKernel(msg);  break;
    case CLUSTER_TAG_IMAGE_INFO:    HandleMsgImageInfo(msg);     break;
    case CLUSTER_TAG_IMAGE_COPY:    HandleMsgImageCopy(msg);     break;
    case CLUSTER_TAG_IMAGE_SEND:    HandleMsgImageSend(msg);     break;
    case CLUSTER_TAG_IMAGE_RECV:    HandleMsgImageRecv(msg);     break;
    case CLUSTER_TAG_IMAGE_BUFFER:  HandleMsgImageBuffer(msg);   break;
    case CLUSTER_TAG_BUFFER_IMAGE:  HandleMsgBufferImage(msg);   break;

    default: SNUCL_ERROR("Unknown MPI TAG [%d]", tag); break;
  }
#ifdef MESSAGE_CHUNKING
  if (cid_msgs.count(cid + 1)) {
    HandleMsg(cid_msgs[cid + 1]);
    cid_msgs.erase(cid + 1);
  }
#endif
}

void ClusterCompute::HandleMsgExit() {
  running = false;
}

void ClusterCompute::HandleMsgNodeInfo() {
  MessageCodec mc;
  int ndev = devs->size();
  SNUCL_DEBUG("NODE[%d] NDEV[%d]", rank, ndev);
  mc.SetInt(&ndev);
  for (int i = 0; i < ndev; ++i) {
    CLDevice* dev = (*devs)[i];
    mc.SetULong(&(dev->type));
  }
  MPI_Send(mc.msg, MESSAGE_COMMAND_SIZE, MPI_CHAR, 0, CLUSTER_TAG_NODE_INFO, MPI_COMM_WORLD); 
}

void ClusterCompute::HandleMsgDevInfo(char* msg) {
  MessageCodec mc(msg);
  int tag = mc.GetTag();
  unsigned long cid = mc.GetCID();
  int dev_id = mc.GetInt();

  CLDevice* dev = (*devs)[dev_id];

  char dev_info[8192];
  MessageCodec m(dev_info);
  m.SetUInt(&dev->vendor_id);
  m.SetUInt(&dev->max_compute_units);
  m.SetUInt(&dev->max_work_item_dimensions);
  m.Set(dev->max_work_item_sizes, sizeof(size_t) * 3);
  m.SetULong(&dev->max_work_group_size);
  m.SetUInt(&dev->preferred_vector_width_char);
  m.SetUInt(&dev->preferred_vector_width_short);
  m.SetUInt(&dev->preferred_vector_width_int);
  m.SetUInt(&dev->preferred_vector_width_long);
  m.SetUInt(&dev->preferred_vector_width_float);
  m.SetUInt(&dev->preferred_vector_width_double);
  m.SetUInt(&dev->preferred_vector_width_half);
  m.SetUInt(&dev->native_vector_width_char);
  m.SetUInt(&dev->native_vector_width_short);
  m.SetUInt(&dev->native_vector_width_int);
  m.SetUInt(&dev->native_vector_width_long);
  m.SetUInt(&dev->native_vector_width_float);
  m.SetUInt(&dev->native_vector_width_double);
  m.SetUInt(&dev->native_vector_width_half);
  m.SetUInt(&dev->max_clock_frequency);
  m.SetUInt(&dev->address_bits);
  m.SetULong(&dev->max_mem_alloc_size);
  m.SetUInt(&dev->image_support);
  m.SetUInt(&dev->max_read_image_args);
  m.SetUInt(&dev->max_write_image_args);
  m.SetULong(&dev->image2d_max_width);
  m.SetULong(&dev->image2d_max_height);
  m.SetULong(&dev->image3d_max_width);
  m.SetULong(&dev->image3d_max_height);
  m.SetULong(&dev->image3d_max_depth);
  m.SetULong(&dev->image_max_buffer_size);
  m.SetULong(&dev->image_max_array_size);
  m.SetUInt(&dev->max_samplers);
  m.SetULong(&dev->max_parameter_size);
  m.SetUInt(&dev->mem_base_addr_align);
  m.SetUInt(&dev->min_data_type_align_size);
  m.SetULong(&dev->single_fp_config);
  m.SetULong(&dev->double_fp_config);
  m.SetUInt(&dev->global_mem_cache_type);
  m.SetUInt(&dev->global_mem_cacheline_size);
  m.SetULong(&dev->global_mem_cache_size);
  m.SetULong(&dev->global_mem_size);
  m.SetULong(&dev->max_constant_buffer_size);
  m.SetUInt(&dev->max_constant_args);
  m.SetUInt(&dev->local_mem_type);
  m.SetULong(&dev->local_mem_size);
  m.SetUInt(&dev->error_correction_support);
  m.SetUInt(&dev->host_unified_memory);
  m.SetULong(&dev->profiling_timer_resolution);
  m.SetUInt(&dev->endian_little);
  m.SetUInt(&dev->available);
  m.SetUInt(&dev->compiler_available);
  m.SetUInt(&dev->linker_available);
  m.SetULong(&dev->execution_capabilities);
  m.SetULong(&dev->queue_properties);
  m.Set(dev->built_in_kernels, 64);
  m.Set(dev->name, 64);
  m.Set(dev->vendor, 64);
  m.Set(dev->driver_version, 64);
  m.Set(dev->profile, 32);
  m.Set(dev->device_version, 32);
  m.Set(dev->openclc_version, 32);
  m.Set(dev->device_extensions, 1024);
  m.SetLong(&dev->printf_buffer_size);
  m.SetUInt(&dev->preferred_interop_user_sync);
  //m.SetUInt(&dev->parent_device);
  m.SetUInt(&dev->partition_max_sub_devices);
  m.Set(dev->partition_properties, sizeof(cl_device_partition_property) * 3);
  m.SetULong(&dev->num_partition_properties);
  m.SetULong(&dev->affinity_domain);
  //m.SetULong(dev->partition_type);
  m.SetUInt(&dev->partition_type_len);

  MPI_Send(m.msg, sizeof(dev_info), MPI_CHAR, 0, CLUSTER_TAG_DEV_INFO, MPI_COMM_WORLD); 
}

void ClusterCompute::HandleMsgProgramBuild(char* msg) {
  MessageCodec mc(msg);
  int tag = mc.GetTag();
  unsigned long cid = mc.GetCID();
  int dev_id = mc.GetInt();
  unsigned long program_id = mc.GetULong();
  size_t len_src = mc.GetULong();
  size_t len_options = mc.GetULong();
  unsigned long event_id = mc.GetULong();

  char* src = (char*) calloc(len_src + 1, 1);
  char* options = NULL;

  MPI_Status mpi_status;
  mpi_ret = MPI_Recv((void*) src, (int) len_src, MPI_CHAR, 0, CLUSTER_TAG_PROGRAM_BUILD, MPI_COMM_WORLD, &mpi_status);
  if (len_options) {
    options = (char*) calloc(len_options + 1, 1);
    mpi_ret = MPI_Recv((void*) options, (int) len_options, MPI_CHAR, 0, CLUSTER_TAG_PROGRAM_BUILD, MPI_COMM_WORLD, &mpi_status);
  }

  CLDevice* dev = (*devs)[dev_id];
  CLProgram* program = NULL;
  if (programs.count(program_id)) {
    program = programs[program_id];
  } else {
    program = new CLProgram(ctx, NULL, 0);
    programs[program_id] = program;
  }
  program->CheckOptions(options, NULL);
  program->SetSrc(dev, src);
  dev->EnqueueBuildProgram(program, options);

  free(src);
  if (options) free(options);

  mpi_ret = MPI_Send(&event_id, sizeof(event_id), MPI_CHAR, 0, CLUSTER_TAG_EVENT_WAIT(event_id), MPI_COMM_WORLD);
}

void ClusterCompute::HandleMsgKernelLaunch(char* msg) {
  MessageCodec mc(msg);
  int tag = mc.GetTag();
  unsigned long cid = mc.GetCID();
  int dev_id = mc.GetInt();
  unsigned long program_id = mc.GetULong();
  unsigned long kernel_id = mc.GetULong();
  size_t len_kernel_name = mc.GetULong();
  char kernel_name[256];
  mc.Get(kernel_name, len_kernel_name);
  kernel_name[len_kernel_name] = 0;

  if (!programs.count(program_id)) SNUCL_ERROR("NO PROGRAM[%lu]", program_id);

  CLDevice* dev = (*devs)[dev_id];
  CLProgram* program = programs[program_id];
  CLKernel* kernel = NULL;

  if (kernels.count(kernel_id)) {
    kernel = kernels[kernel_id];
  } else {
    kernel = new CLKernel(program, kernel_name);
    for (uint i = 0; i < program->num_kernels; i++) {
      if(strcmp(kernel_name, program->kernel_names[i]) == 0) {
        kernel->kernel_idx = i;
        break;
      }
    }
    kernels[kernel_id] = kernel;
  }

  unsigned int num_arg = mc.GetUInt();
  for (cl_uint i = 0; i < num_arg; ++i) {
    int arg_type = mc.GetInt();
    if (arg_type == 0) {
      unsigned long mem_id = GetMemID(dev_id, &mc);
      if (!mems.count(mem_id)) SNUCL_ERROR("NO MEM[%lu]", mem_id);
      CLMem* mem = mems[mem_id];
      cl_mem m = &mem->st_obj;
      kernel->SetArg(i, sizeof(cl_mem), &m);
    } else if (arg_type == 1) {
      unsigned long sampler_id = GetSamplerID(dev_id, &mc);
      if (!samplers.count(sampler_id)) SNUCL_ERROR("NO SAMPLER[%lu]", sampler_id);
      CLSampler* sampler = samplers[sampler_id];
      cl_sampler s = &sampler->st_obj;
      kernel->SetArg(i, sizeof(cl_sampler), &s);
    } else if (arg_type == 2) {
      size_t size = mc.GetULong();
      kernel->SetArg(i, size, NULL);
    } else if (arg_type == 3) {
      size_t size = mc.GetULong();
      char value[256];
      mc.Get(value, size);
      kernel->SetArg(i, size, value);
    } else {
      SNUCL_ERROR("UNSPPORTED ARG_TYPE[%d]", arg_type);
    }
  }

  unsigned int work_dim = mc.GetUInt();
  size_t gwo[3];
  size_t gws[3];
  size_t lws[3];

  gwo[0] = mc.GetULong();
  gwo[1] = mc.GetULong();
  gwo[2] = mc.GetULong();
  gws[0] = mc.GetULong();
  gws[1] = mc.GetULong();
  gws[2] = mc.GetULong();
  lws[0] = mc.GetULong();
  lws[1] = mc.GetULong();
  lws[2] = mc.GetULong();

  unsigned long event_id = mc.GetULong();
  bool event_disclosed = mc.GetBool();

  SNUCL_DEBUG("DEV[%d] PROGRAM[%lu] KERNEL[%lu][%lu][%s] DIM[%u] GWO[%lu,%lu,%lu] GWS[%lu,%lu,%lu] LWS[%lu,%lu,%lu] EVENT[%lu]", dev_id, program_id, kernel_id, len_kernel_name, kernel_name, work_dim, gwo[0], gwo[1], gwo[2], gws[0], gws[1], gws[2], lws[0], lws[1], lws[2], event_id);

  CLCommand* command = new CLCommand(CL_COMMAND_NDRANGE_KERNEL);
  command->device = dev;
  command->kernel = kernel;
  command->program = program;
  command->work_dim = work_dim;
  for (uint i = 0; i < 3; ++i) {
    command->gwo[i] = gwo[i];
    command->gws[i] = gws[i];
    command->lws[i] = lws[i];
    command->nwg[i] = gws[i] / lws[i];
  }
  command->SetKernelArgs();
  command->event->disclosed = event_disclosed;

  EnqueueReadyQueue(dev, command, CL_FALSE);
  wait_events[event_id] = command;
}

void ClusterCompute::HandleMsgMemAlloc(char* msg) {
  MessageCodec mc(msg);
  int tag = mc.GetTag();
  unsigned long cid = mc.GetCID();
  int dev_id = mc.GetInt();
  unsigned long mem_id = mc.GetULong();
  cl_mem_flags flags = mc.GetULong();
  size_t size = mc.GetULong();

  AllocMem(dev_id, mem_id, flags, size, false, NULL, NULL, false, 0, 0, NULL);
}

void ClusterCompute::HandleMsgMemCopy(char* msg) {
  MessageCodec mc(msg);
  int tag = mc.GetTag();
  unsigned long cid = mc.GetCID();
  int dev_dst_id = mc.GetInt();
  int dev_src_id = mc.GetInt();
  unsigned long mem_dst_id = GetMemID(dev_dst_id, &mc);
  unsigned long mem_src_id = GetMemID(dev_src_id, &mc);
  unsigned long off_dst = mc.GetLong();
  unsigned long off_src = mc.GetLong();
  unsigned long size = mc.GetLong();
  unsigned long event_id = mc.GetLong();

  CLDevice* dev_src = (*devs)[dev_src_id];
  CLDevice* dev_dst = (*devs)[dev_dst_id];
  CLMem* mem_src = mems[mem_src_id];
  CLMem* mem_dst = mems[mem_dst_id];

  if (dev_src == dev_dst) {
    CLCommand* command = new CLCommand(CL_COMMAND_COPY_BUFFER);
    command->mem_src = mem_src;
    command->mem_dst = mem_dst;
    command->off_src = off_src;
    command->off_dst = off_dst;
    command->cb = size;
    EnqueueReadyQueue(dev_dst, command, CL_FALSE);
    wait_events[event_id] = command;
  } else {
    if (dev_src->type == CL_DEVICE_TYPE_CPU && dev_dst->type == CL_DEVICE_TYPE_GPU) {
      CLCommand* command = new CLCommand(CL_COMMAND_WRITE_BUFFER);
      command->mem_dst = mem_dst;
      command->off_dst = off_dst;
      command->cb = size;
      void* m = mem_src->dev_specific[dev_src];
      command->ptr = (void*) ((size_t) m + off_src);
      EnqueueReadyQueue(dev_dst, command, CL_FALSE);
      wait_events[event_id] = command;
    } else if (dev_src->type == CL_DEVICE_TYPE_GPU && dev_dst->type == CL_DEVICE_TYPE_CPU) {
      CLCommand* command = new CLCommand(CL_COMMAND_READ_BUFFER);
      command->mem_src = mem_src;
      command->off_src = off_src;
      command->cb = size;
      void* m = mem_dst->dev_specific[dev_dst];
      command->ptr = (void*) ((size_t) m + off_dst);
      command->misc[0] = CLUSTER_TAG_MEM_COPY;

      EnqueueReadyQueue(dev_src, command, CL_FALSE);
      wait_events[event_id] = command;
    } else if (dev_src->type == CL_DEVICE_TYPE_GPU && dev_dst->type == CL_DEVICE_TYPE_GPU) {
      if (mem_src->space_host_valid) {
        CLCommand* command = new CLCommand(CL_COMMAND_WRITE_BUFFER);
        command->mem_dst = mem_dst;
        command->off_dst = off_dst;
        command->cb = size;
        void* m = mem_src->space_host;
        command->ptr = (void*) ((size_t) m + off_src);
        EnqueueReadyQueue(dev_dst, command, CL_FALSE);
        wait_events[event_id] = command;
      } else {
        CLCommand* command = new CLCommand(CL_COMMAND_READ_BUFFER);
        command->mem_src = mem_src;
        command->off_src = off_src;
        command->cb = size;
        command->ptr = (void*) ((size_t) mem_src->space_host + off_src);
        command->misc[0] = CLUSTER_TAG_MEM_COPY;

        CLCommand* sub_command = new CLCommand(CL_COMMAND_WRITE_BUFFER);
        sub_command->mem_dst = mem_dst;
        sub_command->off_dst = off_dst;
        sub_command->cb = size;
        sub_command->ptr = command->ptr;
        sub_command->device = dev_dst;

        command->sub_command = sub_command;
        EnqueueReadyQueue(dev_src, command, CL_FALSE);
        wait_events[event_id] = command;
      }
    } else {
      SNUCL_ERROR("UNSUPPORT DEVICE TYPE SRC[%lx] DST[%lx]", dev_src->type, dev_dst->type);
      return;
    }
  }
}

void ClusterCompute::HandleMsgMemSend(char* msg) {
  MessageCodec mc(msg);
  int tag = mc.GetTag();
  unsigned long cid = mc.GetCID();
  int dev_id = mc.GetInt();
  int node_id = mc.GetInt();
  unsigned long mem_id = GetMemID(dev_id, &mc);
  size_t offset = mc.GetULong();
  size_t size = mc.GetULong();
  unsigned long event_id = mc.GetULong();

  CLDevice* dev = (*devs)[dev_id];
  CLMem* mem = mems[mem_id];

  if (dev->type == CL_DEVICE_TYPE_CPU) {
    CLCommand* command = new CLCommand(CL_COMMAND_NOP);
    command->mem_src = mem;
    command->off_src = offset;
    command->cb = size;
    command->device = dev;

    command->misc[0] = CLUSTER_TAG_MEM_SEND;
    command->misc[1] = node_id;

    EnqueueReadyQueue(dev, command, CL_FALSE);
    wait_events[event_id] = command;
    /*
    void* m = mem->dev_specific[dev];
    MPI_Request mpi_request;
    mpi_ret = MPI_Isend((void*) ((size_t) m + offset), size, MPI_CHAR, node_id, CLUSTER_TAG_MEM_SEND_BODY(event_id), MPI_COMM_WORLD, &mpi_request);
    */
  } else if (dev->type == CL_DEVICE_TYPE_GPU) {
    CLCommand* command = new CLCommand(CL_COMMAND_READ_BUFFER);
    command->mem_src = mem;
    command->off_src = offset;
    command->cb = size;
    command->ptr = (void*) ((size_t) mem->space_host + offset);

    command->misc[0] = CLUSTER_TAG_MEM_SEND;
    command->misc[1] = node_id;

    EnqueueReadyQueue(dev, command, CL_FALSE);
    wait_events[event_id] = command;
  } else {
    SNUCL_ERROR("UNSUPPORT DEVICE TYPE [%lx]", dev->type);
    return;
  }
}

void ClusterCompute::HandleMsgMemRecv(char* msg) {
  MessageCodec mc(msg);
  int tag = mc.GetTag();
  unsigned long cid = mc.GetCID();
  int dev_id = mc.GetInt();
  int node_id = mc.GetInt();
  unsigned long mem_id = GetMemID(dev_id, &mc);
  size_t offset = mc.GetULong();
  size_t size = mc.GetULong();
  unsigned long event_id = mc.GetULong();

  SNUCL_DEBUG("MEM RECV DEV[%d] NODE[%d] MEM[%d] OFFSET[%lu] SIZE[%lu] EVENT[%lu]", dev_id, node_id, mem_id, offset, size, event_id);

#if 0
  CLDevice* dev = (*devs)[dev_id];
  CLMem* mem = mems[mem_id];
  ClusterSync* cs = MakeCS(event_id);
  void* buf = NULL;
  if (dev->type == CL_DEVICE_TYPE_CPU) {
    void* m = mem->dev_specific[dev];
    buf = (void*) ((size_t) m + offset);
  } else if (dev->type == CL_DEVICE_TYPE_GPU) {
    buf = (void*) ((size_t) mem->space_host + offset);
    CLCommand* command = new CLCommand(CL_COMMAND_WRITE_BUFFER); 
    command->mem_dst = mem;
    command->off_dst = offset;
    command->cb = size;
    command->ptr = buf;
    cs->command = command;
    cs->device = dev;
    mem->space_host_valid = true;
  } else {
    SNUCL_ERROR("UNSUPPORT DEVICE TYPE [%lx]", dev->type);
    return;
  }
  mpi_ret = MPI_Irecv(buf, size, MPI_CHAR, node_id, CLUSTER_TAG_MEM_RECV_BODY(event_id), MPI_COMM_WORLD, &cs->mpi_request_wait);
  wait_events_cs.push_back(cs);
#else
  CLDevice* dev = (*devs)[dev_id];
  CLMem* mem = mems[mem_id];

  CLCommand* command = new CLCommand(CL_COMMAND_NOP);
  command->mem_dst = mem;
  command->off_dst = offset;
  command->cb = size;
  command->device = dev;

  command->misc[0] = CLUSTER_TAG_MEM_RECV;
  command->misc[1] = node_id;

  EnqueueReadyQueue(dev, command, CL_FALSE);
  wait_events[event_id] = command;
#endif
}

void ClusterCompute::HandleMsgMemCopyRect(char* msg) {
  MessageCodec mc(msg);
  int tag = mc.GetTag();
  unsigned long cid = mc.GetCID();
  int dev_dst_id = mc.GetInt();
  int dev_src_id = mc.GetInt();
  unsigned long mem_dst_id = GetMemID(dev_dst_id, &mc);
  unsigned long mem_src_id = GetMemID(dev_src_id, &mc);
  size_t src_origin[3];
  size_t dst_origin[3];
  size_t region[3];

  src_origin[0] = mc.GetULong();
  src_origin[1] = mc.GetULong();
  src_origin[2] = mc.GetULong();
  dst_origin[0] = mc.GetULong();
  dst_origin[1] = mc.GetULong();
  dst_origin[2] = mc.GetULong();
  region[0] = mc.GetULong();
  region[1] = mc.GetULong();
  region[2] = mc.GetULong();

  size_t src_row_pitch = mc.GetULong();
  size_t src_slice_pitch = mc.GetULong();
  size_t dst_row_pitch = mc.GetULong();
  size_t dst_slice_pitch = mc.GetULong();

  unsigned long event_id = mc.GetLong();
  bool event_disclosed = mc.GetBool();

  CLDevice* dev_src = (*devs)[dev_src_id];
  CLDevice* dev_dst = (*devs)[dev_dst_id];
  CLMem* mem_src = mems[mem_src_id];
  CLMem* mem_dst = mems[mem_dst_id];

  CLCommand* command = new CLCommand(CL_COMMAND_COPY_BUFFER_RECT);
  command->mem_src = mem_src;
  command->mem_dst = mem_dst;
  command->src_origin[0] = src_origin[0];
  command->src_origin[1] = src_origin[1];
  command->src_origin[2] = src_origin[2];
  command->dst_origin[0] = dst_origin[0];
  command->dst_origin[1] = dst_origin[1];
  command->dst_origin[2] = dst_origin[2];
  command->region[0] = region[0];
  command->region[1] = region[1];
  command->region[2] = region[2];
  command->src_row_pitch = src_row_pitch;
  command->src_slice_pitch = src_slice_pitch;
  command->dst_row_pitch = dst_row_pitch;
  command->dst_slice_pitch = dst_slice_pitch;
  EnqueueReadyQueue(dev_dst, command, CL_FALSE);
  wait_events[event_id] = command;

}

void ClusterCompute::HandleMsgMemSendRect(char* msg) {
  MessageCodec mc(msg);
  int tag = mc.GetTag();
  unsigned long cid = mc.GetCID();
  int dev_id = mc.GetInt();
  int node_id = mc.GetInt();
  unsigned long mem_id = GetMemID(dev_id, &mc);
  size_t src_origin[3];
  size_t dst_origin[3];
  size_t region[3];

  src_origin[0] = mc.GetULong();
  src_origin[1] = mc.GetULong();
  src_origin[2] = mc.GetULong();
  dst_origin[0] = mc.GetULong();
  dst_origin[1] = mc.GetULong();
  dst_origin[2] = mc.GetULong();
  region[0] = mc.GetULong();
  region[1] = mc.GetULong();
  region[2] = mc.GetULong();

  size_t src_row_pitch = mc.GetULong();
  size_t src_slice_pitch = mc.GetULong();
  size_t dst_row_pitch = mc.GetULong();
  size_t dst_slice_pitch = mc.GetULong();
  size_t size = mc.GetULong();
  size_t offset = 0;
  unsigned long event_id = mc.GetULong();
  bool event_disclosed = mc.GetBool();

  SNUCL_DEBUG("MEM SEND RECT DEV[%d] NODE[%d] MEM[%d] SRC_ORIGIN[%lu,%lu,%lu] DST_ORIGIN[%lu,%lu,%lu] REGION[%lu, %lu, %lu] SRC_ROW[%lu] SRC_SLICE[%lu] DST_ROW[%lu] DST_SLICE[%lu] SIZE[%lu] OFFSET[%lu] EVENT[%lu]", dev_id, node_id, mem_id, src_origin[0], src_origin[1], src_origin[2], dst_origin[0], dst_origin[1], dst_origin[2], region[0], region[1], region[2], src_row_pitch, src_slice_pitch, dst_row_pitch, dst_slice_pitch, size, offset, event_id);

  CLDevice* dev = (*devs)[dev_id];
  CLMem* mem = mems[mem_id];

  CLCommand* command = new CLCommand(CL_COMMAND_READ_BUFFER);
  command->mem_src = mem;
  command->off_src = offset;
  command->cb = size;
  command->ptr = (void*) ((size_t) mem->space_host + offset);

  command->misc[0] = CLUSTER_TAG_MEM_SEND;
  command->misc[1] = node_id;

  EnqueueReadyQueue(dev, command, CL_FALSE);
  wait_events[event_id] = command;
}

void ClusterCompute::HandleMsgMemRecvRect(char* msg) {
  MessageCodec mc(msg);
  int tag = mc.GetTag();
  unsigned long cid = mc.GetCID();
  int dev_id = mc.GetInt();
  int node_id = mc.GetInt();
  unsigned long mem_id = GetMemID(dev_id, &mc);
  size_t src_origin[3];
  size_t dst_origin[3];
  size_t region[3];

  src_origin[0] = mc.GetULong();
  src_origin[1] = mc.GetULong();
  src_origin[2] = mc.GetULong();
  dst_origin[0] = mc.GetULong();
  dst_origin[1] = mc.GetULong();
  dst_origin[2] = mc.GetULong();
  region[0] = mc.GetULong();
  region[1] = mc.GetULong();
  region[2] = mc.GetULong();

  size_t src_row_pitch = mc.GetULong();
  size_t src_slice_pitch = mc.GetULong();
  size_t dst_row_pitch = mc.GetULong();
  size_t dst_slice_pitch = mc.GetULong();
  size_t size = mc.GetULong();
  size_t offset = 0;
  unsigned long event_id = mc.GetULong();
  bool event_disclosed = mc.GetBool();

  SNUCL_DEBUG("MEM RECV RECT DEV[%d] NODE[%d] MEM[%d] SRC_ORIGIN[%lu,%lu,%lu] DST_ORIGIN[%lu,%lu,%lu] REGION[%lu, %lu, %lu] SRC_ROW[%lu] SRC_SLICE[%lu] DST_ROW[%lu] DST_SLICE[%lu] SIZE[%lu] OFFSET[%lu] EVENT[%lu]", dev_id, node_id, mem_id, src_origin[0], src_origin[1], src_origin[2], dst_origin[0], dst_origin[1], dst_origin[2], region[0], region[1], region[2], src_row_pitch, src_slice_pitch, dst_row_pitch, dst_slice_pitch, size, offset, event_id);

  CLDevice* dev = (*devs)[dev_id];
  CLMem* mem = mems[mem_id];

  CLCommand* command = new CLCommand(CL_COMMAND_NOP);
  command->device = dev;
  command->buffer = mem;
  command->src_origin[0] = src_origin[0];
  command->src_origin[1] = src_origin[1];
  command->src_origin[2] = src_origin[2];
  command->dst_origin[0] = dst_origin[0];
  command->dst_origin[1] = dst_origin[1];
  command->dst_origin[2] = dst_origin[2];
  command->region[0] = region[0];
  command->region[1] = region[1];
  command->region[2] = region[2];
  command->src_row_pitch = src_row_pitch;
  command->src_slice_pitch = src_slice_pitch;
  command->dst_row_pitch = dst_row_pitch;
  command->dst_slice_pitch = dst_slice_pitch;
  command->cb = size;

  command->misc[0] = CLUSTER_TAG_MEM_RECV_RECT;
  command->misc[1] = node_id;

  EnqueueReadyQueue(dev, command, CL_FALSE);
  wait_events[event_id] = command;
}

void ClusterCompute::HandleMsgMemBcast(char* msg) {
  MessageCodec mc(msg);
  int tag = mc.GetTag();
  unsigned long cid = mc.GetCID();
  int dev_id = mc.GetInt();
  int node_id = mc.GetInt();
  unsigned long mem_src_id = mc.GetULong();
  unsigned long mem_dst_id = mc.GetULong();
  size_t off_src = mc.GetULong();
  size_t off_dst = mc.GetULong();
  size_t size = mc.GetULong();
  unsigned long event_id = mc.GetULong();

  SNUCL_DEBUG("MEM BCAST DEV[%d] NODE[%d] MEM_SRC[%lu] MEM_DST[%lu] OFF_SRC[%lu] OFF_DST[%lu] SIZE[%lu] EVENT[%lu]", dev_id, node_id, mem_src_id, mem_dst_id, off_src, off_dst, size, event_id);

  CLDevice* dev = (*devs)[dev_id];
  CLMem* mem_dst = mems[mem_dst_id];

  if (rank == node_id) {
    CLMem* mem_src = mems[mem_src_id];
    void* m = mem_src->dev_specific[dev];
    void* buf = (void*) ((size_t) m + off_src);
    mpi_ret = MPI_Bcast(buf, (int) size, MPI_CHAR, node_id - 1, MPI_COMM_NODE);
  } else {
    void* m = mem_dst->dev_specific[dev];
    void* buf = (void*) ((size_t) m + off_dst);
    mpi_ret = MPI_Bcast(buf, (int) size, MPI_CHAR, node_id - 1, MPI_COMM_NODE);
  }

  mpi_ret = MPI_Send(&event_id, sizeof(event_id), MPI_CHAR, 0, CLUSTER_TAG_EVENT_WAIT(event_id), MPI_COMM_WORLD);
}

void ClusterCompute::HandleMsgMemAlltoall(char* msg) {
  MessageCodec mc(msg);
  int tag = mc.GetTag();
  unsigned long cid = mc.GetCID();
  int dev_id = mc.GetInt();
  unsigned long mem_src_id = mc.GetULong();
  unsigned long mem_dst_id = mc.GetULong();
  size_t off_src = mc.GetULong();
  size_t off_dst = mc.GetULong();
  size_t size = mc.GetULong();
  unsigned long event_id = mc.GetULong();

  SNUCL_DEBUG("MEM ALLTOALL DEV[%d] MEM_SRC[%lu] MEM_DST[%lu] OFF_SRC[%lu] OFF_DST[%lu] SIZE[%lu] EVENT[%lu]", dev_id, mem_src_id, mem_dst_id, off_src, off_dst, size, event_id);

  CLDevice* dev = (*devs)[dev_id];
  CLMem* mem_src = mems[mem_src_id];
  CLMem* mem_dst = mems[mem_dst_id];

  void* m_src = mem_src->dev_specific[dev];
  void* m_dst = mem_dst->dev_specific[dev];

  void* buf_src = (void*) ((size_t) m_src + off_src);
  void* buf_dst = (void*) ((size_t) m_dst + off_dst);

  mpi_ret = MPI_Alltoall(buf_src, size, MPI_CHAR, buf_dst, size, MPI_CHAR, MPI_COMM_NODE);

  mpi_ret = MPI_Send(&event_id, sizeof(event_id), MPI_CHAR, 0, CLUSTER_TAG_EVENT_WAIT(event_id), MPI_COMM_WORLD);
}

void ClusterCompute::HandleMsgMemFree(char* msg) {
  MessageCodec mc(msg);
  int tag = mc.GetTag();
  unsigned long cid = mc.GetCID();
  int dev_id = mc.GetInt();
  unsigned long mem_id = mc.GetULong();
  unsigned long event_id = mc.GetULong();

  CLDevice* dev = (*devs)[dev_id];
  CLMem* mem = mems[mem_id];

  CLCommand* command = new CLCommand(CL_COMMAND_FREE_BUFFER);
  command->device = dev;
  command->mem_src = mem;
  dev->FreeBuffer(command);

  mpi_ret = MPI_Send(&event_id, sizeof(event_id), MPI_CHAR, 0, CLUSTER_TAG_EVENT_WAIT(event_id), MPI_COMM_WORLD);
}

void ClusterCompute::HandleMsgDevStats(char* msg) {
  MessageCodec mc(msg);
  int tag = mc.GetTag();
  unsigned long cid = mc.GetCID();
  int dev_id = mc.GetInt();
  CLDevice* dev = (*devs)[dev_id];
  dev->PrintStatistics();
}

void ClusterCompute::HandleMsgMarker(char* msg) {
  MessageCodec mc(msg);
  int tag = mc.GetTag();
  unsigned long cid = mc.GetCID();
  int dev_id = mc.GetInt();
  unsigned long event_id = mc.GetULong();

  SNUCL_DEBUG("MARKER DEV[%d] EVENT[%lu]", dev_id, event_id);

  CLDevice* dev = (*devs)[dev_id];

  CLCommand* command = new CLCommand(CL_COMMAND_MARKER);
  command->device = dev;
  EnqueueReadyQueue(dev, command, CL_FALSE);
  wait_events[event_id] = command;
}

void ClusterCompute::HandleMsgNativeKernel(char* msg) {
  MessageCodec mc(msg);
  int tag = mc.GetTag();
  unsigned long cid = mc.GetCID();
  int dev_id = mc.GetInt();
  size_t user_func_p = mc.GetULong();
  void (*user_func)(void*);
  memcpy(&user_func, &user_func_p, sizeof(size_t));
  size_t cb_args = mc.GetULong();
  void* args = malloc(cb_args);
  mc.Get(args, cb_args);
  int num_mem_objects = mc.GetInt();

  cl_mem* mem_list = (cl_mem*) malloc(num_mem_objects * sizeof(cl_mem));

  for (int i = 0; i < num_mem_objects; i++) {
    unsigned long mem_id = GetMemID(dev_id, &mc);
    if (!mems.count(mem_id)) SNUCL_ERROR("NO MEM[%lu]", mem_id);
    CLMem* mem = mems[mem_id];
    mem_list[i] = &mem->st_obj;
  }

  void** args_mem_loc = (void**) malloc(num_mem_objects * sizeof(void*));
  for (int i = 0; i < num_mem_objects; i++) {
    args_mem_loc[i] = (void*) ((size_t) args + mc.GetULong());
  }

  unsigned long event_id = mc.GetULong();
  bool event_disclosed = mc.GetBool();

  CLDevice* dev = (*devs)[dev_id];

  CLCommand* command = new CLCommand(CL_COMMAND_NATIVE_KERNEL);
  command->user_func = user_func;
  command->ptr = args;
  command->kernel_args = new map<cl_uint, CLKernelArg*>();
  (*command->kernel_args)[0] = new CLKernelArg(cb_args, args); 
  (*command->kernel_args)[1] = new CLKernelArg(num_mem_objects * sizeof(cl_mem), mem_list);
  (*command->kernel_args)[2] = new CLKernelArg(num_mem_objects * sizeof(void*), args_mem_loc);

  command->event->disclosed = event_disclosed;

  SNUCL_DEBUG("NATIVE KERNEL DEV[%d] USER_FUNC[%p] CB_ARGS[%lu] ARGS[%p] NUM_MEMS[%d] MEM[%lu, %lu], LOC[%p, %p] EVENT[%lu], USER[%d]", dev_id, user_func, cb_args, args, num_mem_objects, mem_list[0]->c_obj->id, mem_list[1]->c_obj->id, args_mem_loc[0], args_mem_loc[1], event_id, event_disclosed);

  EnqueueReadyQueue(dev, command, CL_FALSE);
  wait_events[event_id] = command;
}

void ClusterCompute::HandleMsgImageInfo(char* msg) {
  MessageCodec mc(msg);
  int tag = mc.GetTag();
  unsigned long cid = mc.GetCID();
  int dev_id = mc.GetInt();
  cl_mem_flags flags = mc.GetULong();
  cl_mem_object_type image_type = mc.GetUInt();

  cl_image_format image_formats[128];
  cl_uint num_image_formats;

  CLDevice* dev = (*devs)[dev_id];

  cl_int err = dev->GetSupportedImageFormats(flags, image_type, 128, image_formats, &num_image_formats);
  SNUCL_ECHECK(err);

  char image_info[8192];
  MessageCodec m(image_info);

  m.Init();
  m.SetUInt(&num_image_formats);
  for (cl_uint i = 0; i < num_image_formats; i++) {
    m.SetUInt(&image_formats[i].image_channel_order);
    m.SetUInt(&image_formats[i].image_channel_data_type);
  }

  MPI_Send(m.msg, sizeof(image_info), MPI_CHAR, 0, CLUSTER_TAG_IMAGE_INFO, MPI_COMM_WORLD); 
}

void ClusterCompute::HandleMsgImageCopy(char* msg) {
  MessageCodec mc(msg);
  int tag = mc.GetTag();
  unsigned long cid = mc.GetCID();
  int dev_dst_id = mc.GetInt();
  int dev_src_id = mc.GetInt();
  unsigned long mem_dst_id = GetMemID(dev_dst_id, &mc);
  unsigned long mem_src_id = GetMemID(dev_src_id, &mc);
  size_t src_origin[3];
  size_t dst_origin[3];
  size_t region[3];

  src_origin[0] = mc.GetULong();
  src_origin[1] = mc.GetULong();
  src_origin[2] = mc.GetULong();
  dst_origin[0] = mc.GetULong();
  dst_origin[1] = mc.GetULong();
  dst_origin[2] = mc.GetULong();
  region[0] = mc.GetULong();
  region[1] = mc.GetULong();
  region[2] = mc.GetULong();

  unsigned long event_id = mc.GetLong();
  bool event_disclosed = mc.GetBool();

  CLDevice* dev_src = (*devs)[dev_src_id];
  CLDevice* dev_dst = (*devs)[dev_dst_id];
  CLMem* mem_src = mems[mem_src_id];
  CLMem* mem_dst = mems[mem_dst_id];

  CLCommand* command = new CLCommand(CL_COMMAND_COPY_IMAGE);
  command->mem_src = mem_src;
  command->mem_dst = mem_dst;
  command->src_origin[0] = src_origin[0];
  command->src_origin[1] = src_origin[1];
  command->src_origin[2] = src_origin[2];
  command->dst_origin[0] = dst_origin[0];
  command->dst_origin[1] = dst_origin[1];
  command->dst_origin[2] = dst_origin[2];
  command->region[0] = region[0];
  command->region[1] = region[1];
  command->region[2] = region[2];
  EnqueueReadyQueue(dev_dst, command, CL_FALSE);
  wait_events[event_id] = command;

}

void ClusterCompute::HandleMsgImageRecv(char* msg) {
  MessageCodec mc(msg);
  int tag = mc.GetTag();
  unsigned long cid = mc.GetCID();
  int dev_id = mc.GetInt();
  int node_id = mc.GetInt();
  unsigned long mem_id = GetMemID(dev_id, &mc);
  size_t origin[3];
  size_t region[3];
  origin[0] = mc.GetULong();
  origin[1] = mc.GetULong();
  origin[2] = mc.GetULong();
  region[0] = mc.GetULong();
  region[1] = mc.GetULong();
  region[2] = mc.GetULong();
  size_t row_pitch = mc.GetULong();
  size_t slice_pitch = mc.GetULong();
  size_t size = mc.GetULong();
  size_t offset = mc.GetULong();
  unsigned long event_id = mc.GetULong();
  bool event_disclosed = mc.GetBool();

  SNUCL_DEBUG("MEM RECV DEV[%d] NODE[%d] MEM[%d] ORIGIN[%lu,%lu,%lu] REGION[%lu, %lu, %lu] ROW[%lu] SLICE[%lu] SIZE[%lu] OFFSET[%lu] EVENT[%lu]", dev_id, node_id, mem_id, origin[0], origin[1], origin[2], region[0], region[1], region[2], row_pitch, slice_pitch, size, offset, event_id);

  CLDevice* dev = (*devs)[dev_id];
  CLMem* mem = mems[mem_id];

  CLCommand* command = new CLCommand(CL_COMMAND_NOP);
  command->mem_dst = mem;
  command->off_dst = offset;
  command->src_origin[0] = 0;
  command->src_origin[1] = 0;
  command->src_origin[2] = 0;
  command->dst_origin[0] = origin[0];
  command->dst_origin[1] = origin[1];
  command->dst_origin[2] = origin[2];
  command->region[0] = region[0];
  command->region[1] = region[1];
  command->region[2] = region[2];
  command->src_row_pitch = row_pitch;
  command->src_slice_pitch = slice_pitch;
  command->cb = size;
  command->device = dev;

  command->misc[0] = CLUSTER_TAG_IMAGE_RECV;
  command->misc[1] = node_id;

  EnqueueReadyQueue(dev, command, CL_FALSE);
  wait_events[event_id] = command;
}

void ClusterCompute::HandleMsgImageSend(char* msg) {
  MessageCodec mc(msg);
  int tag = mc.GetTag();
  unsigned long cid = mc.GetCID();
  int dev_id = mc.GetInt();
  int node_id = mc.GetInt();
  unsigned long mem_id = GetMemID(dev_id, &mc);
  size_t origin[3];
  size_t region[3];
  origin[0] = mc.GetULong();
  origin[1] = mc.GetULong();
  origin[2] = mc.GetULong();
  region[0] = mc.GetULong();
  region[1] = mc.GetULong();
  region[2] = mc.GetULong();
  size_t row_pitch = mc.GetULong();
  size_t slice_pitch = mc.GetULong();
  size_t size = mc.GetULong();
  size_t offset = mc.GetULong();
  unsigned long event_id = mc.GetULong();
  bool event_disclosed = mc.GetBool();

  SNUCL_DEBUG("MEM SEND DEV[%d] NODE[%d] MEM[%d] ORIGIN[%lu,%lu,%lu] REGION[%lu, %lu, %lu] ROW[%lu] SLICE[%lu] SIZE[%lu] OFFSET[%lu] EVENT[%lu]", dev_id, node_id, mem_id, origin[0], origin[1], origin[2], region[0], region[1], region[2], row_pitch, slice_pitch, size, offset, event_id);

  CLDevice* dev = (*devs)[dev_id];
  CLMem* mem = mems[mem_id];

  /*
  if (dev->type == CL_DEVICE_TYPE_CPU) {
    SNUCL_ERROR("%s", "NOT YET");
  } else if (dev->type == CL_DEVICE_TYPE_GPU) {
  */
    CLCommand* command = new CLCommand(CL_COMMAND_READ_IMAGE);
    command->mem_src = mem;
    command->src_origin[0] = origin[0];
    command->src_origin[1] = origin[1];
    command->src_origin[2] = origin[2];
    command->region[0] = region[0];
    command->region[1] = region[1];
    command->region[2] = region[2];
    command->dst_row_pitch = row_pitch;
    command->dst_slice_pitch = slice_pitch;
    command->off_src = offset;
    command->cb = size;
    command->ptr = (void*) ((size_t) mem->space_host + offset);

    command->misc[0] = CLUSTER_TAG_IMAGE_SEND;
    command->misc[1] = node_id;

    EnqueueReadyQueue(dev, command, CL_FALSE);
    wait_events[event_id] = command;
    /*
  } else {
    SNUCL_ERROR("UNSUPPORT DEVICE TYPE [%lx]", dev->type);
    return;
  }
  */
}

void ClusterCompute::HandleMsgImageBuffer(char* msg) {
  MessageCodec mc(msg);
  int tag = mc.GetTag();
  unsigned long cid = mc.GetCID();
  int dev_dst_id = mc.GetInt();
  int dev_src_id = mc.GetInt();
  unsigned long mem_dst_id = GetMemID(dev_dst_id, &mc);
  unsigned long mem_src_id = GetMemID(dev_src_id, &mc);
  size_t src_origin[3];
  size_t region[3];

  src_origin[0] = mc.GetULong();
  src_origin[1] = mc.GetULong();
  src_origin[2] = mc.GetULong();
  region[0] = mc.GetULong();
  region[1] = mc.GetULong();
  region[2] = mc.GetULong();

  size_t off_dst = mc.GetULong();
  unsigned long event_id = mc.GetLong();
  bool event_disclosed = mc.GetBool();

  CLDevice* dev_src = (*devs)[dev_src_id];
  CLDevice* dev_dst = (*devs)[dev_dst_id];
  CLMem* mem_src = mems[mem_src_id];
  CLMem* mem_dst = mems[mem_dst_id];

  CLCommand* command = new CLCommand(CL_COMMAND_COPY_IMAGE_TO_BUFFER);
  command->mem_src = mem_src;
  command->mem_dst = mem_dst;
  command->src_origin[0] = src_origin[0];
  command->src_origin[1] = src_origin[1];
  command->src_origin[2] = src_origin[2];
  command->region[0] = region[0];
  command->region[1] = region[1];
  command->region[2] = region[2];
  command->off_dst = off_dst;
  EnqueueReadyQueue(dev_dst, command, CL_FALSE);
  wait_events[event_id] = command;
}

void ClusterCompute::HandleMsgBufferImage(char* msg) {
  MessageCodec mc(msg);
  int tag = mc.GetTag();
  unsigned long cid = mc.GetCID();
  int dev_dst_id = mc.GetInt();
  int dev_src_id = mc.GetInt();
  unsigned long mem_dst_id = GetMemID(dev_dst_id, &mc);
  unsigned long mem_src_id = GetMemID(dev_src_id, &mc);
  size_t dst_origin[3];
  size_t region[3];

  dst_origin[0] = mc.GetULong();
  dst_origin[1] = mc.GetULong();
  dst_origin[2] = mc.GetULong();
  region[0] = mc.GetULong();
  region[1] = mc.GetULong();
  region[2] = mc.GetULong();

  size_t off_src = mc.GetULong();
  unsigned long event_id = mc.GetLong();
  bool event_disclosed = mc.GetBool();

  CLDevice* dev_src = (*devs)[dev_src_id];
  CLDevice* dev_dst = (*devs)[dev_dst_id];
  CLMem* mem_src = mems[mem_src_id];
  CLMem* mem_dst = mems[mem_dst_id];

  CLCommand* command = new CLCommand(CL_COMMAND_COPY_BUFFER_TO_IMAGE);
  command->mem_src = mem_src;
  command->mem_dst = mem_dst;
  command->dst_origin[0] = dst_origin[0];
  command->dst_origin[1] = dst_origin[1];
  command->dst_origin[2] = dst_origin[2];
  command->region[0] = region[0];
  command->region[1] = region[1];
  command->region[2] = region[2];
  command->off_src = off_src;
  EnqueueReadyQueue(dev_dst, command, CL_FALSE);
  wait_events[event_id] = command;
}

void ClusterCompute::AllocMem(int dev_id, unsigned long mem_id, cl_mem_flags flags, size_t size, bool image, cl_image_format* image_format, cl_image_desc* image_desc, bool sub, unsigned long p_mem_id, cl_buffer_create_type types, cl_buffer_region* buf_create_info) {
  SNUCL_DEBUG("MEM ALLOC DEV[%d] MEM[%lu] FLAGS[%lx] SIZE[%lu]", dev_id, mem_id, flags, size);

  if (!mems.count(mem_id)) {
    CLMem* mem = new CLMem(ctx, flags, size, NULL, false);
    if (image) {
      size_t size = 0;
      size_t elem_size = 0;
      size_t row_pitch = 0;
      size_t slice_pitch = 0;
      int channels = 4;
      int errcode_ret = 0;

      GetImageDesc(image_format, image_desc, &size, &elem_size, &row_pitch, &slice_pitch, &channels, &errcode_ret);
      mem->is_image = true;
      mem->image_format = *image_format;
      mem->image_desc = *image_desc;
      mem->image_row_pitch = row_pitch;
      mem->image_slice_pitch = slice_pitch;
      mem->image_elem_size = elem_size;
      mem->image_channels = channels;
    } else if (sub) {
      if (!mems.count(p_mem_id)) SNUCL_ERROR("NO PARENT MEM[%d]", p_mem_id);
      CLMem* p_mem = mems[p_mem_id];
      mem->parent = p_mem;
      mem->is_sub = true;
      mem->sub_flags = flags;
      mem->create_type = types;
      mem->buf_create_info = *buf_create_info;
      mem->space_host = (void*) ((size_t) p_mem->space_host + mem->buf_create_info.origin);
    }
    mems[mem_id] = mem;
  }

  CLDevice* dev = (*devs)[dev_id];
  CLMem* mem = mems[mem_id];
  if (dev->type == CL_DEVICE_TYPE_CPU) {
    if (!mem->space_host) mem->space_host = memalign(4096, size);
    dev->AllocBuffer(mem);
  } else if (dev->type == CL_DEVICE_TYPE_GPU) {
    if (!mem->space_host) mem->space_host = memalign(4096, size);
  } else {
    SNUCL_ERROR("UNSUPPORT DEVICE TYPE [%lx]", dev->type);
    return;
  }
}

unsigned long ClusterCompute::GetMemID(int dev_id, MessageCodec* mc) {
  unsigned long mem_id = mc->GetULong();
  if (mem_id != 0UL) return mem_id;
  
  mem_id = mc->GetULong();
  cl_mem_flags flags = mc->GetULong();
  flags = flags & ~(CL_MEM_USE_HOST_PTR | CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR);
  size_t size = mc->GetULong();
  bool is_image = mc->GetBool();
  bool is_sub = mc->GetBool();
  cl_image_format image_format;
  cl_image_desc image_desc;

  unsigned long p_mem_id;
  cl_buffer_create_type types;
  cl_buffer_region buf_create_info;

  if (is_image) {
    image_format.image_channel_order = mc->GetUInt();
    image_format.image_channel_data_type = mc->GetUInt();

    image_desc.image_type = mc->GetUInt();
    image_desc.image_width = mc->GetULong();
    image_desc.image_height = mc->GetULong();
    image_desc.image_depth = mc->GetULong();
    image_desc.image_array_size = mc->GetULong();
    image_desc.image_row_pitch = mc->GetULong();
    image_desc.image_slice_pitch = mc->GetULong();
    image_desc.num_mip_levels = mc->GetUInt();
    image_desc.num_samples = mc->GetUInt();
  } else if (is_sub) {
    p_mem_id = mc->GetULong();
    types = mc->GetUInt();
    buf_create_info.origin = mc->GetULong();
    buf_create_info.size = mc->GetULong();
  }

  AllocMem(dev_id, mem_id, flags, size, is_image, &image_format, &image_desc, is_sub, p_mem_id, types, &buf_create_info);
  return mem_id;
}

unsigned long ClusterCompute::GetSamplerID(int dev_id, MessageCodec* mc) {
  unsigned long sampler_id = mc->GetULong();
  if (sampler_id != 0UL) return sampler_id;

  sampler_id = mc->GetULong();
  cl_bool normalized_coords = mc->GetUInt();
  cl_addressing_mode addressing_mode = mc->GetUInt();
  cl_filter_mode filter_mode = mc->GetUInt();


  if (!samplers.count(sampler_id)) {
    CLSampler* sampler = new CLSampler(ctx, normalized_coords, addressing_mode, filter_mode);
    samplers[sampler_id] = sampler;
  }
  return sampler_id;
}

ClusterSync* ClusterCompute::MakeCS(unsigned long event_id) {
  ClusterSync* cs = new ClusterSync();
  cs->event_id = event_id;
  return cs;
}

void ClusterCompute::CheckCompleteEvents() {
  for (map<unsigned long, CLCommand*>::iterator it = wait_events.begin(); it != wait_events.end(); ++it) {
    unsigned long event_id = it->first;
    CLCommand* command = it->second;
    CLEvent* event = command->event;
    if (event->status != CL_COMPLETE) continue;
    switch (command->type) {
      case CL_COMMAND_NDRANGE_KERNEL:
      case CL_COMMAND_NATIVE_KERNEL:
        {
          if (command->event->disclosed) {
            mpi_ret = MPI_Send(&event_id, sizeof(event_id), MPI_CHAR, 0, CLUSTER_TAG_EVENT_WAIT(event_id), MPI_COMM_WORLD);
          }
        }
        break;
      case CL_COMMAND_READ_BUFFER:
        {
          if (command->misc[0] == CLUSTER_TAG_MEM_SEND) {
            MPI_Request mpi_request;
            mpi_ret = MPI_Isend(command->ptr, (int) command->cb, MPI_CHAR, command->misc[1], CLUSTER_TAG_MEM_SEND_BODY(event_id), MPI_COMM_WORLD, &mpi_request);
          } else if (command->misc[0] == CLUSTER_TAG_MEM_COPY) {
            if (command->sub_command) {
              CLCommand* sub_command = command->sub_command;
              EnqueueReadyQueue(sub_command->device, sub_command, CL_FALSE);
              wait_events.erase(it--);
              wait_events[event_id] = sub_command;
              return;
            } else {
              mpi_ret = MPI_Send(&event_id, sizeof(event_id), MPI_CHAR, 0, CLUSTER_TAG_EVENT_WAIT(event_id), MPI_COMM_WORLD);
            }
          } else {
            SNUCL_ERROR("%s", "SOMETHING ERROR");
          }
        }
        break;
      case CL_COMMAND_WRITE_BUFFER:
        {
          mpi_ret = MPI_Send(&event_id, sizeof(event_id), MPI_CHAR, 0, CLUSTER_TAG_EVENT_WAIT(event_id), MPI_COMM_WORLD);
        }
        break;
      case CL_COMMAND_COPY_BUFFER:
        {
          mpi_ret = MPI_Send(&event_id, sizeof(event_id), MPI_CHAR, 0, CLUSTER_TAG_EVENT_WAIT(event_id), MPI_COMM_WORLD);
        }
        break;
      case CL_COMMAND_MARKER:
        {
          mpi_ret = MPI_Send(&event_id, sizeof(event_id), MPI_CHAR, 0, CLUSTER_TAG_EVENT_WAIT(event_id), MPI_COMM_WORLD);
        }
        break;
      case CL_COMMAND_NOP:
        {
          if (command->misc[0] == CLUSTER_TAG_MEM_SEND) {
            CLMem* mem_src = command->mem_src; 
            void* m = mem_src->dev_specific[command->device];
            MPI_Request mpi_request;
            mpi_ret = MPI_Isend((void*) ((size_t) m + command->off_src), (int) command->cb, MPI_CHAR, command->misc[1], CLUSTER_TAG_MEM_SEND_BODY(event_id), MPI_COMM_WORLD, &mpi_request);
          } else if (command->misc[0] == CLUSTER_TAG_MEM_RECV) {
            CLDevice* dev = command->device;
            CLMem* mem = command->mem_dst;
            ClusterSync* cs = MakeCS(event_id);
            size_t offset = command->off_dst;
            void* buf = NULL;
            if (dev->type == CL_DEVICE_TYPE_CPU) {
              void* m = mem->dev_specific[dev];
              buf = (void*) ((size_t) m + command->off_dst);
            } else if (dev->type == CL_DEVICE_TYPE_GPU) {
              buf = (void*) ((size_t) mem->space_host + command->off_dst);
              CLCommand* w_command;
              if (mem->is_image) {
                w_command = new CLCommand(CL_COMMAND_WRITE_IMAGE); 
                w_command->mem_dst = mem;
                w_command->dst_origin[0] = 0;
                w_command->dst_origin[1] = 0;
                w_command->dst_origin[2] = 0;
                w_command->region[0] = mem->image_desc.image_width;
                w_command->region[1] = mem->image_desc.image_height;
                w_command->region[2] = mem->image_desc.image_depth;
                w_command->src_row_pitch = mem->image_desc.image_row_pitch;
                w_command->src_slice_pitch = mem->image_desc.image_row_pitch;
                w_command->ptr = buf;
              } else {
                w_command = new CLCommand(CL_COMMAND_WRITE_BUFFER); 
                w_command->mem_dst = mem;
                w_command->off_dst = command->off_dst;
                w_command->cb = command->cb;
                w_command->ptr = buf;
              }
              cs->command = w_command;
              cs->device = dev;
              mem->space_host_valid = true;
            } else {
              SNUCL_ERROR("UNSUPPORT DEVICE TYPE [%lx]", dev->type);
              return;
            }
            mpi_ret = MPI_Irecv(buf, command->cb, MPI_CHAR, command->misc[1], CLUSTER_TAG_MEM_RECV_BODY(event_id), MPI_COMM_WORLD, &cs->mpi_request_wait);
            wait_events_cs.push_back(cs);
          } else if (command->misc[0] == CLUSTER_TAG_IMAGE_RECV) {
            CLDevice* dev = command->device;
            CLMem* mem = command->mem_dst;
            ClusterSync* cs = MakeCS(event_id);
            void* buf = (void*) ((size_t) mem->space_host + command->off_dst);
            CLCommand* w_command;
            w_command = new CLCommand(CL_COMMAND_WRITE_IMAGE); 
            w_command->mem_dst = mem;
            w_command->dst_origin[0] = command->dst_origin[0];
            w_command->dst_origin[1] = command->dst_origin[1];
            w_command->dst_origin[2] = command->dst_origin[2];
            w_command->region[0] = command->region[0];
            w_command->region[1] = command->region[1];
            w_command->region[2] = command->region[2];
            w_command->src_row_pitch = command->src_row_pitch;
            w_command->src_slice_pitch = command->src_slice_pitch;
            w_command->ptr = buf;
            cs->command = w_command;
            cs->device = dev;
            mem->space_host_valid = true;
            mpi_ret = MPI_Irecv(buf, command->cb, MPI_CHAR, command->misc[1], CLUSTER_TAG_MEM_RECV_BODY(event_id), MPI_COMM_WORLD, &cs->mpi_request_wait);
            wait_events_cs.push_back(cs);
          } else if (command->misc[0] == CLUSTER_TAG_MEM_RECV_RECT) {
            CLDevice* dev = command->device;
            CLMem* mem = command->buffer;
            SNUCL_DEBUG("MEM_RECV SIZE[%lu] DEV[%lu] MEM[%lu] FROM[%d]", command->cb, dev->id, mem->id, command->misc[1]);
            ClusterSync* cs = MakeCS(event_id);
            size_t offset = 0;

            void* buf = malloc(command->cb);
            CLCommand* w_command = new CLCommand(CL_COMMAND_WRITE_BUFFER_RECT);
            w_command->buffer = mem;
            w_command->src_origin[0] = command->src_origin[0];
            w_command->src_origin[1] = command->src_origin[1];
            w_command->src_origin[2] = command->src_origin[2];
            w_command->dst_origin[0] = command->dst_origin[0];
            w_command->dst_origin[1] = command->dst_origin[1];
            w_command->dst_origin[2] = command->dst_origin[2];
            w_command->region[0] = command->region[0];
            w_command->region[1] = command->region[1];
            w_command->region[2] = command->region[2];
            w_command->src_row_pitch = command->src_row_pitch;
            w_command->src_slice_pitch = command->src_slice_pitch;
            w_command->dst_row_pitch = command->dst_row_pitch;
            w_command->dst_slice_pitch = command->dst_slice_pitch;
            w_command->ptr = buf;
            w_command->cb = command->size;
            cs->command = w_command;
            cs->device = dev;
            mem->space_host_valid = true;

            mpi_ret = MPI_Irecv(buf, command->cb, MPI_CHAR, command->misc[1], CLUSTER_TAG_MEM_RECV_BODY(event_id), MPI_COMM_WORLD, &cs->mpi_request_wait);
            wait_events_cs.push_back(cs);
          } 
        }
        break;
      case CL_COMMAND_READ_IMAGE:
        {
          if (command->misc[0] == CLUSTER_TAG_IMAGE_SEND) {
            MPI_Request mpi_request;
            mpi_ret = MPI_Isend(command->ptr, (int) command->cb, MPI_CHAR, command->misc[1], CLUSTER_TAG_MEM_SEND_BODY(event_id), MPI_COMM_WORLD, &mpi_request);
          }
        }
        break;
      case CL_COMMAND_COPY_IMAGE:
      case CL_COMMAND_COPY_BUFFER_RECT:
      case CL_COMMAND_COPY_BUFFER_TO_IMAGE:
      case CL_COMMAND_COPY_IMAGE_TO_BUFFER:
        {
          mpi_ret = MPI_Send(&event_id, sizeof(event_id), MPI_CHAR, 0, CLUSTER_TAG_EVENT_WAIT(event_id), MPI_COMM_WORLD);
        }
        break;
      case CL_COMMAND_READ_BUFFER_RECT:
        {
          if (command->misc[0] == CLUSTER_TAG_MEM_SEND_RECT) {
            MPI_Request mpi_request;
            mpi_ret = MPI_Isend(command->ptr, (int) command->cb, MPI_CHAR, command->misc[1], CLUSTER_TAG_MEM_SEND_BODY(event_id), MPI_COMM_WORLD, &mpi_request);
          }
        }
        break;
      default:
        {
          SNUCL_ERROR("UNSUPPORT COMMAND TYPE [%X]", command->type);
          return;
        }
    }
    wait_events.erase(it--);
  }
}

void ClusterCompute::CheckCompleteEventsCS() {
  for (vector<ClusterSync*>::iterator it = wait_events_cs.begin(); it != wait_events_cs.end(); ++it) {
    ClusterSync* cs = *it;
    int flag = 0;
    mpi_ret = MPI_Test(&cs->mpi_request_wait, &flag, &cs->mpi_status);
    if (!flag) continue;

    if (cs->command) EnqueueReadyQueue(cs->device, cs->command, CL_FALSE);

    unsigned long event_id = cs->event_id;
    mpi_ret = MPI_Send(&event_id, sizeof(event_id), MPI_CHAR, 0, CLUSTER_TAG_EVENT_WAIT(event_id), MPI_COMM_WORLD);
    wait_events_cs.erase(it--);
    delete cs;
  }
}

void ClusterCompute::EnqueueReadyQueue(CLDevice* device, CLCommand* command, cl_bool blocking) {
  device->EnqueueReadyQueue(command);
}

