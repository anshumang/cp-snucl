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

#include <arch/cluster/ClusterDevice.h>

SNUCL_DEBUG_HEADER("ClusterDevice");

int ClusterCLDevice::CreateDevices(vector<CLDevice*>* devices, int node_id, int dev_id, cl_device_type type) {
  devices->push_back(new ClusterCLDevice(node_id, dev_id, type));
  return 1;
}

ClusterCLDevice::ClusterCLDevice(int node_id, int dev_id, cl_device_type type) : CLDevice(type) {
  this->node_id = node_id;
  this->dev_id = dev_id;
  CID = 0;

  InitInfo();
}

ClusterCLDevice::~ClusterCLDevice() {
}

void ClusterCLDevice::Init() {
}

void ClusterCLDevice::InitInfo() {
  MessageCodec mc;
  mc.SetTag(CLUSTER_TAG_DEV_INFO);
  mc.SetCID(NewCID());
  mc.SetInt(&dev_id);

  MPI_Status mpi_status;

  char dev_info[8192];
  mpi_ret = MPI_Sendrecv(mc.msg, MESSAGE_COMMAND_SIZE, MPI_CHAR, node_id, CLUSTER_TAG_COMMAND, dev_info, sizeof(dev_info), MPI_CHAR, node_id, CLUSTER_TAG_DEV_INFO, MPI_COMM_WORLD, &mpi_status);

  MessageCodec m(dev_info);

  vendor_id = m.GetUInt();
  max_compute_units = m.GetUInt();
  max_work_item_dimensions = m.GetUInt();
  m.Get(max_work_item_sizes, sizeof(size_t) * 3);
  max_work_group_size = m.GetULong();

  preferred_vector_width_char = m.GetUInt();
  preferred_vector_width_short  = m.GetUInt();
  preferred_vector_width_int = m.GetUInt();
  preferred_vector_width_long = m.GetUInt();
  preferred_vector_width_float = m.GetUInt();
  preferred_vector_width_double =  m.GetUInt();
  preferred_vector_width_half = m.GetUInt();
  native_vector_width_char = m.GetUInt();
  native_vector_width_short = m.GetUInt();
  native_vector_width_int = m.GetUInt();
  native_vector_width_long = m.GetUInt();
  native_vector_width_float = m.GetUInt();
  native_vector_width_double = m.GetUInt();
  native_vector_width_half = m.GetUInt();
  max_clock_frequency = m.GetUInt();
  address_bits = m.GetUInt();
  max_mem_alloc_size = m.GetULong();
  image_support = m.GetUInt();
  max_read_image_args = m.GetUInt();
  max_write_image_args = m.GetUInt();
  image2d_max_width = m.GetULong();
  image2d_max_height = m.GetULong();
  image3d_max_width = m.GetULong();
  image3d_max_height = m.GetULong();
  image3d_max_depth = m.GetULong();
  image_max_buffer_size = m.GetULong();
  image_max_array_size = m.GetULong();
  max_samplers = m.GetUInt();
  max_parameter_size = m.GetULong();
  mem_base_addr_align = m.GetUInt();
  min_data_type_align_size = m.GetUInt();
  single_fp_config = m.GetULong();
  double_fp_config = m.GetULong();
  global_mem_cache_type = m.GetUInt();
  global_mem_cacheline_size = m.GetUInt();
  global_mem_cache_size = m.GetULong();
  global_mem_size = m.GetULong();
  max_constant_buffer_size = m.GetULong();
  max_constant_args = m.GetUInt();
  local_mem_type = m.GetUInt();
  local_mem_size = m.GetULong();
  error_correction_support = m.GetUInt();
  host_unified_memory = m.GetUInt();
  profiling_timer_resolution = m.GetULong();
  endian_little = m.GetUInt();
  available = m.GetUInt();
  compiler_available = m.GetUInt();
  linker_available = m.GetUInt();
  execution_capabilities = m.GetULong();
  queue_properties = m.GetULong();
  m.Get(built_in_kernels, 64);
  m.Get(name, 64);
  m.Get(vendor, 64);
  m.Get(driver_version, 64);
  m.Get(profile, 32);
  m.Get(device_version, 32);
  m.Get(openclc_version, 32);
  m.Get(device_extensions, 1024);
  printf_buffer_size = m.GetULong();
  preferred_interop_user_sync = m.GetUInt();
  //parent_device = m.GetUInt();
  partition_max_sub_devices = m.GetUInt();
  m.Get(partition_properties, sizeof(cl_device_partition_property) * 3);
  num_partition_properties = m.GetULong();
  affinity_domain = m.GetULong();
  //partition_type = m.GetUInt();
  partition_type_len = m.GetUInt();
}

void ClusterCLDevice::LaunchKernel(CLCommand* command) {
  SendWaitCommand(command);
}

void ClusterCLDevice::NativeKernel(CLCommand* command) {
  SendWaitCommand(command);
}

void ClusterCLDevice::ReadBuffer(CLCommand* command) {
  SNUCL_CHECK();
  CLMem* mem_src = command->mem_src;
  CLEvent* event = command->event;

  if (mem_src->alloc_host && mem_src->EmptyLatest()) {
    memcpy(command->ptr, (void*) ((size_t) mem_src->space_host + command->off_src), command->cb);
    event->Complete();
    return;
  }

  SendCommand(command);

  ClusterSync *cs = MakeCS(event);
  mpi_ret = MPI_Irecv(command->ptr, (int) command->cb, MPI_CHAR, node_id, CLUSTER_TAG_MEM_SEND_BODY(event->id), MPI_COMM_WORLD, &cs->mpi_request_wait);

  issuer->EnqueueRunningCommand(command);
  SNUCL_CHECK();
}

void ClusterCLDevice::WriteBuffer(CLCommand* command) {
  CLEvent* event = command->event;
  ClusterSync *cs = MakeCS(event);
  SendCommand(command);
  mpi_ret = MPI_Isend(command->ptr, (int) command->cb, MPI_CHAR, node_id, CLUSTER_TAG_MEM_RECV_BODY(event->id), MPI_COMM_WORLD, &cs->mpi_request);
  mpi_ret = MPI_Irecv(&cs->event_id_ret, sizeof(cs->event_id_ret), MPI_CHAR, node_id, CLUSTER_TAG_EVENT_WAIT(event->id), MPI_COMM_WORLD, &cs->mpi_request_wait);

  issuer->EnqueueRunningCommand(command);
}

void ClusterCLDevice::CopyBuffer(CLCommand* command) {
  SendWaitCommand(command);
}

void ClusterCLDevice::ReadBufferRect(CLCommand* command) {
  SNUCL_CHECK();
  CLEvent* event = command->event;

  SendCommand(command);

#if 0
  ClusterSync *cs = MakeCS(event);
  mpi_ret = MPI_Irecv(command->ptr, (int) command->cb, MPI_CHAR, node_id, CLUSTER_TAG_MEM_SEND_BODY(event->id), MPI_COMM_WORLD, &cs->mpi_request_wait);

  issuer->EnqueueRunningCommand(command);
#else
  void* buf = malloc(command->cb);
  MPI_Status mpi_status;
  mpi_ret = MPI_Recv(buf, (int) command->cb, MPI_CHAR, node_id, CLUSTER_TAG_MEM_SEND_BODY(event->id), MPI_COMM_WORLD, &mpi_status);
  BufferRectCommon(command->src_origin, command->dst_origin, command->region, command->src_row_pitch, command->src_slice_pitch, command->dst_row_pitch, command->dst_slice_pitch, buf, command->ptr);
  command->event->Complete();
#endif
}

void ClusterCLDevice::WriteBufferRect(CLCommand* command) {
  SNUCL_CHECK();
  CLEvent* event = command->event;
  ClusterSync *cs = MakeCS(event);
  SendCommand(command);
  SNUCL_INFO("COMMAND PTR[%p]", command->ptr);
  mpi_ret = MPI_Isend(command->ptr, (int) command->cb, MPI_CHAR, node_id, CLUSTER_TAG_MEM_RECV_BODY(event->id), MPI_COMM_WORLD, &cs->mpi_request);
  mpi_ret = MPI_Irecv(&cs->event_id_ret, sizeof(cs->event_id_ret), MPI_CHAR, node_id, CLUSTER_TAG_EVENT_WAIT(event->id), MPI_COMM_WORLD, &cs->mpi_request_wait);

  issuer->EnqueueRunningCommand(command);
  SNUCL_CHECK();
}

void ClusterCLDevice::CopyBufferRect(CLCommand* command) {
  SendWaitCommand(command);
}

void ClusterCLDevice::SendBuffer(CLCommand* command) {
  SendCommand(command);
  if (command->free) {
    delete command;
  }
}

void ClusterCLDevice::RecvBuffer(CLCommand* command) {
  SendWaitCommand(command);
}

void ClusterCLDevice::BcastBuffer(CLCommand* command) {
  CLMem* mem_src = command->mem_src;
  CLMem* mem_dst = command->mem_dst;
  size_t off_src = command->off_src;
  size_t off_dst = command->off_dst;
  size_t size = command->cb;
  CLEvent* event = command->event;

  CLDevice* dev_src = mem_src->FrontLatest();

  CheckBuffer(mem_dst);

  MessageCodec mc(command);
  mc.SetTag(CLUSTER_TAG_MEM_BCAST);
  mc.SetCID(NewCID());
  mc.SetInt(&dev_id);
  mc.SetInt(&dev_src->node_id);
  mc.SetULong(&mem_src->id);
  mc.SetULong(&mem_dst->id);
  mc.SetULong(&off_src);
  mc.SetULong(&off_dst);
  mc.SetULong(&size);
  mc.SetULong(&event->id);
  mc.SetBool(&event->disclosed);
  mc.Finish(command);
  
  SendWaitCommand(command);
}

void ClusterCLDevice::AlltoallBuffer(CLCommand* command) {
  CLMem* mem_src = command->mem_src;
  CLMem* mem_dst = command->mem_dst;
  size_t off_src = command->off_src;
  size_t off_dst = command->off_dst;
  size_t size = command->cb;
  CLEvent* event = command->event;

  CheckBuffer(mem_dst);

  MessageCodec mc(command);
  mc.SetTag(CLUSTER_TAG_MEM_ALLTOALL);
  mc.SetCID(NewCID());
  mc.SetInt(&dev_id);
  mc.SetULong(&mem_src->id);
  mc.SetULong(&mem_dst->id);
  mc.SetULong(&off_src);
  mc.SetULong(&off_dst);
  mc.SetULong(&size);
  mc.SetULong(&event->id);
  mc.SetBool(&event->disclosed);
  mc.Finish(command);
  
  SendWaitCommand(command);
}

void ClusterCLDevice::Marker(CLCommand* command) {
  SendWaitCommand(command);
}

void ClusterCLDevice::PrepareBuildProgram(CLCommand* command) {
  CLProgram* program = command->program;
  size_t len_src = strlen(program->src);
  size_t len_options = program->options ? strlen(program->options) : 0;

  SNUCL_INFO("%lu , %lu \n %s", len_options, len_src, program->src);
  
  MessageCodec mc(command);
  mc.SetTag(CLUSTER_TAG_PROGRAM_BUILD);
  mc.SetCID(NewCID());
  mc.SetInt(&dev_id);
  mc.SetULong(&program->id);
  mc.SetULong(&len_src);
  mc.SetULong(&len_options);
  mc.SetULong(&command->event->id);
  mc.Finish(command);
}

void ClusterCLDevice::BuildProgram(CLCommand* command) {
  CLProgram* program = command->program;
  size_t len_src = strlen(program->src);
  size_t len_options = program->options ? strlen(program->options) : 0;
  SendCommand(command);
  mpi_ret = MPI_Send((void*) program->src, (int) len_src, MPI_CHAR, node_id, CLUSTER_TAG_PROGRAM_BUILD, MPI_COMM_WORLD);
  if (len_options) mpi_ret = MPI_Send((void*) program->options, (int) len_options, MPI_CHAR, node_id, CLUSTER_TAG_PROGRAM_BUILD, MPI_COMM_WORLD);
  WaitCommand(command);

  program->buildStatus[this] = CL_BUILD_SUCCESS;
	if(command->program->buildCallbackStack.size()) {
		for (vector<ProgramCallback*>::iterator it = command->program->buildCallbackStack.begin(); it != command->program->buildCallbackStack.end(); ++it) {
			(*it)->pfn_notify(&command->program->st_obj, (*it)->user_data);
		}
	}
}

void ClusterCLDevice::AllocBuffer(CLMem* mem) {
  pthread_mutex_lock(&mem->mutex_dev_specific);
  mem->dev_specific[this] = (void*) this;
  pthread_mutex_unlock(&mem->mutex_dev_specific);

  MessageCodec mc;
  mc.SetInt(&dev_id);
  mc.SetULong(&mem->id);
  mc.SetULong(&mem->flags);
  mc.SetULong(&mem->size);

  SendMsg(mc.msg);
}

void ClusterCLDevice::PrepareFreeBuffer(CLCommand* command) {
  MessageCodec mc(command);
  mc.SetTag(CLUSTER_TAG_MEM_FREE);
  mc.SetCID(NewCID());
  mc.SetInt(&dev_id);
  mc.SetULong(&command->mem_src->id);
  mc.SetULong(&command->event->id);
}

void ClusterCLDevice::FreeBuffer(CLCommand* command) {
  SendWaitCommand(command);
}

void ClusterCLDevice::PrepareLaunchKernel(CLCommand* command) {
  MessageCodec mc(command);
  mc.SetTag(CLUSTER_TAG_KERNEL_LAUNCH);
  mc.SetCID(NewCID());
  mc.SetInt(&dev_id);

  size_t len_kernel_name = strlen(command->kernel->name);
  mc.SetULong(&command->program->id);
  mc.SetULong(&command->kernel->id);
  mc.SetULong(&len_kernel_name);
  mc.Set(command->kernel->name, len_kernel_name);

  map<cl_uint, CLKernelArg*>* args = command->kernel_args;
  int num_arg = args->size();
  mc.SetInt(&num_arg);

  for (cl_uint i = 0; i < num_arg; ++i) {
    CLKernelArg* arg = (*args)[i];
    int arg_type = 0;
    if (arg->mem) {
      arg_type = 0;
      mc.SetInt(&arg_type);
      SetMemID(arg->mem, &mc);
    } else if (arg->sampler) {
      arg_type = 1;
      mc.SetInt(&arg_type);
      SetSamplerID(arg->sampler, &mc);
    } else if (arg->local) {
      arg_type = 2;
      mc.SetInt(&arg_type);
      mc.SetULong(&arg->size);
    } else {
      arg_type = 3;
      mc.SetInt(&arg_type);
      mc.SetULong(&arg->size);
      mc.Set(&arg->value, arg->size);
    }
  }

  mc.SetInt(&command->work_dim);
  mc.SetULong(&command->gwo[0]);
  mc.SetULong(&command->gwo[1]);
  mc.SetULong(&command->gwo[2]);
  mc.SetULong(&command->gws[0]);
  mc.SetULong(&command->gws[1]);
  mc.SetULong(&command->gws[2]);
  mc.SetULong(&command->lws[0]);
  mc.SetULong(&command->lws[1]);
  mc.SetULong(&command->lws[2]);
  mc.SetULong(&command->event->id);
  mc.SetBool(&command->event->disclosed);
  mc.Finish(command);

  SNUCL_INFO("LAUNCH KERNEL MSG SIZE [%lu]", mc.Offset());
}

void ClusterCLDevice::PrepareNativeKernel(CLCommand* command) {
  size_t cb_args = (*command->kernel_args)[0]->size;
  void* args = (void*) (*command->kernel_args)[0]->value;
  cl_uint num_mem_objects = (*command->kernel_args)[1]->size / sizeof(cl_mem);
  cl_mem* mem_list = (cl_mem*) (*command->kernel_args)[1]->value;
  void** args_mem_loc = (void**) (*command->kernel_args)[2]->value;

  MessageCodec mc(command);
  mc.SetTag(CLUSTER_TAG_NATIVE_KERNEL);
  mc.SetCID(NewCID());
  mc.SetInt(&dev_id);

  mc.SetULong(&command->user_func);
  mc.SetULong(&cb_args);
  mc.Set(args, cb_args);
  mc.SetInt(&num_mem_objects);

  for (int i = 0; i < num_mem_objects; i++) {
    CLMem* mem = mem_list[i]->c_obj;
    SetMemID(mem, &mc);
  }

  for (int i = 0; i < num_mem_objects; i++) {
    size_t offset = (size_t) args_mem_loc[i] - (size_t) command->ptr;
    mc.SetULong(&offset);
  }

  mc.SetULong(&command->event->id);
  mc.SetBool(&command->event->disclosed);
  mc.Finish(command);
}

void ClusterCLDevice::PrepareReadBuffer(CLCommand* command) {
  SNUCL_CHECK();
  CLMem* mem_src = command->mem_src;
  size_t off_src = command->off_src;
  size_t size = command->cb;
  CLEvent* event = command->event;
  int node_id_host = 0;

  MessageCodec mc(command);
  mc.SetTag(CLUSTER_TAG_MEM_SEND);
  mc.SetCID(NewCID());
  mc.SetInt(&dev_id);
  mc.SetInt(&node_id_host);
  SetMemID(mem_src, &mc);
  mc.SetULong(&off_src);
  mc.SetULong(&size);
  mc.SetULong(&event->id);
  mc.SetBool(&event->disclosed);
  mc.Finish(command);
  SNUCL_CHECK();
}

void ClusterCLDevice::PrepareWriteBuffer(CLCommand* command) {
  CLMem* mem_dst = command->mem_dst;
  size_t off_dst = command->off_dst;
  size_t size = command->cb;
  CLEvent* event = command->event;
  int node_id_host = 0;

  MessageCodec mc(command);
  mc.SetTag(CLUSTER_TAG_MEM_RECV);
  mc.SetCID(NewCID());
  mc.SetInt(&dev_id);
  mc.SetInt(&node_id_host);
  SetMemID(mem_dst, &mc);
  mc.SetULong(&off_dst);
  mc.SetULong(&size);
  mc.SetULong(&event->id);
  mc.SetBool(&event->disclosed);
  mc.Finish(command);
}

void ClusterCLDevice::PrepareCopyBuffer(CLCommand* command) {
  CLDevice* dev_src = command->dev_src;
  CLMem* mem_src = command->mem_src;
  CLMem* mem_dst = command->mem_dst;
  size_t off_src = command->off_src;
  size_t off_dst = command->off_dst;
  size_t size = command->cb;
  CLEvent* event = command->event;

  MessageCodec mc(command);
  mc.SetTag(CLUSTER_TAG_MEM_COPY);
  mc.SetCID(NewCID());
  mc.SetInt(&dev_id);
  mc.SetInt(&dev_src->dev_id);
  SetMemID(mem_dst, &mc);
  SetMemID(mem_src, &mc);
  mc.SetULong(&off_dst);
  mc.SetULong(&off_src);
  mc.SetULong(&size);
  mc.SetULong(&event->id);
  mc.SetBool(&event->disclosed);
  mc.Finish(command);
}

void ClusterCLDevice::PrepareReadBufferRect(CLCommand* command) {
  SNUCL_CHECK();
  CLMem* mem_src = command->buffer;
  size_t src_origin[3] = { command->src_origin[0], command->src_origin[1], command->src_origin[2] };
  size_t dst_origin[3] = { command->dst_origin[0], command->dst_origin[1], command->dst_origin[2] };
  size_t region[3] = { command->region[0], command->region[1], command->region[2] };
  size_t src_row_pitch = command->src_row_pitch;
  size_t src_slice_pitch = command->src_slice_pitch;
  size_t dst_row_pitch = command->dst_row_pitch;
  size_t dst_slice_pitch = command->dst_slice_pitch;
  size_t size = mem_src->size;
  command->cb = size;
  CLEvent* event = command->event;
  int node_id_host = 0;

  MessageCodec mc(command);
  mc.SetTag(CLUSTER_TAG_MEM_SEND_RECT);
  mc.SetCID(NewCID());
  mc.SetInt(&dev_id);
  mc.SetInt(&node_id_host);
  SetMemID(mem_src, &mc);
  mc.SetULong(&src_origin[0]);
  mc.SetULong(&src_origin[1]);
  mc.SetULong(&src_origin[2]);
  mc.SetULong(&dst_origin[0]);
  mc.SetULong(&dst_origin[1]);
  mc.SetULong(&dst_origin[2]);
  mc.SetULong(&region[0]);
  mc.SetULong(&region[1]);
  mc.SetULong(&region[2]);
  mc.SetULong(&src_row_pitch);
  mc.SetULong(&src_slice_pitch);
  mc.SetULong(&dst_row_pitch);
  mc.SetULong(&dst_slice_pitch);
  mc.SetULong(&size);
  mc.SetULong(&event->id);
  mc.SetBool(&event->disclosed);
  mc.Finish(command);
  SNUCL_CHECK();
}

void ClusterCLDevice::PrepareWriteBufferRect(CLCommand* command) {
  SNUCL_CHECK();
  CLMem* mem_dst = command->buffer;
  size_t src_origin[3] = { command->src_origin[0], command->src_origin[1], command->src_origin[2] };
  size_t dst_origin[3] = { command->dst_origin[0], command->dst_origin[1], command->dst_origin[2] };
  size_t region[3] = { command->region[0], command->region[1], command->region[2] };
  size_t src_row_pitch = command->src_row_pitch;
  size_t src_slice_pitch = command->src_slice_pitch;
  size_t dst_row_pitch = command->dst_row_pitch;
  size_t dst_slice_pitch = command->dst_slice_pitch;
  size_t size_src_slice_pitch = src_slice_pitch == 0 ? region[1] * src_row_pitch : src_slice_pitch;
  size_t size = size_src_slice_pitch * (src_origin[2] + region[2]);
  SNUCL_INFO("WRITE RECT SIZE [%lu]", size);
  command->cb = size;
  CLEvent* event = command->event;
  int node_id_host = 0;

  MessageCodec mc(command);
  mc.SetTag(CLUSTER_TAG_MEM_RECV_RECT);
  mc.SetCID(NewCID());
  mc.SetInt(&dev_id);
  mc.SetInt(&node_id_host);
  SetMemID(mem_dst, &mc);
  mc.SetULong(&src_origin[0]);
  mc.SetULong(&src_origin[1]);
  mc.SetULong(&src_origin[2]);
  mc.SetULong(&dst_origin[0]);
  mc.SetULong(&dst_origin[1]);
  mc.SetULong(&dst_origin[2]);
  mc.SetULong(&region[0]);
  mc.SetULong(&region[1]);
  mc.SetULong(&region[2]);
  mc.SetULong(&src_row_pitch);
  mc.SetULong(&src_slice_pitch);
  mc.SetULong(&dst_row_pitch);
  mc.SetULong(&dst_slice_pitch);
  mc.SetULong(&size);
  mc.SetULong(&event->id);
  mc.SetBool(&event->disclosed);
  mc.Finish(command);
  SNUCL_CHECK();
}

void ClusterCLDevice::PrepareCopyBufferRect(CLCommand* command) {
  SNUCL_CHECK();
  CLDevice* dev_src = command->device;
  CLMem* mem_src = command->mem_src;
  CLMem* mem_dst = command->mem_dst;
  size_t src_origin[3] = { command->src_origin[0], command->src_origin[1], command->src_origin[2] };
  size_t dst_origin[3] = { command->dst_origin[0], command->dst_origin[1], command->dst_origin[2] };
  size_t region[3] = { command->region[0], command->region[1], command->region[2] };
  size_t src_row_pitch = command->src_row_pitch;
  size_t src_slice_pitch = command->src_slice_pitch;
  size_t dst_row_pitch = command->dst_row_pitch;
  size_t dst_slice_pitch = command->dst_slice_pitch;
  CLEvent* event = command->event;

  MessageCodec mc(command);
  mc.SetTag(CLUSTER_TAG_MEM_COPY_RECT);
  mc.SetCID(NewCID());
  mc.SetInt(&dev_id);
  mc.SetInt(&dev_src->dev_id);
  SetMemID(mem_dst, &mc);
  SetMemID(mem_src, &mc);
  mc.SetULong(&src_origin[0]);
  mc.SetULong(&src_origin[1]);
  mc.SetULong(&src_origin[2]);
  mc.SetULong(&dst_origin[0]);
  mc.SetULong(&dst_origin[1]);
  mc.SetULong(&dst_origin[2]);
  mc.SetULong(&region[0]);
  mc.SetULong(&region[1]);
  mc.SetULong(&region[2]);
  mc.SetULong(&src_row_pitch);
  mc.SetULong(&src_slice_pitch);
  mc.SetULong(&dst_row_pitch);
  mc.SetULong(&dst_slice_pitch);
  mc.SetULong(&event->id);
  mc.SetBool(&event->disclosed);
  mc.Finish(command);
  SNUCL_CHECK();
}

void ClusterCLDevice::PrepareSendBuffer(CLCommand* command) {
  CLDevice* dev_dst = command->dev_dst;
  CLMem* mem_src = command->mem_src;
  size_t off_src = command->off_src;
  size_t size = command->cb;

  MessageCodec mc(command);
  mc.SetTag(CLUSTER_TAG_MEM_SEND);
  mc.SetCID(NewCID());
  mc.SetInt(&dev_id);
  mc.SetInt(&dev_dst->node_id);
  SetMemID(mem_src, &mc);
  mc.SetULong(&off_src);
  mc.SetULong(&size);
  mc.SetULong(&command->misc[0]);
  mc.Finish(command);
}

void ClusterCLDevice::PrepareRecvBuffer(CLCommand* command) {
  CLDevice* dev_src = command->dev_src;
  CLMem* mem_dst = command->mem_dst;
  size_t off_dst = command->off_dst;
  size_t size = command->cb;
  CLEvent* event = command->event;

  MessageCodec mc(command);
  mc.SetTag(CLUSTER_TAG_MEM_RECV);
  mc.SetCID(NewCID());
  mc.SetInt(&dev_id);
  mc.SetInt(&dev_src->node_id);
  SetMemID(mem_dst, &mc);
  mc.SetULong(&off_dst);
  mc.SetULong(&size);
  mc.SetULong(&event->id);
  mc.SetBool(&event->disclosed);
  mc.Finish(command);
}

void ClusterCLDevice::PrepareReadImage(CLCommand* command) {
  SNUCL_CHECK();
  CLMem* mem_src = command->mem_src;
  size_t origin[3] = { command->src_origin[0], command->src_origin[1], command->src_origin[2] };
  size_t region[3] = { command->region[0], command->region[1], command->region[2] };
  size_t row_pitch = command->dst_row_pitch;
  size_t slice_pitch = command->dst_slice_pitch;
  size_t size = 0;
  size_t size_row_pitch = row_pitch == 0 ? region[0] * mem_src->image_elem_size : row_pitch;
  size_t size_slice_pitch = slice_pitch == 0 ? region[1] * size_row_pitch : slice_pitch;

  if (mem_src->image_desc.image_type == CL_MEM_OBJECT_IMAGE1D) {
    size = mem_src->image_desc.image_width * mem_src->image_elem_size;
  } else if (mem_src->image_desc.image_type == CL_MEM_OBJECT_IMAGE2D) {
    size = region[1] * size_row_pitch;
  } else if (mem_src->image_desc.image_type == CL_MEM_OBJECT_IMAGE3D) {
    size = region[2] * size_slice_pitch;
  }
  size_t offset = 0;
  command->cb = size;
  command->off_src = offset;

  CLEvent* event = command->event;
  int node_id_host = 0;

  MessageCodec mc(command);
  mc.SetTag(CLUSTER_TAG_IMAGE_SEND);
  mc.SetCID(NewCID());
  mc.SetInt(&dev_id);
  mc.SetInt(&node_id_host);
  SetMemID(mem_src, &mc);
  mc.SetULong(&origin[0]);
  mc.SetULong(&origin[1]);
  mc.SetULong(&origin[2]);
  mc.SetULong(&region[0]);
  mc.SetULong(&region[1]);
  mc.SetULong(&region[2]);
  mc.SetULong(&row_pitch);
  mc.SetULong(&slice_pitch);
  mc.SetULong(&size);
  mc.SetULong(&offset);
  mc.SetULong(&event->id);
  mc.SetBool(&event->disclosed);
  mc.Finish(command);
}

void ClusterCLDevice::PrepareWriteImage(CLCommand* command) {
  SNUCL_CHECK();
  CLMem* mem_dst = command->mem_dst;
  size_t origin[3] = { command->dst_origin[0], command->dst_origin[1], command->dst_origin[2] };
  size_t region[3] = { command->region[0], command->region[1], command->region[2] };
  size_t row_pitch = command->src_row_pitch;
  size_t slice_pitch = command->src_slice_pitch;
  size_t size = 0;
  size_t size_row_pitch = row_pitch == 0 ? region[0] * mem_dst->image_elem_size : row_pitch;
  size_t size_slice_pitch = slice_pitch == 0 ? region[1] * size_row_pitch : slice_pitch;

  if (mem_dst->image_desc.image_type == CL_MEM_OBJECT_IMAGE1D) {
    size = mem_dst->image_desc.image_width * mem_dst->image_elem_size;
  } else if (mem_dst->image_desc.image_type == CL_MEM_OBJECT_IMAGE2D) {
    size = region[1] * size_row_pitch;
  } else if (mem_dst->image_desc.image_type == CL_MEM_OBJECT_IMAGE3D) {
    size = region[2] * size_slice_pitch;
  }
  size_t offset = 0;
  command->cb = size;
  command->off_dst = offset;

  CLEvent* event = command->event;
  int node_id_host = 0;

  MessageCodec mc(command);
  mc.SetTag(CLUSTER_TAG_IMAGE_RECV);
  mc.SetCID(NewCID());
  mc.SetInt(&dev_id);
  mc.SetInt(&node_id_host);
  SetMemID(mem_dst, &mc);
  mc.SetULong(&origin[0]);
  mc.SetULong(&origin[1]);
  mc.SetULong(&origin[2]);
  mc.SetULong(&region[0]);
  mc.SetULong(&region[1]);
  mc.SetULong(&region[2]);
  mc.SetULong(&row_pitch);
  mc.SetULong(&slice_pitch);
  mc.SetULong(&size);
  mc.SetULong(&offset);
  mc.SetULong(&event->id);
  mc.SetBool(&event->disclosed);
  mc.Finish(command);
  SNUCL_CHECK();
}

void ClusterCLDevice::PrepareCopyImage(CLCommand* command) {
  //TODO
  //CLDevice* dev_src = command->dev_src;
  CLDevice* dev_src = command->device;
  CLMem* mem_src = command->mem_src;
  CLMem* mem_dst = command->mem_dst;
  size_t src_origin[3] = { command->src_origin[0], command->src_origin[1], command->src_origin[2] };
  size_t dst_origin[3] = { command->dst_origin[0], command->dst_origin[1], command->dst_origin[2] };
  size_t region[3] = { command->region[0], command->region[1], command->region[2] };

  CLEvent* event = command->event;

  MessageCodec mc(command);
  mc.SetTag(CLUSTER_TAG_IMAGE_COPY);
  mc.SetCID(NewCID());
  mc.SetInt(&dev_id);
  mc.SetInt(&dev_src->dev_id);
  SetMemID(mem_dst, &mc);
  SetMemID(mem_src, &mc);
  mc.SetULong(&src_origin[0]);
  mc.SetULong(&src_origin[1]);
  mc.SetULong(&src_origin[2]);
  mc.SetULong(&dst_origin[0]);
  mc.SetULong(&dst_origin[1]);
  mc.SetULong(&dst_origin[2]);
  mc.SetULong(&region[0]);
  mc.SetULong(&region[1]);
  mc.SetULong(&region[2]);
  mc.SetULong(&event->id);
  mc.SetBool(&event->disclosed);
  mc.Finish(command);
}

void ClusterCLDevice::PrepareCopyImageToBuffer(CLCommand* command) {
  SNUCL_CHECK();
  //CLDevice* dev_src = command->dev_src;
  CLDevice* dev_src = command->device;
  CLMem* mem_src = command->mem_src;
  CLMem* mem_dst = command->mem_dst;
  size_t src_origin[3] = { command->src_origin[0], command->src_origin[1], command->src_origin[2] };
  size_t region[3] = { command->region[0], command->region[1], command->region[2] };
  size_t off_dst = command->off_dst;

  CLEvent* event = command->event;

  MessageCodec mc(command);
  mc.SetTag(CLUSTER_TAG_IMAGE_BUFFER);
  mc.SetCID(NewCID());
  mc.SetInt(&dev_id);
  mc.SetInt(&dev_src->dev_id);
  SetMemID(mem_dst, &mc);
  SetMemID(mem_src, &mc);
  mc.SetULong(&src_origin[0]);
  mc.SetULong(&src_origin[1]);
  mc.SetULong(&src_origin[2]);
  mc.SetULong(&region[0]);
  mc.SetULong(&region[1]);
  mc.SetULong(&region[2]);
  mc.SetULong(&off_dst);
  mc.SetULong(&event->id);
  mc.SetBool(&event->disclosed);
  mc.Finish(command);
}

void ClusterCLDevice::PrepareCopyBufferToImage(CLCommand* command) {
  SNUCL_CHECK();
  //CLDevice* dev_src = command->dev_src;
  CLDevice* dev_src = command->device;
  CLMem* mem_src = command->mem_src;
  CLMem* mem_dst = command->mem_dst;
  size_t dst_origin[3] = { command->dst_origin[0], command->dst_origin[1], command->dst_origin[2] };
  size_t region[3] = { command->region[0], command->region[1], command->region[2] };
  size_t off_src = command->off_src;

  CLEvent* event = command->event;

  MessageCodec mc(command);
  mc.SetTag(CLUSTER_TAG_BUFFER_IMAGE);
  mc.SetCID(NewCID());
  mc.SetInt(&dev_id);
  mc.SetInt(&dev_src->dev_id);
  SetMemID(mem_dst, &mc);
  SetMemID(mem_src, &mc);
  mc.SetULong(&dst_origin[0]);
  mc.SetULong(&dst_origin[1]);
  mc.SetULong(&dst_origin[2]);
  mc.SetULong(&region[0]);
  mc.SetULong(&region[1]);
  mc.SetULong(&region[2]);
  mc.SetULong(&off_src);
  mc.SetULong(&event->id);
  mc.SetBool(&event->disclosed);
  mc.Finish(command);
}

void ClusterCLDevice::ReadImage(CLCommand* command) {
  CLMem* mem_src = command->mem_src;
  CLEvent* event = command->event;

  SendCommand(command);

  ClusterSync *cs = MakeCS(event);
  SNUCL_INFO("READ IMAGE PTR[%p], SIZE[%lu]", command->ptr, command->cb); 
  mpi_ret = MPI_Irecv(command->ptr, (int) command->cb, MPI_CHAR, node_id, CLUSTER_TAG_MEM_SEND_BODY(event->id), MPI_COMM_WORLD, &cs->mpi_request_wait);

  issuer->EnqueueRunningCommand(command);
}

void ClusterCLDevice::WriteImage(CLCommand* command) {
  SNUCL_CHECK();
  CLEvent* event = command->event;
  ClusterSync *cs = MakeCS(event);
  SendCommand(command);
  SNUCL_INFO("WRITE IMAGE PTR[%p], SIZE[%lu]", command->ptr, command->cb); 
  mpi_ret = MPI_Isend(command->ptr, (int) command->cb, MPI_CHAR, node_id, CLUSTER_TAG_MEM_RECV_BODY(event->id), MPI_COMM_WORLD, &cs->mpi_request);
  mpi_ret = MPI_Irecv(&cs->event_id_ret, sizeof(cs->event_id_ret), MPI_CHAR, node_id, CLUSTER_TAG_EVENT_WAIT(event->id), MPI_COMM_WORLD, &cs->mpi_request_wait);

  issuer->EnqueueRunningCommand(command);
  SNUCL_CHECK();
}

void ClusterCLDevice::CopyImage(CLCommand* command) {
  SendWaitCommand(command);
}

void ClusterCLDevice::CopyImageToBuffer(CLCommand* command) {
  SNUCL_CHECK();
  SendWaitCommand(command);
}

void ClusterCLDevice::CopyBufferToImage(CLCommand* command) {
  SNUCL_CHECK();
  SendWaitCommand(command);
}

void ClusterCLDevice::PrepareMarker(CLCommand* command) {
  CLEvent* event = command->event;

  MessageCodec mc(command);
  mc.SetTag(CLUSTER_TAG_MARKER);
  mc.SetCID(NewCID());
  mc.SetInt(&dev_id);
  mc.SetULong(&event->id);
  mc.SetBool(&event->disclosed);
  mc.Finish(command);
}

void ClusterCLDevice::MigrateMemObjects(CLCommand * command) {
  cl_uint num_mem_objects = command->num_mem_objects;
  cl_mem* mem_objects = command->mem_objects;

  for(int i=0; i < num_mem_objects; ++i) {
    CheckBuffer(mem_objects[i]->c_obj);
  }

  free(mem_objects);

  command->event->Complete();
}

bool ClusterCLDevice::IsComplete(CLEvent* event) {
  ClusterSync *cs = (ClusterSync*) event->dev_specific[this];
  if (cs == NULL) SNUCL_ERROR("%s", "CS IS NULL");
  int flag = 0;
  mpi_ret = MPI_Test(&cs->mpi_request_wait, &flag, &cs->mpi_status);
  if (flag == 0) return false;
  delete cs;
  return true;
}

void ClusterCLDevice::PrintStatistics() {
  CLDevice::PrintStatistics();
  MessageCodec mc;
  mc.SetTag(CLUSTER_TAG_DEV_STATS);
  mc.SetCID(NewCID());
  mc.SetInt(&dev_id);

  SendMsg(mc.msg);
}

void ClusterCLDevice::SendMsg(char* msg) {
#if 0
  mpi_ret = MPI_Send(msg, MESSAGE_COMMAND_SIZE, MPI_CHAR, node_id, CLUSTER_TAG_COMMAND, MPI_COMM_WORLD);
#else
  MPI_Request mpi_request;
  mpi_ret = MPI_Isend(msg, MESSAGE_COMMAND_SIZE, MPI_CHAR, node_id, CLUSTER_TAG_COMMAND, MPI_COMM_WORLD, &mpi_request);
#endif
}

void ClusterCLDevice::SendCommand(CLCommand* command) {
#if 1
  mpi_ret = MPI_Send(command->msg, MESSAGE_COMMAND_SIZE, MPI_CHAR, node_id, CLUSTER_TAG_COMMAND, MPI_COMM_WORLD);
#else
  MPI_Request mpi_request;
  mpi_ret = MPI_Isend(command->msg, MESSAGE_COMMAND_SIZE, MPI_CHAR, node_id, CLUSTER_TAG_COMMAND, MPI_COMM_WORLD, &mpi_request);
#endif
}

void ClusterCLDevice::WaitCommand(CLCommand* command) {
  //if (command->event->disclosed) {
  if (1) {
    CLEvent* event = command->event;
    ClusterSync* cs = MakeCS(event);
    mpi_ret = MPI_Irecv(&cs->event_id_ret, sizeof(cs->event_id_ret), MPI_CHAR, node_id, CLUSTER_TAG_EVENT_WAIT(event->id), MPI_COMM_WORLD, &cs->mpi_request_wait);
  }
  issuer->EnqueueRunningCommand(command);
}

void ClusterCLDevice::SendWaitCommand(CLCommand* command) {
  SendCommand(command);
  WaitCommand(command);
}

cl_int ClusterCLDevice::GetSupportedImageFormats(cl_mem_flags flags, cl_mem_object_type image_type, cl_uint num_entries, cl_image_format *_image_formats, cl_uint* _num_image_formats) {
  MessageCodec mc;
  mc.SetTag(CLUSTER_TAG_IMAGE_INFO);
  mc.SetCID(NewCID());
  mc.SetInt(&dev_id);
  mc.SetULong(&flags);
  mc.SetUInt(&image_type);

  MPI_Status mpi_status;
  char image_info[8192];

  mpi_ret = MPI_Sendrecv(mc.msg, MESSAGE_COMMAND_SIZE, MPI_CHAR, node_id, CLUSTER_TAG_COMMAND, image_info, sizeof(image_info), MPI_CHAR, node_id, CLUSTER_TAG_IMAGE_INFO, MPI_COMM_WORLD, &mpi_status);

  MessageCodec m(image_info);

  unsigned int num_image_formats = m.GetUInt();

  if (_image_formats == NULL) {
    *_num_image_formats = num_image_formats;
    return CL_SUCCESS;
  }

  for (unsigned int i = 0; i < num_image_formats && i < num_entries; i++) {
    _image_formats[i].image_channel_order = m.GetUInt();  
    _image_formats[i].image_channel_data_type = m.GetUInt();  
    *_num_image_formats = i + 1;
  }

  return CL_SUCCESS;
}

ClusterSync* ClusterCLDevice::MakeCS(CLEvent* event) {
  ClusterSync* cs = new ClusterSync();
  cs->event_id = event->id;

  pthread_mutex_lock(&event->mutex_dev_specific);
  event->dev_specific[this] = (void*) cs;
  pthread_mutex_unlock(&event->mutex_dev_specific);

  return cs;
}

void ClusterCLDevice::SetMemID(CLMem* mem, MessageCodec* mc) {
  if (IsAllocBuffer(mem)) {
    mc->SetULong(&mem->id);
  } else {
    pthread_mutex_lock(&mem->mutex_dev_specific);
    mem->dev_specific[this] = (void*) this;
    pthread_mutex_unlock(&mem->mutex_dev_specific);

    unsigned long ZERO = 0UL;
    mc->SetULong(&ZERO);
    mc->SetULong(&mem->id);
    mc->SetULong(&mem->flags);
    mc->SetULong(&mem->size);
    mc->SetBool(&mem->is_image);
    mc->SetBool(&mem->is_sub);

    if (mem->is_image) {
      mc->SetUInt(&mem->image_format.image_channel_order);
      mc->SetUInt(&mem->image_format.image_channel_data_type);
      mc->SetUInt(&mem->image_desc.image_type);
      mc->SetULong(&mem->image_desc.image_width);
      mc->SetULong(&mem->image_desc.image_height);
      mc->SetULong(&mem->image_desc.image_depth);
      mc->SetULong(&mem->image_desc.image_array_size);
      mc->SetULong(&mem->image_desc.image_row_pitch);
      mc->SetULong(&mem->image_desc.image_slice_pitch);
      mc->SetUInt(&mem->image_desc.num_mip_levels);
      mc->SetUInt(&mem->image_desc.num_samples);
    } else if (mem->is_sub) {
      SNUCL_ERROR("SUB B[%lu]", mem->id);
      mc->SetULong(&mem->parent->id);
      mc->SetUInt(&mem->create_type);
      mc->SetULong(&mem->buf_create_info.origin);
      mc->SetULong(&mem->buf_create_info.size);
    }
  }
  SNUCL_INFO("MC OFFSET [%lu]", mc->Offset());
}

void ClusterCLDevice::SetSamplerID(CLSampler* sampler, MessageCodec* mc) {
  pthread_mutex_lock(&sampler->mutex_dev_specific);
  int allocated = sampler->dev_specific.count(this);
  pthread_mutex_unlock(&sampler->mutex_dev_specific);
  if (allocated > 0 ) {
    mc->SetULong(&sampler->id);
  } else {
    pthread_mutex_lock(&sampler->mutex_dev_specific);
    sampler->dev_specific[this] = (void*) this;
    pthread_mutex_unlock(&sampler->mutex_dev_specific);

    unsigned long ZERO = 0UL;
    mc->SetULong(&ZERO);
    mc->SetULong(&sampler->id);
    mc->SetUInt(&sampler->normalized_coords);
    mc->SetUInt(&sampler->addressing_mode);
    mc->SetUInt(&sampler->filter_mode);
  }
}

unsigned long ClusterCLDevice::NewCID() {
  while (!__sync_bool_compare_and_swap(&CID, CID, CID + 1)) {}
  return CID;
}

