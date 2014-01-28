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

#include <CLObject.h>
#include <Utils.h>
#include <arch/x86/x86Device.h>
#include <arch/legacy/LegacyDevice.h>
#include <malloc.h>
#include <sys/time.h>

SNUCL_DEBUG_HEADER("CLObject");

CLPlatform g_platform;
CommandFactory* CommandFactory::s_instance = new CommandFactory();

static unsigned long UID;

static unsigned long cmd_c;
static unsigned long cmd_d;

static unsigned long NewID() {
  while (!__sync_bool_compare_and_swap(&UID, UID, UID + 1)) {}
  return UID;
}

static unsigned long CMD_C() {
  while (!__sync_bool_compare_and_swap(&cmd_c, cmd_c, cmd_c + 1)) {}
  return cmd_c;
}

static unsigned long CMD_D() {
  while (!__sync_bool_compare_and_swap(&cmd_d, cmd_d, cmd_d + 1)) {}
  return cmd_d;
}

CLObject::CLObject() {
  id = NewID();
  ref_cnt = 0;
}

int CLObject::Retain() {
  return ++ref_cnt;
}

int CLObject::Release() {
  if (ref_cnt > 0) --ref_cnt;
  return ref_cnt;
}

CLPlatform::CLPlatform() {
  st_obj.c_obj = this;

  profile = "FULL_PROFILE";
  version = "OpenCL 1.2 rev00";
  name = "SnuCL";
  vendor = "Seoul National University";
  extensions = "";

  cluster = false;
  host = false;

  num_schedulers = 1;
  schedulers = NULL;
  issuer = NULL;
  profiler = new CLProfiler(true);

  sem_init(&sem_dev, 0, 0);

  InitDevices();

  start_timestamp = SNUCL_Timestamp();
}

CLPlatform::~CLPlatform() {
  sem_destroy(&sem_dev);
  if (schedulers) {
    for (int i = 0; i < num_schedulers; i++) delete schedulers[i];
    delete[] schedulers;
  }
  delete profiler;

  SNUCL_INFO("CMD C[%lu] D[%lu]", cmd_c, cmd_d);
}

void CLPlatform::InitDevices() {
  int total_dev = 0;
  int total_gpu = 0;
  int total_cpu = 0;
  int total_acc = 0;
  int total_cus = 0;
#ifdef USE_X86_DEVICE
  total_dev += x86CLDevice::CreateDevices(&devices);
#endif
#ifdef USE_LEGACY_DEVICE
  total_dev += LegacyCLDevice::CreateDevices(&devices, CL_DEVICE_TYPE_GPU, "NVIDIA Corporation");
#endif

  for (int i = 0; i < total_dev; i++) {
    sem_wait(&sem_dev);
  }

  for (vector<CLDevice*>::iterator it = devices.begin(); it != devices.end(); ++it) {
    CLDevice* dev = *it;
    if (dev->type == CL_DEVICE_TYPE_CPU) total_cpu++;
    else if (dev->type == CL_DEVICE_TYPE_GPU) total_gpu++;
    else if (dev->type == CL_DEVICE_TYPE_ACCELERATOR) total_acc++;
    else if (dev->type == CL_DEVICE_TYPE_CUSTOM) total_cus++;
    else SNUCL_ERROR("UNSUPPORT DEVICE TYPE [%lx]", dev->type);
  }

  SNUCL_INFO("SNUCL platform has been initialized. Total %d devices (%d CPUs, %d GPUs) are in the platform.", devices.size(), total_cpu, total_gpu);
}

void CLPlatform::InitSchedulers(bool running) {
  if (schedulers) return;

  schedulers = new CLScheduler*[num_schedulers];
  for (int i = 0; i < num_schedulers; i++) {
    schedulers[i] = new CLScheduler(this);
  }
  if (cluster) issuer = new CLIssuer(this);

  for (int i = 0; i < devices.size(); i++) {
    int idx_scheduler = i % num_schedulers;
    CLScheduler* scheduler = schedulers[idx_scheduler];
    CLDevice* dev = devices[i];
    scheduler->AddDevice(dev);
  }

  if (running) {
    for (int i = 0; i < num_schedulers; i++) schedulers[i]->Start();
    if (issuer) issuer->Start();
  }
}

CLDevice::CLDevice(cl_device_type type) {
  st_obj.c_obj = this;

  this->type = type;

	platform = &g_platform;

  command_queues = new CLCommandQueue*[MAX_NUM_CQ_PER_DEVICE];
  command_queue_retained = (bool*)malloc(MAX_NUM_CQ_PER_DEVICE * sizeof(bool));
	for (int i=0; i<MAX_NUM_CQ_PER_DEVICE; i++)
		command_queue_retained[i] = false;
  ready_queue = new LockFreeQueueMS(MAX_NUM_COMMANDS_IN_READY_QUEUE);
  num_command_queues = 0;
  pthread_mutex_init(&mutex_q, NULL);

  node_id = 0;
  dev_id = id;
  gethostname(node_name, sizeof(node_name));

  pthread_mutex_init(&mutex_p, NULL);

  sem_init(&sem_rq, 0, 0);

	parent_device = NULL; 
  available = CL_TRUE;
  compiler_available = CL_TRUE;
	partition_type_len = 0;
	partition_type = NULL;
}

CLDevice::~CLDevice() {
	scheduler->devices[devIdxInScheduler] = NULL; //delete from scheduler
  for (int i = 0; i < num_command_queues; i++) delete command_queues[i];
  delete[] command_queues;
  free(command_queue_retained);
  delete ready_queue;
  pthread_mutex_destroy(&mutex_q);
	if(partition_type)
		free(partition_type);
  sem_destroy(&sem_rq);
  pthread_mutex_destroy(&mutex_p);
}

void CLDevice::Prepare(CLCommand* command) {
  switch (command->type) {
    case CL_COMMAND_NDRANGE_KERNEL:
    case CL_COMMAND_TASK:                   PrepareLaunchKernel(command);      break;
    case CL_COMMAND_NATIVE_KERNEL:          PrepareNativeKernel(command);      break;
    case CL_COMMAND_READ_BUFFER:            PrepareReadBuffer(command);        break;
    case CL_COMMAND_WRITE_BUFFER:           PrepareWriteBuffer(command);       break;
    case CL_COMMAND_COPY_BUFFER:            PrepareCopyBuffer(command);        break;
    case CL_COMMAND_READ_IMAGE:             PrepareReadImage(command);         break;
    case CL_COMMAND_WRITE_IMAGE:            PrepareWriteImage(command);        break;
    case CL_COMMAND_COPY_IMAGE:             PrepareCopyImage(command);         break;
    case CL_COMMAND_COPY_IMAGE_TO_BUFFER:   PrepareCopyImageToBuffer(command); break;
    case CL_COMMAND_COPY_BUFFER_TO_IMAGE:   PrepareCopyBufferToImage(command); break;
    case CL_COMMAND_MAP_BUFFER:                                                break;
    case CL_COMMAND_MAP_IMAGE:                                                 break;
    case CL_COMMAND_UNMAP_MEM_OBJECT:                                          break;
    case CL_COMMAND_WAIT_FOR_EVENTS:                                           break;
    case CL_COMMAND_BARRIER:                                                   break;
    case CL_COMMAND_MARKER:                 PrepareMarker(command);            break;
    case CL_COMMAND_ACQUIRE_GL_OBJECTS:                                        break;
    case CL_COMMAND_RELEASE_GL_OBJECTS:                                        break;
    case CL_COMMAND_READ_BUFFER_RECT:       PrepareReadBufferRect(command);    break;
    case CL_COMMAND_WRITE_BUFFER_RECT:      PrepareWriteBufferRect(command);   break;
    case CL_COMMAND_COPY_BUFFER_RECT:       PrepareCopyBufferRect(command);    break;
    case CL_COMMAND_USER:                                                      break;
    case CL_COMMAND_COMPILE_PROGRAM:                                           break;
    case CL_COMMAND_LINK_PROGRAM:                                              break;
    case CL_COMMAND_BUILD_PROGRAM:          PrepareBuildProgram(command);      break;
    case CL_COMMAND_SEND_BUFFER:            PrepareSendBuffer(command);        break;
    case CL_COMMAND_RECV_BUFFER:            PrepareRecvBuffer(command);        break;
    case CL_COMMAND_FREE_BUFFER:            PrepareFreeBuffer(command);        break;
    case CL_COMMAND_NOP:                                                       break;
    case CL_COMMAND_BROADCAST_BUFFER:                                          break;
    case CL_COMMAND_ALLTOALL_BUFFER:                                           break;
    default:
      SNUCL_ERROR("DEVICE[%lu] UNKNOWN COMMAND[%lu] TYPE[%x]", id, command->id, command->type);
      break;
  }
}

void CLDevice::Execute(CLCommand* command) {
	command->event->SetStatus(CL_RUNNING);
  if (command->command_queue && command->command_queue->IsProfiled())
    command->event->SetTimestamp(CL_PROFILING_COMMAND_START);
  switch (command->type) {
    case CL_COMMAND_NDRANGE_KERNEL:
    case CL_COMMAND_TASK:                   LaunchKernel(command);        break;
    case CL_COMMAND_NATIVE_KERNEL:          NativeKernel(command);        break;
    case CL_COMMAND_READ_BUFFER:            ReadBuffer(command);          break;
    case CL_COMMAND_WRITE_BUFFER:           WriteBuffer(command);         break;
    case CL_COMMAND_COPY_BUFFER:            CopyBuffer(command);          break;
    case CL_COMMAND_READ_IMAGE:             ReadImage(command);           break;
    case CL_COMMAND_WRITE_IMAGE:            WriteImage(command);          break;
    case CL_COMMAND_COPY_IMAGE:             CopyImage(command);           break;
    case CL_COMMAND_COPY_IMAGE_TO_BUFFER:   CopyImageToBuffer(command);   break;
    case CL_COMMAND_COPY_BUFFER_TO_IMAGE:   CopyBufferToImage(command);   break;
    case CL_COMMAND_MAP_BUFFER:             MapBuffer(command);           break;
    case CL_COMMAND_MAP_IMAGE:              MapImage(command);            break;
    case CL_COMMAND_UNMAP_MEM_OBJECT:       UnmapMemObject(command);      break;
    case CL_COMMAND_WAIT_FOR_EVENTS:
    case CL_COMMAND_BARRIER:                Barrier(command);             break;
    case CL_COMMAND_MARKER:                 Marker(command);              break;
    case CL_COMMAND_ACQUIRE_GL_OBJECTS:
    case CL_COMMAND_RELEASE_GL_OBJECTS:
    case CL_COMMAND_READ_BUFFER_RECT:       ReadBufferRect(command);      break;
    case CL_COMMAND_WRITE_BUFFER_RECT:      WriteBufferRect(command);     break;
    case CL_COMMAND_COPY_BUFFER_RECT:       CopyBufferRect(command);      break;
    case CL_COMMAND_USER:                                                 break;
    case CL_COMMAND_COMPILE_PROGRAM:        CompileProgram(command);      break;
    case CL_COMMAND_LINK_PROGRAM:           LinkProgram(command);         break;
    case CL_COMMAND_BUILD_PROGRAM:          BuildProgram(command);        break;
    case CL_COMMAND_SEND_BUFFER:            SendBuffer(command);          break;
    case CL_COMMAND_RECV_BUFFER:            RecvBuffer(command);          break;
    case CL_COMMAND_FREE_BUFFER:            FreeBuffer(command);          break;
    case CL_COMMAND_NOP:                    command->event->Complete();   break;
    case CL_COMMAND_BROADCAST_BUFFER:       BcastBuffer(command);         break;
    case CL_COMMAND_ALLTOALL_BUFFER:        AlltoallBuffer(command);      break;
    case CL_COMMAND_FILL_BUFFER:            FillBuffer(command);   break;
    case CL_COMMAND_FILL_IMAGE:             FillImage(command);    break;
    case CL_COMMAND_MIGRATE_MEM_OBJECTS:    MigrateMemObjects(command); break;
    default:
      SNUCL_ERROR("DEVICE[%lu] UNKNOWN COMMAND[%lu] TYPE[%x]", id, command->id, command->type);
      break;
  }
}

void CLDevice::Marker(CLCommand* command) {
  command->event->Complete();
}

void CLDevice::Barrier(CLCommand* command) {
  command->event->Complete();
}

bool CLDevice::IsAllocBuffer(CLMem* mem) {
  pthread_mutex_lock(&mem->mutex_dev_specific);
  int allocated = mem->dev_specific.count(this);
  pthread_mutex_unlock(&mem->mutex_dev_specific);
  return allocated > 0;
}

void CLDevice::CheckBuffer(CLMem* mem) {
  if (!IsAllocBuffer(mem)) AllocBuffer(mem);
}

void CLDevice::EnqueueBuildProgram(CLProgram* program, const char* options) {
  CLCommand* command = CommandFactory::instance()->NewCommand(CL_COMMAND_BUILD_PROGRAM);
  command->program = program;

  if (options) {
		if(program->options){
			free(program->options);
			program->options = NULL;
		}
		program->options = (char*)malloc(sizeof(char)*strlen(options)+1);
		memcpy(program->options, options, strlen(options));
		program->options[strlen(options)] = '\0';
	} else {
		if(program->options){
			free(program->options);
			program->options = NULL;
		}
  }
  scheduler->EnqueueReadyQueue(command, this);

	if(program->buildCallbackStack.size() == 0) {
    command->event->Wait();
  }
  AddProgram(program);
}

void CLDevice::EnqueueCompileProgram(CLProgram* program, const char* options) {
  CLCommand* command = new CLCommand(CL_COMMAND_COMPILE_PROGRAM);
  command->program = program;
  if (options) {
		if(program->options){
			free(program->options);
			program->options = NULL;
		}
		program->options = (char*)malloc(sizeof(char)*strlen(options)+1);
		memcpy(program->options, options, strlen(options));
		program->options[strlen(options)] = '\0';
	}
  scheduler->EnqueueReadyQueue(command, this);

	if(program->compileCallbackStack.size() == 0)
		command->event->Wait();
}
void CLDevice::EnqueueLinkProgram(CLProgram* program, const char* options) {
  CLCommand* command = new CLCommand(CL_COMMAND_LINK_PROGRAM);
  command->program = program;
  if (options) {
		if(program->options){
			free(program->options);
			program->options = NULL;
		}
		program->options = (char*)malloc(sizeof(char)*strlen(options)+1);
		memcpy(program->options, options, strlen(options));
		program->options[strlen(options)] = '\0';
	}
  scheduler->EnqueueReadyQueue(command, this);

	if(program->linkCallbackStack.size() == 0)
		command->event->Wait();

  AddProgram(program);
}

void CLDevice::AddCommandQueue(CLCommandQueue* command_queue) {
  pthread_mutex_lock(&mutex_q);
  command_queues[num_command_queues] = command_queue;
  command_queue_retained[num_command_queues] = true;
	command_queue->queueIdx = num_command_queues;
  num_command_queues++;
  if (num_command_queues >= MAX_NUM_CQ_PER_DEVICE) SNUCL_ERROR("NUM CQ IN THIS DEVICE[%lu] IS [%d]", id, num_command_queues);
  pthread_mutex_unlock(&mutex_q);
}

bool CLDevice::EnqueueReadyQueue(CLCommand* command) {
  while (!ready_queue->Enqueue(command)) {}
  if (!g_platform.cluster) sem_post(&sem_rq);
  return true;
}

void CLDevice::AddSubDevicesToScheduler() {
	if (platform->schedulers) {
		for (uint i=0; i<numSubDevices; i++) {
			int idx_scheduler = i % platform->num_schedulers;
			platform->schedulers[idx_scheduler]->AddDevice(subDevices[i]);
		}
	}
	//wait until each worker has created
	for (uint i=0; i<numSubDevices; i++) {
		sem_wait(&platform->sem_dev);
	}

}

void CLDevice::CopyProgram(CLProgram* program, CLDevice* dev) {
  pthread_mutex_lock(&program->mutex_dev_specific);
  program->dev_specific[this] = program->dev_specific[dev];
  pthread_mutex_unlock(&program->mutex_dev_specific);

	AddProgram(program);
}
void CLDevice::AddProgram(CLProgram* program) {
  for (vector<CLProgram*>::iterator it = programs.begin(); it != programs.end(); ++it) {
    if (*it == program) return;
  }
  programs.push_back(program);
}

bool CLDevice::ContainsProgram(CLProgram* program) {
	CLDevice* d;
  for (vector<CLProgram*>::iterator it = programs.begin(); it != programs.end(); ++it) {
    if (*it == program) return true;
  }

	if (parent_device) { //this is a sub device
		d = parent_device->c_obj;
		for (vector<CLProgram*>::iterator it = d->programs.begin(); it != d->programs.end(); ++it) {
			if (*it == program) {
				CopyProgram(program, d);
				return true;
			}
		}
		for (vector<CLDevice*>::iterator it = parent_device->c_obj->subDevices.begin(); it != parent_device->c_obj->subDevices.end(); ++it) {
			d = (*it);
			for (vector<CLProgram*>::iterator it = d->programs.begin(); it != d->programs.end(); ++it) {
				if (*it == program) {
					CopyProgram(program, d);
					return true;
				}
			}
		}
	}
	else if (subDevices.size()>0) {//parent device
		for (vector<CLDevice*>::iterator it = subDevices.begin(); it != subDevices.end(); ++it) {
			d = (*it);
			for (vector<CLProgram*>::iterator it = d->programs.begin(); it != d->programs.end(); ++it) {
				if (*it == program) {
					CopyProgram(program, d);
					return true;
				}
			}
		}
	}

  return false;
}

void CLDevice::PrintStatistics() {
  SNUCL_INFO("DEV[%3lu] [%7.3lf, %7.3lf, %7.3lf, %7.3lf] K[%3d, %7.3lf] M[%3d, %7.3lf] S[%3d, %7.3lf] Q[%7.3lf, %7.3lf] S[%7.3lf, %7.3lf] S[%7.3lf, %7.3lf] E[%7.3lf, %7.3lf] T[%7.3lf]",
      id,
      (double) time_cmd_submit / 1000000.0, (double) time_cmd_start / 1000000.0, (double) time_cmd_end / 1000000.0,
      (double) time_cmd_issue /  1000000.0,
      cnt_cmd_k, (double) time_cmd_k / 1000000.0,
      cnt_cmd_m, (double) time_cmd_m / 1000000.0,
      cnt_cmd_s, (double) time_cmd_s / 1000000.0,
      (double) (time_cmd_queued_first - g_platform.start_timestamp) / 1000000.0,
      (double) (time_cmd_queued_last - g_platform.start_timestamp) / 1000000.0,
      (double) (time_cmd_submit_first - g_platform.start_timestamp) / 1000000.0,
      (double) (time_cmd_submit_last - g_platform.start_timestamp) / 1000000.0,
      (double) (time_cmd_start_first - g_platform.start_timestamp) / 1000000.0,
      (double) (time_cmd_start_last - g_platform.start_timestamp) / 1000000.0,
      (double) (time_cmd_end_first - g_platform.start_timestamp) / 1000000.0,
      (double) (time_cmd_end_last - g_platform.start_timestamp) / 1000000.0,
      (double) (time_cmd_end_last - time_cmd_submit_first) / 1000000.0);
  time_cmd_k = time_cmd_m = time_cmd_s = 0;
  time_cmd_submit = time_cmd_start = time_cmd_end = 0;

  time_cmd_issue = 0;

  time_cmd_queued_first = time_cmd_queued_last = 0;
  time_cmd_submit_first = time_cmd_submit_last = 0;
  time_cmd_start_first = time_cmd_start_last = 0;
  time_cmd_end_first = time_cmd_end_last = 0;
  cnt_cmd_k = cnt_cmd_m = cnt_cmd_s = 0;
}

CLContext::CLContext(cl_uint num_devices, const cl_device_id* devices, bool running_scheduler) {
  st_obj.c_obj = this;

  total_mem_size = 0;

  for (uint i = 0; i < num_devices; ++i) {
    this->devices.push_back(devices[i]->c_obj);
  }

  pthread_mutex_init(&mutex_m, NULL);
  pthread_mutex_init(&mutex_s, NULL);

  g_platform.InitSchedulers(running_scheduler);
}

CLContext::~CLContext() {
  pthread_mutex_destroy(&mutex_m);
  pthread_mutex_destroy(&mutex_s);
}

cl_int CLContext::SetProperties(const cl_context_properties* properties) {
	int index=0;
	bool platform = false;
	bool sync = false;
	if (properties == NULL) return CL_SUCCESS;

	while (properties[index] != 0) {
		switch (properties[index]) {
			case CL_CONTEXT_PLATFORM: {
				if (properties[++index] != (cl_context_properties)&g_platform.st_obj) 
					return CL_INVALID_PLATFORM;
				if (platform) return CL_INVALID_PROPERTY;

				platform = true;
				++index;
				break;
			}
			case CL_CONTEXT_INTEROP_USER_SYNC: {
				if (sync) return CL_INVALID_PROPERTY;
				sync = true;
				index += 2;
			}
			default: return CL_INVALID_PROPERTY;
		}
	}

	this->properties = (cl_context_properties*)malloc((index+1)*sizeof(cl_context_properties));
	memcpy(this->properties, properties, (index+1)*sizeof(cl_context_properties));
	num_properties = index+1;

  return CL_SUCCESS;
}

void CLContext::AddMemory(CLMem* mem) {
  pthread_mutex_lock(&mutex_m);
  mems.push_back(mem);
  if(!mem->is_sub) {
    total_mem_size += mem->size;
  }
  pthread_mutex_unlock(&mutex_m);
}

void CLContext::AddSampler(CLSampler* sampler) {
  pthread_mutex_lock(&mutex_s);
  samplers.push_back(sampler);
  pthread_mutex_unlock(&mutex_s);
}

cl_int CLContext::SetDevices(vector<CLDevice*> devices, cl_device_type type) {
	bool exist = false;

	if (type != CL_DEVICE_TYPE_ALL && (!(type & (CL_DEVICE_TYPE_DEFAULT | CL_DEVICE_TYPE_CPU | CL_DEVICE_TYPE_GPU | CL_DEVICE_TYPE_ACCELERATOR | CL_DEVICE_TYPE_CUSTOM)))) return CL_INVALID_DEVICE_TYPE;

	for(vector<CLDevice*>::iterator it = devices.begin(); it != devices.end(); ++it) {
		if ((*it)->type == type) {
			exist = true;
			if ((*it)->available)
				this->devices.push_back((*it));
		}
	}

	if (this->devices.size() == 0) {
		if (exist) return CL_DEVICE_NOT_AVAILABLE;
		else return CL_DEVICE_NOT_FOUND;
	}

	this->num_devices = this->devices.size();

	return CL_SUCCESS;
}

CLCommandQueue::CLCommandQueue(CLContext *context, CLDevice* device, cl_command_queue_properties properties) {
  st_obj.c_obj = this;

  this->context = context;
  this->device = device;
  this->properties = properties;

  if (IsInOrder()) commands_io = new LockFreeQueueMS(MAX_NUM_COMMANDS_IN_READY_QUEUE);
  last_event = NULL;

  pthread_mutex_init(&mutex_q, NULL);

	queueIdx = -1;
  device->AddCommandQueue(this);

	Retain();
}

CLCommandQueue::~CLCommandQueue() {
  pthread_mutex_destroy(&mutex_q);
}

bool CLCommandQueue::IsInOrder() {
  return !(properties & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE);
}
bool CLCommandQueue::IsProfiled() {
  return (properties & CL_QUEUE_PROFILING_ENABLE);
}

void CLCommandQueue::Enqueue(CLCommand* command) {
  if (IsInOrder()) {
#ifdef ALWAYS_NOTIFY_COMPLETE
    if (last_event) command->AddWaitEvent(last_event);
#else
    if (last_event) {
      if (last_event->type != CL_COMMAND_NDRANGE_KERNEL && last_event->type != CL_COMMAND_NATIVE_KERNEL) {
        command->AddWaitEvent(last_event);
      }
    }
#endif
    last_event = command->event;
    commands_io->Enqueue(command);
  } else {
    pthread_mutex_lock(&mutex_q);
    commands.push_back(command);
    pthread_mutex_unlock(&mutex_q);
  }

  if(IsProfiled())
    command->event->SetTimestamp(CL_PROFILING_COMMAND_QUEUED);

  g_platform.profiler->EnqueueCommand(command);
}

void CLCommandQueue::Dequeue(CLCommand* command) {
  if (IsInOrder()) {
    CLCommand* cmd = NULL;
    commands_io->Dequeue(&cmd);
    if (cmd != command) SNUCL_ERROR("CMD[%lu] CMD[%lu]", cmd->id, command->id);
  } else {
    if (commands.front() == command) commands.pop_front();
    else commands.remove(command);
  }
  commands_dequeued.push_back(command);
}

void CLCommandQueue::Remove(CLCommand* command) {
  if (commands_dequeued.front() == command) commands_dequeued.pop_front();
  else commands_dequeued.remove(command);
}

cl_event* CLCommandQueue::GetAllEvents(cl_uint *num_events) {
  cl_uint size = 0;
  cl_uint index = 0;
  size = commands.size();
  size += commands_dequeued.size();
  cl_event *events = (cl_event*)malloc(size*sizeof(cl_event));

  std::list<CLCommand*>::iterator i;
  for(i=commands.begin(); i!=commands.end(); ++i) {
    events[index] = &(*i)->event->st_obj;
    index++;
  }
  for(i=commands_dequeued.begin(); i!=commands_dequeued.end(); ++i) {
    events[index] = &(*i)->event->st_obj;
    index++;
  }

  *num_events = size;
  return events;
}
CLCommand::CLCommand(cl_command_type type) {
	this->command_queue = NULL;
  this->type = type;
  event = new CLEvent(NULL, this->type, NULL, this);
  misc[0] = misc[1] = 0;
  sub_command = NULL;
  p_command = NULL;

  if (type == CL_COMMAND_NDRANGE_KERNEL || type == CL_COMMAND_TASK || type == CL_COMMAND_NATIVE_KERNEL) {
    kernel_args = new map<cl_uint, CLKernelArg*>();
  }

  free = true;

  CMD_C();
}

CLCommand::CLCommand(CLCommandQueue* command_queue, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_command_type type) {
  this->command_queue = command_queue;
  this->type = type;
  this->device = command_queue->device;

  for (cl_uint i = 0; i < num_events_in_wait_list; ++i) {
    AddWaitEvent(event_wait_list[i]->c_obj);
  }

  event = new CLEvent(&command_queue->context->st_obj, type, &command_queue->st_obj, this);

  misc[0] = misc[1] = 0;

  p_command = NULL;

	mem_src = NULL;
	mem_dst = NULL;
	kernel = NULL;

  if (type == CL_COMMAND_NDRANGE_KERNEL || type == CL_COMMAND_TASK || type == CL_COMMAND_NATIVE_KERNEL) {
    kernel_args = new map<cl_uint, CLKernelArg*>();
  }

  free = true;

  CMD_C();
}

CLCommand::~CLCommand() {
  /*
  if (type == CL_COMMAND_NDRANGE_KERNEL || type == CL_COMMAND_NATIVE_KERNEL) {
    for (map<cl_uint, CLKernelArg*>::iterator it = kernel_args->begin(); it != kernel_args->end(); ++it) {
      CLKernelArg* arg = it->second;
      delete arg;
    }
  }
  if (type == CL_COMMAND_NATIVE_KERNEL) delete kernel_args;
  */
}

void CLCommand::SetType(cl_command_type type) {
  this->type = type;
  event->type = type;
}

void CLCommand::AddWaitEvent(CLEvent* event) {
  for (vector<CLEvent*>::iterator it = wait_events.begin(); it != wait_events.end(); ++it) {
    if (event == *it) return;
  }
  wait_events.push_back(event);
  event->Retain();
}

bool CLCommand::Executable() {
  for (vector<CLEvent*>::iterator it = wait_events.begin(); it != wait_events.end(); ++it) {
    CLEvent* event = *it;
    if (event->status != CL_COMPLETE && event->status > 0) return false;

		if (event->status <0)
			this->event->waitEventsComplete = false;

    event->Release();
    wait_events.erase(it--);
  }
  return true;
}

bool CLCommand::Issuable() {
  for (vector<CLEvent*>::iterator it = wait_events.begin(); it != wait_events.end(); ++it) {
    CLEvent* event = *it;
    if (event->status == CL_COMPLETE || event->status < 0) {
			if (event->status <0)
				this->event->waitEventsComplete = false;
			event->Release();
			wait_events.erase(it--);
    }
    if (type != CL_COMMAND_NDRANGE_KERNEL || event->type != CL_COMMAND_NDRANGE_KERNEL) return false;
  }
  return true;
}

void CLCommand::Execute() {
  device->Execute(this);
}

void CLCommand::SetKernelArgs() {
  if (kernel == NULL) SNUCL_FATAL("COMMAND[%lu] KERNEL IS NULL", id);

#if 0
  map<cl_uint, CLKernelArg*>* args = &kernel->args;
  for (map<cl_uint, CLKernelArg*>::iterator it = args->begin(); it != args->end(); ++it) {
    cl_uint index = it->first;
    CLKernelArg* arg = it->second;
    if (kernel_args->count(index)) {
      CLKernelArg* arg_cmd = (*kernel_args)[index];
      arg_cmd->Init(arg->size, arg->local ? NULL : (const void*) arg->value);
      arg_cmd->mem = arg->mem;
    } else {
      CLKernelArg* arg_cmd = new CLKernelArg(arg->size, arg->local ? NULL : (const void*) arg->value);
      arg_cmd->mem = arg->mem;
      (*kernel_args)[index] = arg_cmd;
    }
  }
#else
  kernel_args->clear();
  map<cl_uint, CLKernelArg*>* args = &kernel->args;
  for (map<cl_uint, CLKernelArg*>::iterator it = args->begin(); it != args->end(); ++it) {
    cl_uint index = it->first;
    CLKernelArg* arg = it->second;

    CLKernelArg* arg_cmd = new CLKernelArg(arg->size, arg->local ? NULL : (const void*) arg->value);
    arg_cmd->mem = arg->mem;
    arg_cmd->sampler = arg->sampler;
    if(arg_cmd->mem) {
      arg_cmd->mem->SetCommand(this);
    }
    if(arg_cmd->sampler) {
      arg_cmd->mem->SetCommand(this);
    }
    (*kernel_args)[index] = arg_cmd;
  }

#endif

  /*
  if (kernel->args_dirty) {
    //TODO: delete previous args_last
    kernel->args_last = new map<cl_uint, CLKernelArg*>();

    map<cl_uint, CLKernelArg*>* args = &kernel->args;
    for (map<cl_uint, CLKernelArg*>::iterator it = args->begin(); it != args->end(); ++it) {
      cl_uint index = it->first;
      CLKernelArg* arg = it->second;
      CLKernelArg* arg_copy = new CLKernelArg(arg->size, arg->local ? NULL : (const void*) arg->value);
      arg_copy->mem = arg->mem;
      (*kernel->args_last)[index] = arg_copy;
    }

    kernel->args_dirty = false;
  }
  kernel_args = kernel->args_last;
  */
}

cl_event CLCommand::DisclosedToUser() {
  event->disclosed = true;
  return &event->st_obj;
}

CLMem::CLMem(CLContext* context, cl_mem_flags flags, size_t size, void* host_ptr, bool is_sub) {
  Init(context, flags, size, CL_MEM_OBJECT_BUFFER, host_ptr, is_sub);
}

CLMem::CLMem(CLContext* context, cl_mem_flags flags, const cl_image_format* image_format, size_t image_width, size_t image_height, size_t image_row_pitch, void* host_ptr) {
  memcpy(&this->image_format, image_format, sizeof(cl_image_format));
  this->image_row_pitch = image_row_pitch;

  if (image_format->image_channel_data_type != CL_FLOAT) SNUCL_ERROR("NOT SUPPORTED CHANNEL TYPE [%u]", image_format->image_channel_data_type);
  size_t size = image_width * image_height * sizeof(float);
  Init(context, flags, size, CL_MEM_OBJECT_IMAGE2D, host_ptr, false);
}

CLMem::~CLMem() {
  vector<MemObjectDestructorCallback>::reverse_iterator iit 
    = callbackStack.rbegin();
  for (iit=callbackStack.rbegin(); iit < callbackStack.rend(); ++iit) {
    (*iit).pfn_notify(&st_obj, (*iit).user_data);
  }

  pthread_mutex_lock(&mutex_dev_specific);
  for (map<CLDevice*, void*>::iterator it = dev_specific.begin(); it != dev_specific.end(); it++) {
    CLDevice* dev = it->first;
    CLCommand* command = CommandFactory::instance()->NewCommand(CL_COMMAND_FREE_BUFFER);
    command->mem_src = this;
    command->DisclosedToUser();
    dev->scheduler->EnqueueReadyQueue(command, dev);
    command->event->Wait();
  }
  dev_specific.clear();
  pthread_mutex_unlock(&mutex_dev_specific);

  pthread_mutex_destroy(&mutex_latest);
  pthread_mutex_destroy(&mutex_dev_specific);
}

void CLMem::Init(CLContext* context, cl_mem_flags flags, size_t size, cl_mem_object_type type, void* host_ptr, bool is_sub) {
  st_obj.c_obj = this;

  this->context = context;
  this->flags = flags == 0 ? CL_MEM_READ_WRITE : flags;
  this->size = size;
  this->type = type;
  this->alloc_host = false;
  this->use_host = false;
  this->space_host = NULL;
  this->space_host_valid = false;
  this->is_sub = is_sub;
  this->is_image = false;

  pthread_mutex_init(&mutex_latest, NULL);
  pthread_mutex_init(&mutex_dev_specific, NULL);

  context->AddMemory(this);
  if (!is_sub) {
    if (flags & CL_MEM_USE_HOST_PTR) {
      space_host = host_ptr;
      use_host = true;
    }
    else if (flags & CL_MEM_COPY_HOST_PTR) {
      space_host = memalign(4096, size);
      memcpy(space_host, host_ptr, size);
      alloc_host = true;
      use_host = true;
    }
  }

	Retain();
}

int CLMem::Release() {
	return CLObject::Release();
}

void CLMem::SetCommand(CLCommand* command) {
	commands.push_back(command);
}

CLSampler::CLSampler(CLContext* context, cl_bool normalized_coords, cl_addressing_mode addressing_mode, cl_filter_mode filter_mode) {
  st_obj.c_obj = this;
  this->context = context;
  this->normalized_coords = normalized_coords;
  this->addressing_mode = addressing_mode;
  this->filter_mode = filter_mode;

  pthread_mutex_init(&mutex_dev_specific, NULL);

  context->AddSampler(this);
}

void CLSampler::SetCommand(CLCommand* command) {
	commands.push_back(command);
}

CLDevice* CLMem::FrontLatest() {
  CLDevice* ret = NULL;
  pthread_mutex_lock(&mutex_latest);
  if (device_list.empty()) {
    pthread_mutex_unlock(&mutex_latest);
    return NULL;
  }
  ret = device_list.front();
  pthread_mutex_unlock(&mutex_latest);
  return ret;
}

bool CLMem::HasLatest(CLDevice* device) {
  pthread_mutex_lock(&mutex_latest);
  for (vector<CLDevice*>::iterator it = device_list.begin(); it != device_list.end(); ++it) {
    if (device == *it) {
      pthread_mutex_unlock(&mutex_latest);
      return true;
    }
  } 
  pthread_mutex_unlock(&mutex_latest);
  return false;
}

bool CLMem::EmptyLatest() {
  bool ret = false;
  pthread_mutex_lock(&mutex_latest);
  ret = device_list.empty();
  pthread_mutex_unlock(&mutex_latest);
  return ret;
}

void CLMem::AddLatest(CLDevice* device) {
  pthread_mutex_lock(&mutex_latest);
  if (device) device_list.push_back(device);
  pthread_mutex_unlock(&mutex_latest);
}

void CLMem::ClearLatest(CLDevice* device) {
  pthread_mutex_lock(&mutex_latest);
  device_list.clear();
  if (device) device_list.push_back(device);
  pthread_mutex_unlock(&mutex_latest);
}

CLProgram::CLProgram(CLContext* context, const cl_device_id* device_list, cl_uint num_devices) {
  st_obj.c_obj = this;

  this->context = context;
	src = NULL;

  pthread_mutex_init(&mutex_dev_specific, NULL);

	if (device_list) {
		for (cl_uint i = 0; i < num_devices; i++) {
			devices.push_back(device_list[i]->c_obj);
			buildStatus[device_list[i]->c_obj] = CL_BUILD_NONE;
			fromSource[device_list[i]->c_obj] = false;
			fromBinary[device_list[i]->c_obj] = false;
			createWithBinary[device_list[i]->c_obj] = false;
			fromObj[device_list[i]->c_obj] = false;
		}
	}
	else {
		for (cl_uint i = 0; i < context->devices.size(); i++) {
			devices.push_back(context->devices[i]);
			buildStatus[context->devices[i]] = CL_BUILD_NONE;
			fromSource[context->devices[i]] = false;
			fromBinary[context->devices[i]] = false;
			createWithBinary[context->devices[i]] = false;
			fromObj[context->devices[i]] = false;
		}
	}

	options = NULL;
	optArgInfo = false;
	createLibrary = false;

	//kernel information
	kernel_names = NULL;
	kernel_num_args = NULL;
	kernel_attributes = NULL;
	kernel_work_group_size_hint = NULL;
	kernel_reqd_work_group_size = NULL;
	kernel_local_mem_size = NULL;
	kernel_private_mem_size = NULL;
	kernel_arg_address_qualifier = NULL;
	kernel_arg_access_qualifier = NULL;
	kernel_arg_type_name = NULL;
	kernel_arg_type_qualifier = NULL;
	kernel_arg_name = NULL;
	isBuiltInKernel = NULL;
	buildVersion = 0;

	Retain();
}

CLProgram::~CLProgram() {
  pthread_mutex_destroy(&mutex_dev_specific);
  free(src);
	if (options)
		free(options);
  for (map<CLDevice*, char*>::iterator it = buildLog.begin(); it != buildLog.end(); ++it) 
		free(it->second);

	for (int i=0; i<devices.size(); i++) {
		if(dev_specific.count(devices[i]))
			devices[i]->FreeProgramInfo(this);
	}

	FreeKernelInfo();
}
void CLProgram::FreeKernelInfo(){

	if(isBuiltInKernel) free(isBuiltInKernel);
	if(kernel_arg_name) {
		for (uint i=0; i<this->num_kernels; i++) {
			for (uint j=0; j<this->kernel_num_args[i]; j++) {
				free(kernel_arg_name[i][j]);
			}
			free(kernel_arg_name[i]);
		}
		free(kernel_arg_name);
	}
	if(kernel_arg_type_qualifier) {
		for (uint i=0; i<this->num_kernels; i++)
			free(kernel_arg_type_qualifier[i]);
		free(kernel_arg_type_qualifier);
	}
	if(kernel_arg_type_name) {
		for (uint i=0; i<this->num_kernels; i++) {
			for (uint j=0; j<this->kernel_num_args[i]; j++) {
				free(kernel_arg_type_name[i][j]);
			}
			free(kernel_arg_type_name[i]);
		}
		free(kernel_arg_type_name);
	}
	if(kernel_arg_access_qualifier) {
		for (uint i=0; i<this->num_kernels; i++) 
			free(kernel_arg_access_qualifier[i]);
		free(kernel_arg_access_qualifier);
	}
	if(kernel_arg_address_qualifier) {
		for (uint i=0; i<this->num_kernels; i++) 
			free(kernel_arg_address_qualifier[i]);
		free(kernel_arg_address_qualifier);
	}
	if(kernel_private_mem_size) free(kernel_private_mem_size);
	if(kernel_local_mem_size) free(kernel_local_mem_size);
	if(kernel_reqd_work_group_size) {
		for (uint i=0; i<this->num_kernels; i++) 
			free(kernel_reqd_work_group_size[i]);
		free(kernel_reqd_work_group_size);
	}
	if(kernel_work_group_size_hint) {
		for (uint i=0; i<this->num_kernels; i++) 
			free(kernel_work_group_size_hint[i]);
		free(kernel_work_group_size_hint);
	}
	if(kernel_attributes) {
		for (uint i=0; i<this->num_kernels; i++) 
			free(kernel_attributes[i]);
		free(kernel_attributes);
	}
	if(kernel_num_args) free(kernel_num_args);
	if(kernel_names) {
		for (uint i=0; i<this->num_kernels; i++) 
			free(kernel_names[i]);
		free(kernel_names);
	}
}

void CLProgram::SetObj(CLDevice* dev, char* idx) {
	if (compiledObject.count(dev) <= 0)
		compiledObject[dev] = (char*)malloc(11 * sizeof(char));

	strcpy(compiledObject[dev], idx);
	fromObj[dev] = true;
}
void CLProgram::SetSrc(CLDevice* dev, char* _src) {
  size_t len_src = strlen(_src);
  src = (char*) calloc(len_src + 1, sizeof(char));
  strncpy(src, _src, len_src);

	fromBinary[dev] = false;
	fromSource[dev] = true;
}
void CLProgram::SetBin(CLDevice* dev, CLBinary* _bin, cl_program_binary_type _type) {
	if (bins[dev])
		delete bins[dev];

	bins[dev] = _bin;

	if (_type == CL_PROGRAM_BINARY_TYPE_EXECUTABLE) {
		fromBinary[dev] = true;
		fromSource[dev] = false;
	}
}
void CLProgram::CheckOptions(const char* options, cl_int* err) {
	if (options) {
		string test (options);
		string str ("-cl-kernel-arg-info");

		size_t pos = test.find(str);

		if (pos != test.npos)
			this->optArgInfo = true;

		//need recompile
		if (this->options == NULL || strcmp(this->options, options) != 0) {
			for (int i=0; i<devices.size(); i++) {
				if (!createWithBinary[devices[i]]) {
					fromBinary[devices[i]] = false;
					fromSource[devices[i]] = true;
					fromObj[devices[i]] = false;
				}
			}
			buildCallbackStack.clear();
		}
	}
	else {
		//need recompile
		if (this->options) {
			for (int i=0; i<devices.size(); i++) {
				if (!createWithBinary[devices[i]]) {
					fromBinary[devices[i]] = false;
					fromSource[devices[i]] = true;
					fromObj[devices[i]] = false;
				}
			}
			buildCallbackStack.clear();
		}
	}

	err = CL_SUCCESS;
}
void CLProgram::CheckLinkOptions(char* newOptions, const char* options, cl_int* err) {
	string opt (options);
	string str[7];
	str[0] = "-create-library";
	str[1] = "-enable-link-options";
	str[2] = "-cl-denorms-are-zero";
	str[3] = "-cl-no-signed-zeroes";
	str[4] = "-cl-unsafe-math-optimizations";
	str[5] = "-cl-finite-math-only";
	str[6] = "-cl-fast-relaxed-math";

	string pre, post, ret;
	size_t pos;

	for (int i=0; i<7; i++) {
		pos = opt.find(str[i]);

		if (pos != opt.npos) {
			if (i==0)
				this->createLibrary = true;
			pre = opt.substr(0, pos);
			post = opt.substr(pos+str[i].length());
			opt = pre + post; 
		}
	}

	strncpy(newOptions, opt.c_str(), opt.length());
}

CLBinary::CLBinary(char* _binary, size_t _size, cl_program_binary_type _type) {
	binary = _binary;
	size = _size;
	type = _type;
}

CLKernel::CLKernel(CLProgram* program, const char* kernel_name) {
  st_obj.c_obj = this;

  this->program = program;
  strcpy(name, kernel_name);

  args_last = NULL;
  args_dirty = true;

  pthread_mutex_init(&mutex_dev_specific, NULL);
	Retain();
}

CLKernel::~CLKernel() {
  pthread_mutex_destroy(&mutex_dev_specific);
}
void CLKernel::SetCommand(CLCommand* command) {
	commands.push_back(command);
}

cl_int CLKernel::SetArg(cl_uint arg_index, size_t arg_size, const void* arg_value) {
  CLKernelArg* arg;
  map<cl_uint, CLKernelArg*>::iterator it = args.find(arg_index);
  if (it == args.end()) {
    arg = new CLKernelArg(arg_size, arg_value);
  } else {
    arg = it->second;
    arg->Init(arg_size, arg_value);
  }

  if (arg_value) {
    cl_mem m = *(cl_mem*) arg_value;
    vector<CLMem*>* mems = &program->context->mems;
    for (vector<CLMem*>::iterator it = mems->begin(); it != mems->end(); ++it) {
      CLMem* mem = *it;
      if (m == &(mem->st_obj)) {
        arg->mem = mem;
				if(arg_size != sizeof(cl_mem))
					return CL_INVALID_ARG_SIZE;
        break;
      }
    }

    if (arg->mem == NULL) {
      cl_sampler s = *(cl_sampler*) arg_value;
      vector<CLSampler*>* samplers = &program->context->samplers;
      for (vector<CLSampler*>::iterator it = samplers->begin(); it != samplers->end(); ++it) {
        CLSampler* sampler = *it;
        if (s == &(sampler->st_obj)) {
          arg->sampler = sampler;
          if(arg_size != sizeof(cl_sampler))
            return CL_INVALID_ARG_SIZE;
          break;
        }
      }
    }
  }

  args[arg_index] = arg;

  args_dirty = true;

  return CL_SUCCESS;
}

CLKernelArg::CLKernelArg(size_t arg_size, const void* arg_value) {
  Init(arg_size, arg_value);
}

CLKernelArg::~CLKernelArg() {

}

void CLKernelArg::Init(size_t arg_size, const void* arg_value) {
  this->size = arg_size;
  if (arg_value) memcpy(value, arg_value, size);
  local = arg_value == NULL;
  mem = NULL;
  sampler = NULL;
}

CLEvent::CLEvent(cl_context context, cl_command_type type, cl_command_queue queue, CLCommand* command) {
  st_obj.c_obj = this;

  this->type = type;
	this->context = context;
	this->queue = queue;
  this->command = command;

  pthread_mutex_init(&mutex_c, NULL);
  SetStatus(CL_QUEUED);

	user = false;
  disclosed = false;
  userSetStatus = false;
	waitEventsComplete = true;

  profiling_info[0] = profiling_info[1] = profiling_info[2] = profiling_info[3] = 0;

  sem_init(&sem_complete, 0, 0);

  pthread_mutex_init(&mutex_dev_specific, NULL);

	Retain();
}

CLEvent::~CLEvent() {
  sem_destroy(&sem_complete);
  pthread_mutex_destroy(&mutex_dev_specific);
  pthread_mutex_destroy(&mutex_c);
}

cl_int CLEvent::Wait() {
	Retain();
  if (status != CL_COMPLETE && status > 0) {
    sem_wait(&sem_complete);
  }
	Release();

	if (status < 0) return status;
	return CL_COMPLETE;
}

void CLEvent::Complete() {
  if(queue)
    if(queue->c_obj->IsProfiled())
      SetTimestamp(CL_PROFILING_COMMAND_END);
	if(waitEventsComplete)
		SetStatus(CL_COMPLETE);
	else
		SetStatus(-1); //the events that current event is waiting are erroneously tereminated
  if (g_platform.cluster) g_platform.profiler->CompleteCommand(command);
}

void CLEvent::SetTimestamp(int type) {
  profiling_info[type - CL_PROFILING_COMMAND_QUEUED] = SNUCL_Timestamp();
}
void CLEvent::SetStatus(cl_int status) {
	this->status = status;

	vector<EventCallback*> callbackStack_copy;
	pthread_mutex_lock(&mutex_c);
	callbackStack_copy.resize(callbackStack.size());
	copy(callbackStack.begin(), callbackStack.end(), callbackStack_copy.begin());
	pthread_mutex_unlock(&mutex_c);

	if(status == CL_COMPLETE || status < 0) {
		for (vector<EventCallback*>::iterator it = callbackStack_copy.begin(); it != callbackStack_copy.end(); ++it) {
			if((*it)->command_exec_callback_type == CL_COMPLETE)
				(*it)->pfn_notify(&st_obj, status, (*it)->user_data);
		}
		sem_post(&sem_complete);
	}
	else { //CL_SUBMITTED , CL_RUNNING
		for (vector<EventCallback*>::iterator it = callbackStack_copy.begin(); it != callbackStack_copy.end(); ++it) {
			if((*it)->command_exec_callback_type == status)
				(*it)->pfn_notify(&st_obj, status, (*it)->user_data);
		}
	}

}

void CLEvent::AddCallback(void (CL_CALLBACK *pfn_notify)(cl_event, cl_int, void *), void * user_data, cl_int command_exec_callback_type) {
	int call_immediately = 0;
	pthread_mutex_lock(&mutex_c);
	if (status <= command_exec_callback_type) {
		call_immediately = 1;
	} else {
		EventCallback* ec = new EventCallback(pfn_notify, user_data, command_exec_callback_type);
		callbackStack.push_back(ec);
	}
	pthread_mutex_unlock(&mutex_c);

	if (call_immediately) {
		pfn_notify(&st_obj, status, user_data);
	}
}

CLDeviceWorker::CLDeviceWorker(CLDevice* device) {
  this->device = device;
  pthread_create(&thread, NULL, &CLDeviceWorker::ThreadFunc, this);
  SNUCL_SetThreadAffinity(&thread, 1);
}

CLDeviceWorker::~CLDeviceWorker() {
  running = false;
  pthread_cancel(thread);
}

void* CLDeviceWorker::ThreadFunc(void *argp) {
  ((CLDeviceWorker*) argp)->Run();
  return NULL;
}

void CLDeviceWorker::Run() {
  device->Init();
  device->InitInfo();
  sem_post(&g_platform.sem_dev);

  running = true;

  while (running) {
    sem_wait(&device->sem_rq);
    CLCommand* command = NULL;
    if (device->ready_queue->Dequeue(&command)) {
      device->Execute(command);
    }
  }
}

CLScheduler::CLScheduler(CLPlatform* platform) {
  this->platform = platform;
  thread = (pthread_t) NULL;
  devices = new CLDevice*[MAX_NUM_DEVICES_PER_SCHEDULER];
  num_devices = 0;
  ready_queue = new LockFreeQueueMS(MAX_NUM_COMMANDS_IN_READY_QUEUE);

  pthread_mutex_init(&mutex_rc, NULL);
}

CLScheduler::~CLScheduler() {
  if (thread) {
    running = false;
  }
  pthread_mutex_destroy(&mutex_rc);
  delete ready_queue;
  delete devices;
}

void CLScheduler::AddDevice(CLDevice* device) {
  if (platform->cluster) SNUCL_DEBUG("SCHEDULER[%lu] ADD_DEVICE[%lu]", id, device->id);
  devices[num_devices++] = device;
  device->scheduler = this;
	device->devIdxInScheduler = num_devices-1;
  device->issuer = platform->issuer;
  if (num_devices >= MAX_NUM_DEVICES_PER_SCHEDULER) SNUCL_ERROR("NUM DEVICES IN THIS SCHEDULER[%lu] IS [%d]", id, num_devices);
}

void CLScheduler::Start() {
  pthread_create(&thread, NULL, &CLScheduler::ThreadFunc, this);
}

bool CLScheduler::EnqueueReadyQueue(CLCommand* command, CLDevice* device) {
  if (device == NULL) SNUCL_ERROR("DEVICE IS NULL COMMAND[%lu] TYPE[%x]", command->id, command->type);
  command->device = device;
  UpdateLatest(command, device);
#if 0
  if (platform->cluster) {
    device->Prepare(command);
    return platform->issuer->EnqueueReadyQueue(command, device);
  }
  return device->EnqueueReadyQueue(command);
#else
  if (platform->cluster) {
    device->Prepare(command);
    return ready_queue->Enqueue(command);
  }
  return device->EnqueueReadyQueue(command);
#endif
}

bool CLScheduler::EnqueueRunningCommand(CLCommand* command) {
  pthread_mutex_lock(&mutex_rc);
  running_commands.push_back(command);
  pthread_mutex_unlock(&mutex_rc);
  return true;
}

void* CLScheduler::ThreadFunc(void* argp) {
  ((CLScheduler*) argp)->Run();
  return NULL;
}

void CLScheduler::Run() {
  if (num_devices == 0) return;

  running = true;

  while (running) {
    for (int i = 0; i < num_devices; i++) {
      CLDevice* device = devices[i];
			if (!device) {//released device
				continue;
			}
      for (int j = 0; j < device->num_command_queues; j++) {
        CLCommandQueue* command_queue = device->command_queues[j];
        if (!command_queue->IsInOrder()) pthread_mutex_lock(&command_queue->mutex_q);
        CLCommand* command = Schedule(command_queue);
        if (command) Issue(command);
        if (!command_queue->IsInOrder()) pthread_mutex_unlock(&command_queue->mutex_q);
      }
    }

#if 0
    pthread_mutex_lock(&mutex_rc);
    for (vector<CLCommand*>::iterator it = running_commands.begin(); it != running_commands.end(); ++it) {
      CLCommand* command = *it;
      CLEvent* event = command->event;
      if (!command->device->IsComplete(event)) continue;
      event->Complete();
      running_commands.erase(it--);
      SNUCL_INFO("COMPLETE EVENT[%lu] TYPE[%x] COMMAND[%lu]", event->id, event->type, command->id);
    }
    pthread_mutex_unlock(&mutex_rc);
#endif

  }

}

CLCommand* CLScheduler::Schedule(CLCommandQueue* command_queue) {
  if (command_queue->IsInOrder()) {
    LockFreeQueue* commands_io = command_queue->commands_io;
    if (commands_io->Size() == 0) return NULL;
    CLCommand* command = NULL;
    if (!commands_io->Peek(&command)) return NULL;
    return command->Executable() ? command : NULL;
  }

  list<CLCommand*>* commands = &command_queue->commands;
  if (commands->empty()) return NULL;

  bool found = false;
  CLCommand* command = NULL;
  for (list<CLCommand*>::iterator it = commands->begin(); it != commands->end(); ++it) {
    command = *it;

    if (command->type == CL_COMMAND_MARKER || command->type == CL_COMMAND_BARRIER)
			found = (it == commands->begin());
		else
			found = command->Executable();

    if (found || command->type == CL_COMMAND_MARKER || command->type == CL_COMMAND_BARRIER || command->type == CL_COMMAND_WAIT_FOR_EVENTS) break;
  }
  return found ? command : NULL;
}

void CLScheduler::Issue(CLCommand* command) {
  bool issued = true;
  switch (command->type) {
    case CL_COMMAND_NDRANGE_KERNEL:     issued = IssueLaunchKernel(command);      break;
    case CL_COMMAND_NATIVE_KERNEL:      issued = IssueNativeKernel(command);      break;
    case CL_COMMAND_READ_BUFFER:        issued = IssueReadBuffer(command);        break;
    case CL_COMMAND_WRITE_BUFFER:       issued = IssueWriteBuffer(command);       break;
    case CL_COMMAND_COPY_BUFFER:        issued = IssueCopyBuffer(command);        break;
//    case CL_COMMAND_BROADCAST_BUFFER:   issued = IssueBroadcastBuffer(command);    break;
  }
  if (issued) {
    CLDevice* device = command->device;
    CLCommandQueue* command_queue = command->command_queue;
    command_queue->Dequeue(command);
		if(command_queue->IsProfiled())
			command->event->SetTimestamp(CL_PROFILING_COMMAND_SUBMIT);
    command->event->SetStatus(CL_SUBMITTED);
    EnqueueReadyQueue(command, device);
  }
}

bool CLScheduler::IssueLaunchKernel(CLCommand* command) {
  bool issued = true;
  CLDevice* dev = command->device;
  map<cl_uint, CLKernelArg*>* args = command->kernel_args;

  int num_mem = 0;
  CLMem* mem_list[512];

  for (map<cl_uint, CLKernelArg*>::iterator it = args->begin(); it != args->end(); ++it) {
    CLKernelArg* arg = it->second;
    CLMem* mem = arg->mem;
    if (!mem) continue;
    if (mem->flags & CL_MEM_WRITE_ONLY || mem->flags & CL_MEM_READ_WRITE) {
      mem_list[num_mem++] = mem;
    }
  }
  if (num_mem > 512) SNUCL_ERROR("RACE CHECK OVERFLOW MEM[%d]", num_mem);

  if (CheckRace(mem_list, num_mem, command)) return false;

  for (map<cl_uint, CLKernelArg*>::iterator it = args->begin(); it != args->end(); ++it) {
    CLKernelArg* arg = it->second;
    CLMem* mem = arg->mem;
    if (!mem) continue;

    if (mem->alloc_host && !mem->HasLatest(dev)) {
      CLCommand* w_command;
      w_command = CommandFactory::instance()->NewCommand(CL_COMMAND_WRITE_BUFFER);
      w_command->mem_dst = mem;
      w_command->off_dst = 0;
      w_command->cb = mem->size;
      w_command->ptr = mem->space_host;
      w_command->misc[0] = CL_COMMAND_NDRANGE_KERNEL;

      w_command->DisclosedToUser();
      EnqueueReadyQueue(w_command, dev);

      command->AddWaitEvent(w_command->event);

      issued = false;
    } else if (!mem->EmptyLatest() && !mem->HasLatest(dev)) {
      CLDevice* dev_src = NearestDevice(mem, dev);
      if (dev_src == NULL) SNUCL_ERROR("DEV_SRC IS NULL MEM[%lu]", mem->id);
      if (dev_src->node_id == dev->node_id) {
        CLCommand* cb_command = CommandFactory::instance()->NewCommand(CL_COMMAND_COPY_BUFFER);
        cb_command->mem_src = mem;
        cb_command->mem_dst = mem;
        cb_command->off_src = 0;
        cb_command->off_dst = 0;
        cb_command->cb = mem->size;
        cb_command->dev_src = dev_src;
        cb_command->misc[0] = CL_COMMAND_NDRANGE_KERNEL;
        cb_command->DisclosedToUser();
        EnqueueReadyQueue(cb_command, dev);

        command->AddWaitEvent(cb_command->event);
        issued = false;
      } else {
        CLCommand* rb_command = CommandFactory::instance()->NewCommand(CL_COMMAND_RECV_BUFFER);
        rb_command->mem_dst = mem;
        rb_command->off_dst = 0;
        rb_command->cb = mem->size;
        rb_command->dev_src = dev_src;
        rb_command->misc[0] = CL_COMMAND_NDRANGE_KERNEL;
        rb_command->DisclosedToUser();
        EnqueueReadyQueue(rb_command, dev);

        CLCommand* sb_command = CommandFactory::instance()->NewCommand(CL_COMMAND_SEND_BUFFER);
        sb_command->mem_src = mem;
        sb_command->off_src = 0;
        sb_command->cb = mem->size;
        sb_command->dev_dst = dev;
        sb_command->misc[0] = rb_command->event->id;
        sb_command->DisclosedToUser();
        EnqueueReadyQueue(sb_command, dev_src);

        command->AddWaitEvent(rb_command->event);
        issued = false;
      }
    } else if (mem->HasLatest(dev) || mem->EmptyLatest()) {
    } else {
      SNUCL_ERROR("%s", "THIS CONDITION IS IMPOSSIBLE");
    }
  }
  return issued;
}

bool CLScheduler::IssueNativeKernel(CLCommand* command) {
  bool issued = true;
  CLDevice* dev = command->device;
  cl_uint num_mem_ptrs = (*command->kernel_args)[1]->size / sizeof(cl_mem);
  cl_mem* mem_ptr_list = (cl_mem*) (*command->kernel_args)[1]->value;

  int num_mem = 0;
  CLMem* mem_list[512];

  for (int i = 0; i < num_mem_ptrs; ++i) {
    CLMem* mem = mem_ptr_list[i]->c_obj;
    if (!mem) continue;
    if (mem->flags & CL_MEM_WRITE_ONLY || mem->flags & CL_MEM_READ_WRITE) {
      mem_list[num_mem++] = mem;
    }
  }
  if (num_mem > 512) SNUCL_ERROR("RACE CHECK OVERFLOW MEM[%d]", num_mem);

  if (CheckRace(mem_list, num_mem, command)) return false;

  for (int i = 0; i < num_mem_ptrs; ++i) {
    CLMem* mem = mem_ptr_list[i]->c_obj;
    if (!mem) continue;

    if (mem->alloc_host && !mem->HasLatest(dev)) {
      CLCommand* w_command;
      if (mem->is_image) {
        w_command = CommandFactory::instance()->NewCommand(CL_COMMAND_WRITE_IMAGE);
        w_command->mem_dst = mem;
        w_command->src_origin[0] = 0;
        w_command->src_origin[1] = 0;
        w_command->src_origin[2] = 0;
        w_command->dst_origin[0] = 0;
        w_command->dst_origin[1] = 0;
        w_command->dst_origin[2] = 0;
        w_command->region[0] = mem->image_desc.image_width;
        w_command->region[1] = mem->image_desc.image_height == 0 ? 1 : mem->image_desc.image_height;
        w_command->region[2] = mem->image_desc.image_depth == 0 ? 1 : mem->image_desc.image_depth;
        w_command->src_row_pitch = 0;
        w_command->src_slice_pitch = 0;

        w_command->ptr = mem->space_host;
        w_command->misc[0] = CL_COMMAND_NDRANGE_KERNEL;
      } else {
        w_command = CommandFactory::instance()->NewCommand(CL_COMMAND_WRITE_BUFFER);
        w_command->mem_dst = mem;
        w_command->off_dst = 0;
        w_command->cb = mem->size;
        w_command->ptr = mem->space_host;
        w_command->misc[0] = CL_COMMAND_NDRANGE_KERNEL;
      }
      w_command->DisclosedToUser();
      EnqueueReadyQueue(w_command, dev);

      command->AddWaitEvent(w_command->event);

      issued = false;
    } else if (!mem->EmptyLatest() && !mem->HasLatest(dev)) {
      CLDevice* dev_src = NearestDevice(mem, dev);
      if (dev_src == NULL) SNUCL_ERROR("DEV_SRC IS NULL MEM[%lu]", mem->id);
      if (dev_src->node_id == dev->node_id) {
        CLCommand* cb_command = CommandFactory::instance()->NewCommand(CL_COMMAND_COPY_BUFFER);
        cb_command->mem_src = mem;
        cb_command->mem_dst = mem;
        cb_command->off_src = 0;
        cb_command->off_dst = 0;
        cb_command->cb = mem->size;
        cb_command->dev_src = dev_src;
        cb_command->misc[0] = CL_COMMAND_NDRANGE_KERNEL;
        cb_command->DisclosedToUser();
        EnqueueReadyQueue(cb_command, dev);

        command->AddWaitEvent(cb_command->event);
        issued = false;
      } else {
        CLCommand* rb_command = CommandFactory::instance()->NewCommand(CL_COMMAND_RECV_BUFFER);
        rb_command->mem_dst = mem;
        rb_command->off_dst = 0;
        rb_command->cb = mem->size;
        rb_command->dev_src = dev_src;
        rb_command->misc[0] = CL_COMMAND_NDRANGE_KERNEL;
        rb_command->DisclosedToUser();
        EnqueueReadyQueue(rb_command, dev);

        CLCommand* sb_command = CommandFactory::instance()->NewCommand(CL_COMMAND_SEND_BUFFER);
        sb_command->mem_src = mem;
        sb_command->off_src = 0;
        sb_command->cb = mem->size;
        sb_command->dev_dst = dev;
        sb_command->misc[0] = rb_command->event->id;
        sb_command->DisclosedToUser();
        EnqueueReadyQueue(sb_command, dev_src);

        command->AddWaitEvent(rb_command->event);
        issued = false;
      }
    } else if (mem->HasLatest(dev) || mem->EmptyLatest()) {
    } else {
      SNUCL_ERROR("%s", "THIS CONDITION IS IMPOSSIBLE");
    }
  }
  return issued;
}

bool CLScheduler::IssueReadBuffer(CLCommand* command) {
  CLMem* mem = command->mem_src;
  CLDevice* dev = command->device;
  if (!mem->HasLatest(dev) && !mem->EmptyLatest()) {
    command->device = NearestDevice(mem, dev);
  }

  return true;
}

bool CLScheduler::IssueWriteBuffer(CLCommand* command) {
  CLMem* mem_dst = command->mem_dst;
  size_t off_dst = command->off_dst;
  size_t size = command->cb;
  CLDevice* dev_dst = command->device;

  if (CheckRace(&mem_dst, 1, command)) return false;

  if (mem_dst->alloc_host && !(off_dst == 0 && size == mem_dst->size)) {
    memcpy((void*) ((size_t) mem_dst->space_host + off_dst), command->ptr, size);
    command->ptr = mem_dst->space_host;
    command->off_dst = 0;
    command->cb = mem_dst->size;
    return true;
  } else if (mem_dst->EmptyLatest() || mem_dst->HasLatest(dev_dst) || (off_dst == 0 && size == mem_dst->size)) {
    return true;
  } else {
    CLDevice* dev_src = NearestDevice(mem_dst, dev_dst);
    if (dev_src == NULL) SNUCL_ERROR("DEV_SRC IS NULL MEM[%lu]", mem_dst->id);
    if (dev_src->node_id == dev_dst->node_id) {
      CLCommand* cb_command = CommandFactory::instance()->NewCommand(CL_COMMAND_COPY_BUFFER);
      cb_command->mem_src = mem_dst;
      cb_command->mem_dst = mem_dst;
      cb_command->off_src = 0;
      cb_command->off_dst = 0;
      cb_command->cb = mem_dst->size;
      cb_command->dev_src = dev_src;
      cb_command->DisclosedToUser();
      EnqueueReadyQueue(cb_command, dev_dst);

      command->AddWaitEvent(cb_command->event);
      return false;
    } else {
      CLCommand* rb_command = CommandFactory::instance()->NewCommand(CL_COMMAND_RECV_BUFFER);
      rb_command->mem_src = mem_dst;
      rb_command->mem_dst = mem_dst;
      rb_command->off_dst = 0;
      rb_command->cb = mem_dst->size;
      rb_command->dev_src = dev_src;
      rb_command->DisclosedToUser();
      EnqueueReadyQueue(rb_command, dev_dst);

      CLCommand* sb_command = CommandFactory::instance()->NewCommand(CL_COMMAND_SEND_BUFFER);
      sb_command->mem_src = mem_dst;
      sb_command->mem_dst = mem_dst;
      sb_command->off_src = 0;
      sb_command->cb = mem_dst->size;
      sb_command->dev_dst = dev_dst;
      sb_command->misc[0] = rb_command->event->id;
      sb_command->DisclosedToUser();
      EnqueueReadyQueue(sb_command, dev_src);

      command->AddWaitEvent(rb_command->event);
      return false;
    }
  }
}

bool CLScheduler::IssueCopyBuffer(CLCommand* command) {
  CLDevice* dev_dst = command->device;
  CLMem* mem_src = command->mem_src;
  CLMem* mem_dst = command->mem_dst;
  size_t off_src = command->off_src;
  size_t off_dst = command->off_dst;
  size_t size = command->cb;

  if (CheckRace(&mem_dst, 1, command)) return false;

  if (mem_src->use_host) {
    command->SetType(CL_COMMAND_WRITE_BUFFER);
    command->ptr = (void*) ((size_t) mem_src->space_host + off_src);
    return true;
	}
	else if (mem_dst->use_host) {
    command->SetType(CL_COMMAND_READ_BUFFER);
    command->ptr = (void*) ((size_t) mem_dst->space_host + off_dst);
    return true;
	}
	else if (mem_src->alloc_host && mem_dst->alloc_host) {
    SNUCL_CHECK();
    memcpy((void*) ((size_t) mem_dst->space_host + off_dst), (void*) ((size_t) mem_src->space_host + off_src), size);
    command->SetType(CL_COMMAND_WRITE_BUFFER);
    command->off_dst = 0;
    command->cb = mem_dst->size;
    command->ptr = mem_dst->space_host;
    return true;
  } else if (mem_src->alloc_host) {
    SNUCL_CHECK();
    if (mem_dst->EmptyLatest() || mem_dst->HasLatest(dev_dst) || (off_dst == 0 && size == mem_dst->size)) {
    SNUCL_CHECK();
      command->SetType(CL_COMMAND_WRITE_BUFFER);
      command->ptr = (void*) ((size_t) mem_src->space_host + off_src);
      return true;
    } else {
    SNUCL_CHECK();
      mem_dst->alloc_host = true;
			if(!mem_dst->space_host)
				mem_dst->space_host = memalign(4096, mem_dst->size);

      CLDevice* dev_src = PreferCPUDevice(mem_dst);
      CLCommand* rb_command = CommandFactory::instance()->NewCommand(CL_COMMAND_READ_BUFFER);
      rb_command->mem_src = mem_dst;
      rb_command->off_src = 0;
      rb_command->ptr = mem_dst->space_host;
      rb_command->DisclosedToUser();
      EnqueueReadyQueue(rb_command, dev_src);

      command->AddWaitEvent(rb_command->event);

      return false;
    }
  } else if (mem_dst->alloc_host) {
    SNUCL_CHECK();
    if (mem_src->EmptyLatest()) {
    SNUCL_CHECK();
      mem_src->alloc_host = true;
      mem_src->space_host = memalign(4096, mem_src->size);
      return false;
    } else if (mem_src->HasLatest(dev_dst)) {
    SNUCL_CHECK();
      if (off_dst == 0 && size == mem_dst->size) {
    SNUCL_CHECK();
        command->dev_src = dev_dst;
        return true;
      } else {
    SNUCL_CHECK();
        CLCommand* wb_command = CommandFactory::instance()->NewCommand(CL_COMMAND_WRITE_BUFFER);
        wb_command->mem_dst = mem_dst;
        wb_command->off_dst = 0;
        wb_command->cb = mem_dst->size;
        wb_command->ptr = mem_dst->space_host;
        wb_command->DisclosedToUser();
        EnqueueReadyQueue(wb_command, dev_dst);

        command->AddWaitEvent(wb_command->event);
        command->dev_src = dev_dst;
        return false;
      }
    } else {
    SNUCL_CHECK();
      CLDevice* dev_src = NearestDevice(mem_src, dev_dst);
      if (dev_src == NULL) SNUCL_ERROR("DEV_SRC IS NULL MEM[%lu]", mem_src->id);
      if (off_dst == 0 && size == mem_dst->size) {
    SNUCL_CHECK();
        if (dev_src->node_id == dev_dst->node_id) {
    SNUCL_CHECK();
          command->dev_src = dev_src;
          return true;
        } else {
    SNUCL_CHECK();
          CLCommand* sb_command = CommandFactory::instance()->NewCommand(CL_COMMAND_SEND_BUFFER);
          sb_command->mem_src = mem_src;
          sb_command->mem_dst = mem_dst;
          sb_command->off_src = off_src;
          sb_command->off_dst = off_dst;
          sb_command->cb = size;
          sb_command->dev_dst = dev_dst;
          sb_command->misc[0] = command->event->id;
          sb_command->DisclosedToUser();
          EnqueueReadyQueue(sb_command, dev_src);

          command->SetType(CL_COMMAND_RECV_BUFFER);
          command->dev_src = dev_src;
          return true;
        }
      } else {
    SNUCL_CHECK();
        if (dev_src->node_id == dev_dst->node_id) {
    SNUCL_CHECK();
          CLCommand* wb_command = CommandFactory::instance()->NewCommand(CL_COMMAND_WRITE_BUFFER);
          wb_command->mem_dst = mem_dst;
          wb_command->off_dst = 0;
          wb_command->cb = mem_dst->size;
          wb_command->ptr = mem_dst->space_host;
          wb_command->DisclosedToUser();
          EnqueueReadyQueue(wb_command, dev_dst);

          command->AddWaitEvent(wb_command->event);
          command->dev_src = dev_src;
          return false;
        } else {
    SNUCL_CHECK();
          CLCommand* rb_command = CommandFactory::instance()->NewCommand(CL_COMMAND_READ_BUFFER);
          rb_command->mem_src = mem_src;
          rb_command->off_src = off_src;
          rb_command->cb = size;
          rb_command->ptr = (void*) ((size_t) mem_dst->space_host + off_dst);
          rb_command->DisclosedToUser();
          EnqueueReadyQueue(rb_command, dev_src);

          command->SetType(CL_COMMAND_WRITE_BUFFER);
          command->mem_dst = mem_dst;
          command->off_dst = 0;
          command->cb = mem_dst->size;
          command->ptr = mem_dst->space_host;

          command->AddWaitEvent(rb_command->event);
          return false;
        }
      }
    }
  } else if (mem_src->HasLatest(dev_dst) && mem_dst->HasLatest(dev_dst)) {
    SNUCL_CHECK();
    command->dev_src = dev_dst;
    return true;
  } else if (mem_src->HasLatest(dev_dst)) {
    SNUCL_CHECK();
    if (off_dst == 0 && size == mem_dst->size) {
    SNUCL_CHECK();
      command->dev_src = dev_dst;
      return true;
    } else {
    SNUCL_CHECK();
      CLDevice* dev_src = NearestDevice(mem_dst, dev_dst);
      if (dev_src == NULL) {
    SNUCL_CHECK();
        command->dev_src = dev_dst;
        return true;
      } else if (dev_src->node_id == dev_dst->node_id) {
    SNUCL_CHECK();
        CLCommand* cb_command = CommandFactory::instance()->NewCommand(CL_COMMAND_COPY_BUFFER);
        cb_command->mem_src = mem_dst;
        cb_command->mem_dst = mem_dst;
        cb_command->off_src = 0;
        cb_command->off_dst = 0;
        cb_command->cb = mem_dst->size;
        cb_command->dev_src = dev_src;
        cb_command->DisclosedToUser();
        EnqueueReadyQueue(cb_command, dev_dst);

        command->AddWaitEvent(cb_command->event);
        return false;
      } else {
    SNUCL_CHECK();
        CLCommand* rb_command = CommandFactory::instance()->NewCommand(CL_COMMAND_RECV_BUFFER);
        rb_command->mem_src = mem_dst;
        rb_command->mem_dst = mem_dst;
        rb_command->off_dst = 0;
        rb_command->cb = mem_dst->size;
        rb_command->dev_src = dev_src;
        rb_command->DisclosedToUser();
        EnqueueReadyQueue(rb_command, dev_dst);

        CLCommand* sb_command = CommandFactory::instance()->NewCommand(CL_COMMAND_SEND_BUFFER);
        sb_command->mem_src = mem_dst;
        sb_command->mem_dst = mem_dst;
        sb_command->off_src = 0;
        sb_command->cb = mem_dst->size;
        sb_command->dev_dst = dev_dst;
        sb_command->misc[0] = rb_command->event->id;
        sb_command->DisclosedToUser();
        EnqueueReadyQueue(sb_command, dev_src);

        command->AddWaitEvent(rb_command->event);
        return false;
      }
    }
  } else if (mem_dst->HasLatest(dev_dst)) {
    SNUCL_CHECK();
    CLDevice* dev_src = NearestDevice(mem_src, dev_dst);
    if (dev_src == NULL) {
      SNUCL_ERROR("DEV_SRC IS NULL MEM[%lu] DEVICE_LIST[%lu]", mem_src->id, mem_src->device_list.size());
      dev_src = dev_dst;
    }
    SNUCL_DEBUG("COPY BUFFER DEV[%lu] --> DEV[%lu]", dev_src->id, dev_dst->id);
    if (dev_src->node_id == dev_dst->node_id) {
      if (platform->cluster || dev_src == dev_dst) {
        SNUCL_CHECK();
        command->dev_src = dev_src;
        return true;
      }
      if (dev_src->type == CL_DEVICE_TYPE_CPU && dev_dst->type == CL_DEVICE_TYPE_GPU) {
        SNUCL_CHECK();
        command->SetType(CL_COMMAND_WRITE_BUFFER);
        void* m = mem_src->dev_specific[dev_src];
        command->ptr = (void*) ((size_t) m + command->off_src);
        return true;
      } else if (dev_src->type == CL_DEVICE_TYPE_GPU && dev_dst->type == CL_DEVICE_TYPE_CPU) {
        SNUCL_CHECK();
        command->SetType(CL_COMMAND_READ_BUFFER);
        void* m = mem_dst->dev_specific[dev_dst];
        command->ptr = (void*) ((size_t) m + command->off_dst);
        command->device = dev_src;
        return true;
      } else if (dev_src->type == CL_DEVICE_TYPE_GPU && dev_dst->type == CL_DEVICE_TYPE_GPU) {
        SNUCL_CHECK();
        if (!mem_src->space_host) mem_src->space_host = memalign(4096, mem_src->size);
        CLCommand* rb_command = CommandFactory::instance()->NewCommand(CL_COMMAND_READ_BUFFER);
        rb_command->mem_src = mem_src;
        rb_command->off_src = off_src;
        rb_command->cb = size;
        rb_command->ptr = (void*) ((size_t) mem_src->space_host + off_src);
        rb_command->DisclosedToUser();
        EnqueueReadyQueue(rb_command, dev_src);

        command->SetType(CL_COMMAND_WRITE_BUFFER);
        command->ptr = rb_command->ptr;
        command->AddWaitEvent(rb_command->event);
        return false;
      } else {
        SNUCL_ERROR("UNSUPPORT DEVICE TYPE SRC[%lx] DST[%lx]", dev_src->type, dev_dst->type);
        return true;
      }
    } else {
    SNUCL_CHECK();
      CLCommand* sb_command = CommandFactory::instance()->NewCommand(CL_COMMAND_SEND_BUFFER);
      sb_command->mem_src = mem_src;
      sb_command->mem_dst = mem_dst;
      sb_command->off_src = off_src;
      sb_command->cb = size;
      sb_command->dev_dst = dev_dst;
      sb_command->misc[0] = command->event->id;
      sb_command->DisclosedToUser();
      EnqueueReadyQueue(sb_command, dev_src);

      command->SetType(CL_COMMAND_RECV_BUFFER);
      command->dev_src = dev_src;
      return true;
    }
  } else {
    SNUCL_CHECK();
    if (off_dst == 0 && size == mem_dst->size) {
    SNUCL_CHECK();
      CLDevice* dev_src = NearestDevice(mem_src, dev_dst);
      if (dev_src == NULL) SNUCL_ERROR("DEV_SRC IS NULL MEM[%lu]", mem_src->id);
      if (dev_src->node_id == dev_dst->node_id) {
    SNUCL_CHECK();
        command->dev_src = dev_src;
        return true;
      } else {
    SNUCL_CHECK();
        CLCommand* sb_command = CommandFactory::instance()->NewCommand(CL_COMMAND_SEND_BUFFER);
        sb_command->mem_src = mem_src;
        sb_command->mem_dst = mem_dst;
        sb_command->off_src = off_src;
        sb_command->cb = size;
        sb_command->dev_dst = dev_dst;
        sb_command->misc[0] = command->event->id;
        sb_command->DisclosedToUser();
        EnqueueReadyQueue(sb_command, dev_src);

        command->SetType(CL_COMMAND_RECV_BUFFER);
        command->dev_src = dev_src;
        return true;
      }
    } else {
    SNUCL_CHECK();
      CLDevice* dev_src = NearestDevice(mem_dst, dev_dst);
      if (dev_src == NULL) {
    SNUCL_CHECK();
        dev_src = dev_dst;
      }
      if (dev_src->node_id == dev_dst->node_id) {
    SNUCL_CHECK();
        CLCommand* cb_command = CommandFactory::instance()->NewCommand(CL_COMMAND_COPY_BUFFER);
        cb_command->mem_src = mem_dst;
        cb_command->mem_dst = mem_dst;
        cb_command->off_src = 0;
        cb_command->off_dst = 0;
        cb_command->cb = mem_dst->size;
        cb_command->dev_src = dev_src;
        cb_command->DisclosedToUser();
        EnqueueReadyQueue(cb_command, dev_dst);

        command->AddWaitEvent(cb_command->event);
        return false;
      } else {
    SNUCL_CHECK();
        CLCommand* rb_command = CommandFactory::instance()->NewCommand(CL_COMMAND_RECV_BUFFER);
        rb_command->mem_src = mem_dst;
        rb_command->mem_dst = mem_dst;
        rb_command->off_dst = 0;
        rb_command->cb = mem_dst->size;
        rb_command->dev_src = dev_src;
        rb_command->DisclosedToUser();
        EnqueueReadyQueue(rb_command, dev_dst);

        CLCommand* sb_command = CommandFactory::instance()->NewCommand(CL_COMMAND_SEND_BUFFER);
        sb_command->mem_src = mem_dst;
        sb_command->mem_dst = mem_dst;
        sb_command->off_src = 0;
        sb_command->cb = mem_dst->size;
        sb_command->dev_dst = dev_dst;
        sb_command->misc[0] = rb_command->event->id;
        sb_command->DisclosedToUser();
        EnqueueReadyQueue(sb_command, dev_src);

        command->AddWaitEvent(rb_command->event);
        return false;
      }
    }
  }
}

bool CLScheduler::IssueBroadcastBuffer(CLCommand* command) {
  CLCommand* p_command = command->p_command;
  map<int, CLEvent*>* node_list = &p_command->node_list;
  CLDevice* cmq_dev = command->device;
  CLMem* src_buffer = command->mem_src;
  CLMem* dst_buffer = command->mem_dst;
  size_t src_offset = command->off_src;
  size_t dst_offset = command->off_dst;
  int node_id = cmq_dev->node_id;

  CLDevice* src_dev = NearestDevice(src_buffer, cmq_dev);
  if (src_dev == NULL || src_dev->node_id != node_id) {
    if (node_list->count(node_id) == 0) {
      (*node_list)[node_id] = command->event;
    } else {
      CLEvent* node_event = (*node_list)[node_id];
      command->mem_src = node_event->command->mem_dst;
      command->AddWaitEvent(node_event);
    }
  }
  command->SetType(CL_COMMAND_COPY_BUFFER);
  return false;
}

#if 1
bool CLScheduler::CheckRace(CLMem** mem_list, int num_mem, CLCommand* command) {
  return false;
}
#else
bool CLScheduler::CheckRace(CLMem** mem_list, int num_mem, CLCommand* command) {
  bool race = false;
  for (list<CLCommand*>::iterator it = issue_list.begin(); it != issue_list.end(); ++it) {
    CLCommand* cmd = *it;
    if (cmd->event->status == CL_COMPLETE) continue;
    if (cmd->type == CL_COMMAND_NDRANGE_KERNEL) {
      map<cl_uint, CLKernelArg*>* args = cmd->kernel_args;
      for (map<cl_uint, CLKernelArg*>::iterator it = args->begin(); it != args->end(); ++it) {
        bool match = false;
        CLKernelArg* arg = it->second;
        CLMem* mem = arg->mem;
        if (!mem) continue;
        if (mem->flags & CL_MEM_WRITE_ONLY || mem->flags & CL_MEM_READ_WRITE) {
          for (int i = 0; i < num_mem; ++i) {
            if (mem == mem_list[i]) {
              command->AddWaitEvent(cmd->event);
              SNUCL_DEBUG("RACE COMMAND[%lu] -> KERNEL LAUNCH COMMAND[%lu][%d]", command->id, cmd->id, cmd->event->status);
              race = true;
              match = true;
              break;
            }
          }
        }
        if (match) break;
      }
    } else if (cmd->type == CL_COMMAND_WRITE_BUFFER || cmd->type == CL_COMMAND_COPY_BUFFER) {
      CLMem* mem = cmd->mem_dst;
      for (int i = 0; i < num_mem; ++i) {
        if (mem == mem_list[i]) {
          command->AddWaitEvent(cmd->event);
          SNUCL_DEBUG("RACE COMMAND[%lu] -> BUFFER COMMAND[%lu][%d]", command->id, cmd->id, cmd->event->status);
          race = true;
          break;
        }
      }
    }
  }
  return race;
}
#endif

void CLScheduler::UpdateLatest(CLCommand* command, CLDevice* device) {
  CLEvent* event = command->event;
  switch (event->type) {
    case CL_COMMAND_NDRANGE_KERNEL:
      {
        map<cl_uint, CLKernelArg*>* args = command->kernel_args;
        for (map<cl_uint, CLKernelArg*>::iterator it2 = args->begin(); it2 != args->end(); ++it2) {
          CLKernelArg* arg = it2->second;
          CLMem* mem = arg->mem;
          if (mem && ((mem->flags & CL_MEM_WRITE_ONLY) || (mem->flags & CL_MEM_READ_WRITE))) {
            mem->ClearLatest(device);
          }
        }
      }
      break;
    case CL_COMMAND_NATIVE_KERNEL:
      {
        cl_uint num_mem_objects = (*command->kernel_args)[1]->size / sizeof(cl_mem);
        cl_mem* mem_list = (cl_mem*) (*command->kernel_args)[1]->value;
        for (int i = 0; i < num_mem_objects; i++) {
          CLMem* mem = mem_list[i]->c_obj;
          if (mem && ((mem->flags & CL_MEM_WRITE_ONLY) || (mem->flags & CL_MEM_READ_WRITE))) {
            mem->ClearLatest(device);
          }
        }
      }
      break;
    case CL_COMMAND_WRITE_BUFFER:
      {
        CLMem* mem = command->mem_dst;
        if (command->misc[0] == CL_COMMAND_NDRANGE_KERNEL) {
          mem->AddLatest(device);
        } else {
          mem->ClearLatest(device);

          if (mem->alloc_host) {
//            if (mem->space_host) free(mem->space_host);
            mem->alloc_host = false;
          }
        }
      }
      break;
    case CL_COMMAND_COPY_BUFFER:
      {
        CLMem* mem = command->mem_dst;
        if (command->misc[0] == CL_COMMAND_NDRANGE_KERNEL) {
          mem->AddLatest(device);
        } else {
          mem->ClearLatest(device);
        }
      }
      break;
    case CL_COMMAND_RECV_BUFFER:
      {
        CLMem* mem = command->mem_dst;
        if (command->misc[0] == CL_COMMAND_NDRANGE_KERNEL) {
          mem->AddLatest(device);
        } else {
          mem->ClearLatest(device);
        }
      }
      break;
    case CL_COMMAND_WRITE_IMAGE:
      {
        CLMem* mem = command->mem_dst;
        if (command->misc[0] == CL_COMMAND_NDRANGE_KERNEL) {
          mem->AddLatest(device);
        } else {
          mem->ClearLatest(device);
        }
      }
      break;
    case CL_COMMAND_COPY_IMAGE:
      {
        CLMem* mem = command->mem_dst;
        if (command->misc[0] == CL_COMMAND_NDRANGE_KERNEL) {
          mem->AddLatest(device);
        } else {
          mem->ClearLatest(device);
        }
      }
      break;
    case CL_COMMAND_BROADCAST_BUFFER:
    case CL_COMMAND_ALLTOALL_BUFFER:
      {
        CLMem* mem = command->mem_dst;
        mem->ClearLatest(device);
      }
      break;
  }
}

CLDevice* CLScheduler::NearestDevice(CLMem* mem, CLDevice* device) {
  CLDevice* ret = NULL;
  if (mem->EmptyLatest()) return NULL;

  pthread_mutex_lock(&mem->mutex_latest);
  vector<CLDevice*>* device_list = &mem->device_list;
  for (vector<CLDevice*>::iterator it = device_list->begin(); it != device_list->end(); ++it) {
    CLDevice* dev = *it;
    if (device->node_id == dev->node_id) {
      ret = dev;
      if (dev->type == CL_DEVICE_TYPE_CPU) break;
    }
  }
  if (ret) {
    pthread_mutex_unlock(&mem->mutex_latest);
    return ret;
  }

  for (vector<CLDevice*>::iterator it = device_list->begin(); it != device_list->end(); ++it) {
    CLDevice* dev = *it;
    ret = dev;
    if (dev->type == CL_DEVICE_TYPE_CPU) break;
  }

  if (ret == NULL) {
    SNUCL_ERROR("WHY??? MEM[%lu]", mem->id);
  }
  pthread_mutex_unlock(&mem->mutex_latest);
  return ret;
}

CLDevice* CLScheduler::PreferCPUDevice(CLMem* mem) {
  if (mem->EmptyLatest()) return NULL;

  pthread_mutex_lock(&mem->mutex_latest);
  vector<CLDevice*>* device_list = &mem->device_list;
  for (vector<CLDevice*>::iterator it = device_list->begin(); it != device_list->end(); ++it) {
    CLDevice* dev = *it;
    if (dev->type == CL_DEVICE_TYPE_CPU) {
      pthread_mutex_unlock(&mem->mutex_latest);
      return dev;
    }
  }
  CLDevice* ret = device_list->front();
  pthread_mutex_unlock(&mem->mutex_latest);
  return ret;
}

CLIssuer::CLIssuer(CLPlatform* platform) {
  this->platform = platform;
  completion_queue = new LockFreeQueue(MAX_NUM_COMMANDS_IN_READY_QUEUE);
  InitDevices();
  cleaner = new CLIssuerCleaner(this);
}

CLIssuer::~CLIssuer() {
  running = false;
  pthread_cancel(thread);
  for (map<int, int>::iterator it = map_size.begin(); it != map_size.end(); ++it) {
    SNUCL_INFO("RQ SIZE[%d : %d]", it->first, it->second);
  }
  delete[] devices;
  delete cleaner;
}

void CLIssuer::Start() {
  pthread_create(&thread, NULL, &CLIssuer::ThreadFunc, this);
  cleaner->Start();
}

void CLIssuer::InitDevices() {
  vector<CLDevice*>* devs = &platform->devices;
  num_devices = devs->size();
  devices = new CLDevice*[num_devices];
  for (int i = 0; i < num_devices; i++) {
    devices[i] = (*devs)[i];
    int node_id = devices[i]->node_id;
    if (ready_queues.count(node_id) == 0) {
      ready_queues[node_id] = new LockFreeQueueMS(MAX_NUM_COMMANDS_IN_READY_QUEUE);
    }
  }
}

void CLIssuer::EnqueueCompletionQueue(CLCommand* command) {
  completion_list.push_back(command);
  completion_queue->Enqueue(command);
}

bool CLIssuer::EnqueueRunningCommand(CLCommand* command) {
    running_commands.push_back(command);
    /*
  if (command->event->user) {
    running_commands.push_back(command);
  } else {
    CMD_D();
    CommandFactory::instance()->FreeCommand(command);
  }
  */
  return true;
}

bool CLIssuer::EnqueueReadyQueue(CLCommand* command, CLDevice* device) {
  int node_id = device->node_id;
  LockFreeQueue* rq = (LockFreeQueue*) ready_queues[node_id];
  if (rq == NULL) SNUCL_ERROR("RQ IS NULL CMD[%lu] NODE[%d]", command->id, node_id);
  bool ret = rq->Enqueue(command);
  if (!ret) SNUCL_ERROR("CMD[%lu]", command->id);
  return ret;
}

void* CLIssuer::ThreadFunc(void* argp) {
  ((CLIssuer*) argp)->Run();
  return NULL;
}

void CLIssuer::Run() {
  running = true;

#if 0
  int num_rq = ready_queues.size();
  LockFreeQueue** rq = new LockFreeQueue*[num_rq];

  int idx_rq = 0;
  for (map<int, void*>::iterator it = ready_queues.begin(); it != ready_queues.end(); ++it) {
    rq[idx_rq++] = (LockFreeQueue*) it->second;
  }
#else
  int num_rq = platform->num_schedulers;
  LockFreeQueue** rq = new LockFreeQueue*[num_rq];
  for (int i = 0; i < num_rq; i++) {
    rq[i] = platform->schedulers[i]->ready_queue;
  }
#endif

  while (running) {
    //Execute Commands
    for (int i = 0; i < num_rq; i++) {
      int size = (int) rq[i]->Size();
      if (size > 0) {
        if (map_size.count(size)) {
          map_size[size]++;
        } else {
          map_size[size] = 1;
        }
      }
      CLCommand* command = NULL;
      while (rq[i]->Dequeue(&command)) {
        SNUCL_DEBUG("ISSUER CMD[%lu] TYPE[%x] DEV[%lu] SIZE[%d] SCHEDULER[%d]", command->id, command->type, command->device->id, rq[i]->Size(), i);
        issue_list[command->id] = command;
        command->Execute();
      }
    }

    // Check Completion Queue
    for (list<CLCommand*>::iterator it = running_commands.begin(); it != running_commands.end(); ++it) {
      CLCommand* command = *it;
      CLEvent* event = command->event;
      if (!command->device->IsComplete(event)) continue;
      event->Complete();
      EnqueueCompletionQueue(command);
      running_commands.erase(it--);
      //SNUCL_DEBUG("COMPLETE EVENT[%lu] TYPE[%x] COMMAND[%lu]", event->id, event->type, command->id);
    }

#if 0
    for (vector<CLCommand*>::iterator it = completion_list.begin(); it != completion_list.end(); ++it) {
      CLCommand* command = *it;
      CLCommandQueue* command_queue = command->command_queue;
      if (command_queue) {
        command_queue->Remove(command);
        issue_list.erase(command->id);
      }
      completion_list.erase(it--);

      if (command->type == CL_COMMAND_MARKER) continue;
      if (command->event->ref_cnt <= 0 && !command->event->user) {
        SNUCL_DEBUG("DELETE EVENT[%lu] REF_CNT[%d]", command->event->id, command->event->ref_cnt);
        delete command->event;
      }
      SNUCL_DEBUG("DELETE COMMAND[%lu]", command->id);
      delete command;
    }
#endif

  }

  delete[] rq;
}

CLIssuerCleaner::CLIssuerCleaner(CLIssuer* issuer) {
  this->issuer = issuer;
}

CLIssuerCleaner::~CLIssuerCleaner() {
  running = false;
  pthread_cancel(thread);
}

void CLIssuerCleaner::Start() {
  pthread_create(&thread, NULL, &CLIssuerCleaner::ThreadFunc, this);
}

void* CLIssuerCleaner::ThreadFunc(void* argp) {
  ((CLIssuerCleaner*) argp)->Run();
  return NULL;
}

void CLIssuerCleaner::Run() {
  running = true;

  CLCommand* command = NULL;
  LockFreeQueue* q = issuer->completion_queue;
  while (running) {
    while (q->Dequeue(&command)) {
      CMD_D();
      CommandFactory::instance()->FreeCommand(command);
    }
  }

}

CLWorkGroupAssignment::CLWorkGroupAssignment(CLCommand* command) {
  this->command = command;
}

CLWorkGroupAssignment::~CLWorkGroupAssignment() {
}

CLProfiler::CLProfiler(bool debug) {
  this->debug = debug;
  cnt_cmd_k = 0;
  cnt_cmd_m = 0;
  cnt_cmd_s = 0;

  mem_size = 0;
  mem_size_max = 0;
}

CLProfiler::~CLProfiler() {
  if (!debug || cnt_cmd_k + cnt_cmd_m + cnt_cmd_s == 0) return;
  PrintSummary();
}

void CLProfiler::CreateMem(CLMem* mem) {
  mem_size += mem->size;
  if (mem_size_max < mem_size) mem_size_max = mem_size;
}

void CLProfiler::ReleaseMem(CLMem* mem) {
  mem_size -= mem->size;
}

void CLProfiler::EnqueueCommand(CLCommand* command) {
  cl_command_type type = command->type;
  if (type == CL_COMMAND_NDRANGE_KERNEL || type == CL_COMMAND_TASK) {
    ++cnt_cmd_k;
    ++command->device->cnt_cmd_k;
  } else if (type == CL_COMMAND_READ_BUFFER || type == CL_COMMAND_WRITE_BUFFER || type == CL_COMMAND_COPY_BUFFER) {
    ++cnt_cmd_m;
    ++command->device->cnt_cmd_m;
  } else if (type == CL_COMMAND_WAIT_FOR_EVENTS || type == CL_COMMAND_BARRIER || type == CL_COMMAND_MARKER) {
    ++cnt_cmd_s;
    ++command->device->cnt_cmd_s;
  }
}

void CLProfiler::CompleteCommand(CLCommand* command) {
  return;
  CLDevice* device = command->device;
  if (device == NULL) SNUCL_ERROR("DEVICE IS NULL COMMAND[%lu] TYPE[%x]", command->id, command->type);
  CLEvent* event = command->event;
  cl_command_type type = command->type;
  if (event->profiling_info[0] == 0) return;

  pthread_mutex_lock(&device->mutex_p);
  device->time_cmd_submit += event->profiling_info[1] - event->profiling_info[0];
  device->time_cmd_start += event->profiling_info[2] - event->profiling_info[1];
  device->time_cmd_end += event->profiling_info[3] - event->profiling_info[2];

  device->time_cmd_issue += event->profiling_info[4] - event->profiling_info[2];

  if (type == CL_COMMAND_NDRANGE_KERNEL || type == CL_COMMAND_TASK) {
    device->time_cmd_k += event->profiling_info[3] - event->profiling_info[1];
  } else if (type == CL_COMMAND_READ_BUFFER || type == CL_COMMAND_WRITE_BUFFER || type == CL_COMMAND_COPY_BUFFER || type == CL_COMMAND_RECV_BUFFER || type == CL_COMMAND_SEND_BUFFER) {
    device->time_cmd_m += event->profiling_info[3] - event->profiling_info[1];
  } else if (type == CL_COMMAND_WAIT_FOR_EVENTS || type == CL_COMMAND_BARRIER || type == CL_COMMAND_MARKER) {
    device->time_cmd_s += event->profiling_info[3] - event->profiling_info[1];
  } else {
    SNUCL_ERROR("COMMAND[%lu] TYPE[%x]", command->id, command->type);
  }

  if (device->time_cmd_queued_first == 0 || device->time_cmd_queued_first > event->profiling_info[0]) device->time_cmd_queued_first = event->profiling_info[0];
  if (device->time_cmd_queued_last < event->profiling_info[0]) device->time_cmd_queued_last = event->profiling_info[0];

  if (device->time_cmd_submit_first == 0 || device->time_cmd_submit_first > event->profiling_info[1]) device->time_cmd_submit_first = event->profiling_info[1];
  if (device->time_cmd_submit_last < event->profiling_info[1]) device->time_cmd_submit_last = event->profiling_info[1];

  if (device->time_cmd_start_first == 0 || device->time_cmd_start_first > event->profiling_info[2]) device->time_cmd_start_first = event->profiling_info[2];
  if (device->time_cmd_start_last < event->profiling_info[2]) device->time_cmd_start_last = event->profiling_info[2];

  if (device->time_cmd_end_first == 0 || device->time_cmd_end_first > event->profiling_info[3]) device->time_cmd_end_first = event->profiling_info[3];
  if (device->time_cmd_end_last < event->profiling_info[3]) device->time_cmd_end_last = event->profiling_info[3];

  pthread_mutex_unlock(&device->mutex_p);
}

void CLProfiler::PrintSummary() {
  SNUCL_INFO("ENQUEUED COMMAND[%lu] KERNEL[%lu] MEMORY[%lu] SYNC[%lu]", cnt_cmd_k + cnt_cmd_m + cnt_cmd_s, cnt_cmd_k, cnt_cmd_m, cnt_cmd_s); 
  SNUCL_INFO("MAX MEMORY USAGE [%.2lf]GB [%.2lf]MB [%.2lf]KB [%lu]B", (double) mem_size_max / 1024 / 1024 / 1024, (double) mem_size_max / 1024 / 1024, (double) mem_size_max / 1024, mem_size_max);
}

LockFreeQueue::LockFreeQueue(unsigned long size) {
  this->size = size;
  idx_r = 0;
  idx_w = 0;
  elements = (volatile CLCommand**) new CLCommand*[size];
}

LockFreeQueue::~LockFreeQueue() {
  delete[] elements;
}

bool LockFreeQueue::Enqueue(CLCommand* element) {
  unsigned long next_idx_w = (idx_w + 1) % size;
  if (next_idx_w == idx_r) return false;
  elements[idx_w] = element;
  __sync_synchronize();
  idx_w = next_idx_w;
  return true;
}

bool LockFreeQueue::Dequeue(CLCommand** element) {
  if (idx_r == idx_w) return false;
  unsigned long next_idx_r = (idx_r + 1) % size;
  *element = (CLCommand*) elements[idx_r];
  idx_r = next_idx_r;
  return true;
}

bool LockFreeQueue::Peek(CLCommand** element) {
  if (idx_r == idx_w) return false;
  *element = (CLCommand*) elements[idx_r];
  return true;
}

unsigned long LockFreeQueue::Size() {
  if (idx_w >= idx_r) return idx_w - idx_r;
  return size - idx_r + idx_w;
}

LockFreeQueueMS::LockFreeQueueMS(unsigned long size) : LockFreeQueue(size) {
  idx_w_cas = 0;
}

LockFreeQueueMS::~LockFreeQueueMS() {
}

bool LockFreeQueueMS::Enqueue(CLCommand* element) {
  while (true) {
    unsigned long prev_idx_w = idx_w_cas;
    unsigned long next_idx_w = (prev_idx_w + 1) % size;
    if (next_idx_w == idx_r) return false;
    if (__sync_bool_compare_and_swap(&idx_w_cas, prev_idx_w, next_idx_w)) {
      elements[prev_idx_w] = element;
      while (!__sync_bool_compare_and_swap(&idx_w, prev_idx_w, next_idx_w)) {}
      break;
    }
  }
  return true;
}

CommandFactory::CommandFactory() {
#if 0
  commands = new CLCommand*[NUM_COMMANDS_IN_POOL];
  for (int i = 0; i < NUM_COMMANDS_IN_POOL; i++) {
    commands[i] = new CLCommand(CL_COMMAND_NDRANGE_KERNEL);
  }
  current = 0;
  pthread_mutex_init(&mutex_commands, NULL);
#endif
}

CommandFactory::~CommandFactory() {
  pthread_mutex_destroy(&mutex_commands);
}

void CommandFactory::FreeCommand(CLCommand* command) {
#if 0
  if (command->free) {
    delete command;
  } else {
  pthread_mutex_lock(&mutex_commands);
  command->free = true;
  pthread_mutex_unlock(&mutex_commands);
  }
#endif
}

void CommandFactory::FreeCommandEvent(CLCommand* command) {
  pthread_mutex_lock(&mutex_commands);
  command->free = true;
  delete command->event;
  pthread_mutex_unlock(&mutex_commands);
}

CLCommand* CommandFactory::NewCommand(cl_command_type type) {
  CLCommand* command = new CLCommand(type);
  command->free = true;
  return command;
  /*
  pthread_mutex_lock(&mutex_commands);
  int start = current;
  while (true) {
    for (int i = start; i < NUM_COMMANDS_IN_POOL; i++) {
      CLCommand* command = commands[i];
      SNUCL_INFO("NC %d, %d", i, command->free);
      if (command->free) {
        command->type = type;
        command->event = new CLEvent(command);
        command->misc[0] = 0;
        command->sub_command = NULL;
        command->free = false;
        command->wait_events.clear();
        current = i == NUM_COMMANDS_IN_POOL - 1 ? 0 : i + 1;
        pthread_mutex_unlock(&mutex_commands);
        return command;
      }
    }
    start = 0;
  }
  pthread_mutex_unlock(&mutex_commands);
  */
}

CLCommand* CommandFactory::NewCommand(CLCommandQueue* command_queue, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_command_type type) {
#if 1
  return new CLCommand(command_queue, num_events_in_wait_list, event_wait_list, type);
#else
  pthread_mutex_lock(&mutex_commands);
  int start = current;
  while (true) {
    for (int i = start; i < NUM_COMMANDS_IN_POOL; i++) {
      CLCommand* command = commands[i];
      if (command->free) {
        command->id = NewID();
        command->command_queue = command_queue;
        command->type = type;
        command->device = command_queue->device;
        delete command->event;
        command->event = new CLEvent(NULL, type, NULL, command);

        command->wait_events.clear();
        for (cl_uint j = 0; j < num_events_in_wait_list; ++j) {
          command->AddWaitEvent(event_wait_list[j]->c_obj);
        }
        command->free = false;
        pthread_mutex_unlock(&mutex_commands);
        current = i == NUM_COMMANDS_IN_POOL - 1 ? 0 : i + 1;
        return command;
      }
    }
    start = 0;
  }
  pthread_mutex_unlock(&mutex_commands);
#endif
}


