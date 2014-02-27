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

#ifndef __CLOBJECT_H
#define __CLOBJECT_H

#include <CL/cl.h>
#include <CL/cl_ext_collective.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <deque>
#include <list>
#include <map>
#include <vector>
#include <unistd.h>
#include <string>

#define CL_COMMAND_BUILD_PROGRAM                    0x1210
#define CL_COMMAND_SEND_BUFFER                      0x1211
#define CL_COMMAND_RECV_BUFFER                      0x1212
#define CL_COMMAND_FREE_BUFFER                      0x1213
#define CL_COMMAND_WAIT_FOR_EVENTS                  0x1214
#define CL_COMMAND_NOP                              0x1216
#define CL_COMMAND_COMPILE_PROGRAM                  0x1217
#define CL_COMMAND_LINK_PROGRAM                     0x1218

#define MAX_NUM_DEVICES_PER_SCHEDULER               1024
#define USE_LARGE_CQ
#ifdef USE_LARGE_CQ   //should be defined by user application when it is needed
#define MAX_NUM_CQ_PER_DEVICE                       16384
#else
#define MAX_NUM_CQ_PER_DEVICE                       256
#endif
#define MAX_NUM_COMMANDS_IN_READY_QUEUE             4096
#define NUM_COMMANDS_IN_POOL                        16384
#define USE_LARGE_MESSAGE_COMMAND
#ifdef USE_LARGE_MESSAGE_COMMAND
#define MESSAGE_COMMAND_SIZE                        5120
#else
#define MESSAGE_COMMAND_SIZE                        512
#endif
#define ALWAYS_NOTIFY_COMPLETE

#define SNUCL_BUILD_DIR         "snucl_kernels"
#define SNUCL_BUILD_HEADER_DIR  "snucl_kernels/headers"
#define SNUCL_CPU_BUILD_DIR     "snucl_kernels/cpu"

using namespace std;

class CLObject;
class CLPlatform;
class CLDevice;
class CLContext;
class CLCommandQueue;
class CLCommand;
class CLMem;
class CLSampler;
class CLProgram;
class CLBinary;
class CLKernel;
class CLKernelArg;
class CLEvent;

class CLDeviceWorker;
class CLScheduler;
class CLIssuer;
class CLIssuerCleaner;
class CLWorkGroupAssignment;
class CLProfiler;

class LockFreeQueue;
class LockFreeQueueMS;

class ProgramCallback;
class EventCallback;
class MemObjectDestructorCallback;

struct _cl_platform_id {
  CLPlatform* c_obj;
};

struct _cl_device_id {
  CLDevice* c_obj;
};

struct _cl_context {
  CLContext* c_obj;
};

struct _cl_command_queue {
  CLCommandQueue* c_obj;
};

struct _cl_mem {
  CLMem* c_obj; 
};

struct _cl_sampler {
  CLSampler* c_obj; 
};

struct _cl_program {
  CLProgram* c_obj;
};

struct _cl_kernel {
  CLKernel* c_obj;
};

struct _cl_event {
  CLEvent* c_obj;
};

class CLObject {
public:
  CLObject();

  virtual int Retain();
  virtual int Release();

  unsigned long       id;
  cl_uint             ref_cnt;
};

class CLPlatform : public CLObject {
public:
  CLPlatform();
  ~CLPlatform();

  void InitDevices();
  void InitSchedulers(bool running);

  _cl_platform_id     st_obj;

  const char*         profile;
  const char*         version;
  const char*         name;
  const char*         vendor;
  const char*         extensions;

  bool                cluster;
  bool                host;

  int                 num_schedulers;

  vector<CLDevice*>   devices;
  sem_t               sem_dev;
  CLScheduler**       schedulers;
  CLIssuer*           issuer;
  CLProfiler*         profiler;

  cl_ulong            start_timestamp;
};

class CLDevice : public CLObject {
public:
  CLDevice(cl_device_type type);
  virtual ~CLDevice();
  virtual void Init() = 0;
  virtual void InitInfo() = 0;
	virtual cl_int DivideDevices(const cl_device_partition_property* properties, cl_uint num_devices, cl_device_id* out_devices, cl_uint* num_devices_ret) { return CL_SUCCESS; }

  void Prepare(CLCommand* command);
  virtual void PrepareBuildProgram(CLCommand* command) {}
  virtual void PrepareLaunchKernel(CLCommand* command) {}
  virtual void PrepareNativeKernel(CLCommand* command) {}
  virtual void PrepareReadBuffer(CLCommand* command) {}
  virtual void PrepareWriteBuffer(CLCommand* command) {}
  virtual void PrepareCopyBuffer(CLCommand* command) {}
  virtual void PrepareSendBuffer(CLCommand* command) {}
  virtual void PrepareRecvBuffer(CLCommand* command) {}
  virtual void PrepareReadBufferRect(CLCommand* command) {}
  virtual void PrepareWriteBufferRect(CLCommand* command) {}
  virtual void PrepareCopyBufferRect(CLCommand* command) {}
  virtual void PrepareFreeBuffer(CLCommand* command) {}
  virtual void PrepareReadImage(CLCommand* command) {}
  virtual void PrepareWriteImage(CLCommand* command) {}
  virtual void PrepareCopyImage(CLCommand* command) {}
  virtual void PrepareCopyImageToBuffer(CLCommand* command) {}
  virtual void PrepareCopyBufferToImage(CLCommand* command) {}
  virtual void PrepareMarker(CLCommand* command) {}
  
  void Execute(CLCommand* command);
  virtual void BuildProgram(CLCommand* command) = 0;
  virtual void CompileProgram(CLCommand* command) {}
  virtual void LinkProgram(CLCommand* command) {}
  virtual void LaunchKernel(CLCommand* command) = 0;
  virtual void NativeKernel(CLCommand* command) {}
  virtual void ReadBuffer(CLCommand* command) = 0;
  virtual void WriteBuffer(CLCommand* command) = 0;
  virtual void CopyBuffer(CLCommand* command) = 0;
  virtual void SendBuffer(CLCommand* command) {}
  virtual void RecvBuffer(CLCommand* command) {}
  virtual void BcastBuffer(CLCommand* command) {}
  virtual void AlltoallBuffer(CLCommand* command) {}
  virtual void ReadBufferRect(CLCommand* command) {}
  virtual void WriteBufferRect(CLCommand* command) {}
  virtual void ReadImage(CLCommand* command) {}
  virtual void WriteImage(CLCommand* command) {}
  virtual void CopyImage(CLCommand* command) {}
  virtual void CopyImageToBuffer(CLCommand* command) {}
  virtual void CopyBufferToImage(CLCommand* command) {}
  virtual void CopyBufferRect(CLCommand* command) {}
  virtual void FillBuffer(CLCommand* command) {}
  virtual void FillImage(CLCommand* command) {}
  virtual void MapBuffer(CLCommand* command) {}
  virtual void MapImage(CLCommand* command) {}
  virtual void UnmapMemObject(CLCommand* command) {}
  virtual void MigrateMemObjects(CLCommand* command) {}
  virtual void Barrier(CLCommand* command);
  virtual void Marker(CLCommand* command);
  virtual void WaitCommand(CLCommand* command) {}

  bool IsAllocBuffer(CLMem* mem);
  void CheckBuffer(CLMem* mem);
  virtual void AllocBuffer(CLMem* mem) = 0;
  virtual void FreeBuffer(CLCommand* command) = 0;
	virtual void FreeProgramInfo(CLProgram* program) {}

  void EnqueueBuildProgram(CLProgram* program, const char* options);
  void EnqueueCompileProgram(CLProgram* program, const char* options);
  void EnqueueLinkProgram(CLProgram* program, const char* options);

  void AddCommandQueue(CLCommandQueue* command_queue);

  bool EnqueueReadyQueue(CLCommand* command);

  virtual bool IsComplete(CLEvent* event) { return true; }

	void AddSubDevicesToScheduler();

  virtual cl_int GetSupportedImageFormats(cl_mem_flags flags,
                                 cl_mem_object_type image_type,
                                 cl_uint num_entries,
                                 cl_image_format *image_formats,
                                 cl_uint * num_image_formats) { return CL_SUCCESS; }

  void CopyProgram(CLProgram* program, CLDevice* dev);
  void AddProgram(CLProgram* program);
  bool ContainsProgram(CLProgram* program);
  virtual void PrintStatistics();

  _cl_device_id                 st_obj;

	CLPlatform*                   platform;
  CLScheduler*                  scheduler;
	int                           devIdxInScheduler;
  CLIssuer*                     issuer;
  CLDeviceWorker*               worker;
  CLCommandQueue**              command_queues;
  bool*                         command_queue_retained;
  int                           num_command_queues;
  pthread_mutex_t               mutex_q;
  LockFreeQueue*                ready_queue;
  sem_t                         sem_rq;
	vector<CLProgram*>            programs;

	unsigned int                  numSubDevices;

	vector<CLDevice*>             subDevices;

  char                          node_name[64];
  int                           node_id;
  int                           dev_id;

  //Info
  cl_device_type                type;
  cl_uint                       vendor_id;
  cl_uint                       max_compute_units;
  cl_uint                       max_work_item_dimensions;
  size_t                        max_work_item_sizes[3];
  size_t                        max_work_group_size;

  cl_uint                       preferred_vector_width_char;
  cl_uint                       preferred_vector_width_short;
  cl_uint                       preferred_vector_width_int;
  cl_uint                       preferred_vector_width_long;
  cl_uint                       preferred_vector_width_float;
  cl_uint                       preferred_vector_width_double;
  cl_uint                       preferred_vector_width_half;
  cl_uint                       native_vector_width_char;
  cl_uint                       native_vector_width_short;
  cl_uint                       native_vector_width_int;
  cl_uint                       native_vector_width_long;
  cl_uint                       native_vector_width_float;
  cl_uint                       native_vector_width_double;
  cl_uint                       native_vector_width_half;

  cl_uint                       max_clock_frequency;
  cl_uint                       address_bits;
  cl_ulong                      max_mem_alloc_size;
  cl_bool                       image_support;


	cl_uint                       max_read_image_args;
	cl_uint                       max_write_image_args;
	size_t                        image2d_max_width;
	size_t                        image2d_max_height;
	size_t                        image3d_max_width;
	size_t                        image3d_max_height;
	size_t                        image3d_max_depth;
	size_t                        image_max_buffer_size;
	size_t                        image_max_array_size;
	cl_uint                       max_samplers;
  size_t                        max_parameter_size;
  cl_uint                       mem_base_addr_align;
  cl_uint                       min_data_type_align_size;
  cl_device_fp_config           single_fp_config;
  cl_device_fp_config           double_fp_config;
  cl_device_mem_cache_type      global_mem_cache_type;
  cl_uint                       global_mem_cacheline_size;
  cl_ulong                      global_mem_cache_size;
  cl_ulong                      global_mem_size;
  cl_ulong                      max_constant_buffer_size;
  cl_uint                       max_constant_args;
  cl_device_local_mem_type      local_mem_type;
  cl_ulong                      local_mem_size;
  cl_bool                       error_correction_support;
  cl_bool                       host_unified_memory;
  size_t                        profiling_timer_resolution;
  cl_bool                       endian_little;
  cl_bool                       available;
  cl_bool                       compiler_available;
  cl_bool                       linker_available;
  cl_device_exec_capabilities   execution_capabilities;
  cl_command_queue_properties   queue_properties;
	char                          built_in_kernels[64];
  char                          name[64];
  char                          vendor[64];
  char                          driver_version[64];
  char                          profile[32];
  char                          device_version[32];
  char                          openclc_version[32];
  char                          device_extensions[1024];
  size_t                        printf_buffer_size;
  cl_bool                       preferred_interop_user_sync;
	cl_device_id                  parent_device;
	cl_uint                       partition_max_sub_devices;
	cl_device_partition_property  partition_properties[3];
	size_t                        num_partition_properties;
	cl_device_affinity_domain     affinity_domain;
	cl_device_partition_property* partition_type;
	uint                          partition_type_len;

  pthread_mutex_t               mutex_p;

  cl_ulong                      time_cmd_k;
  cl_ulong                      time_cmd_m;
  cl_ulong                      time_cmd_s;

  cl_ulong                      time_cmd_submit;
  cl_ulong                      time_cmd_start;
  cl_ulong                      time_cmd_end;

  cl_ulong                      time_cmd_issue;

  cl_ulong                      time_cmd_queued_first;
  cl_ulong                      time_cmd_queued_last;
  cl_ulong                      time_cmd_submit_first;
  cl_ulong                      time_cmd_submit_last;
  cl_ulong                      time_cmd_start_first;
  cl_ulong                      time_cmd_start_last;
  cl_ulong                      time_cmd_end_first;
  cl_ulong                      time_cmd_end_last;

  size_t                        cnt_cmd_k; 
  size_t                        cnt_cmd_m; 
  size_t                        cnt_cmd_s; 
};

class __sns_CLDevice {
	vector<CLDevice> devInst;
	int numDev;
}

class CLContext : public CLObject {
public:
  CLContext(cl_uint num_devices, const cl_device_id* devices, bool running_scheduler = true);
  ~CLContext();
	cl_int SetDevices(vector<CLDevice*> devices, cl_device_type type);

	cl_int SetProperties(const cl_context_properties* properties);
  void AddMemory(CLMem* mem);
  void AddSampler(CLSampler* sampler);

  _cl_context         st_obj;

  vector<CLDevice*>   devices;
  vector<CLMem*>      mems;
  vector<CLSampler*>  samplers;
  size_t              total_mem_size;
  pthread_mutex_t     mutex_m;
  pthread_mutex_t     mutex_s;

	size_t                   num_devices;
	cl_context_properties*   properties;
	size_t                   num_properties;

  __sns_CLDevice sns_device;
};

class CLCommandQueue : public CLObject {
public:
  CLCommandQueue(CLContext *context, CLDevice* device, cl_command_queue_properties properties);
  ~CLCommandQueue();

  bool IsInOrder();
  bool IsProfiled();

  void Enqueue(CLCommand* command);
  void Dequeue(CLCommand* command);
  void Remove(CLCommand* command);
  cl_event* GetAllEvents(cl_uint *num_events);

  _cl_command_queue           st_obj; 

  cl_command_queue_properties properties;
  CLContext*                  context;
  CLDevice*                   device;
	int                         queueIdx;
  list<CLCommand*>            commands;
  LockFreeQueue*              commands_io;
  list<CLCommand*>            commands_dequeued;
  pthread_mutex_t             mutex_q;

  CLEvent*                    last_event;
};

class __sns_CLCommandQueue : public CLObject {
public:
  vector<CLCommandQueue> cqInst;
  int numDev;
};

class CLCommand : public CLObject {
public:
  CLCommand(cl_command_type type);
  CLCommand(CLCommandQueue* command_queue, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_command_type type);
  ~CLCommand();

  void SetType(cl_command_type type);
  void AddWaitEvent(CLEvent* event);
  bool Executable();
  bool Issuable();
  void Execute();
  void SetKernelArgs();
  cl_event DisclosedToUser();

  CLCommandQueue*             command_queue;
  CLDevice*                   device;
  CLDevice*                   dev_src;
  CLDevice*                   dev_dst;
  cl_command_type             type;
  CLEvent*                    event;
  vector<CLEvent*>            wait_events;

  CLMem*                      mem_src;
  CLMem*                      mem_dst;
  size_t                      off_src;
  size_t                      off_dst;
  size_t                      cb;
  void*                       ptr;

  CLKernel*                   kernel;
  cl_uint                     work_dim;
  size_t                      gwo[3];
  size_t                      gws[3];
  size_t                      lws[3];
  size_t                      nwg[3];
  map<cl_uint, CLKernelArg*>* kernel_args;
  bool                        mems_allocated;

  CLProgram*                  program;

  CLCommand*                  sub_command;
  unsigned long               misc[2];

  CLCommand*                  p_command;
  list<CLCommand*>            children;
  map<int, CLEvent*>          node_list;

  char                        msg[MESSAGE_COMMAND_SIZE];
  int                         msg_size;

  //Native Kernel
  void                        (*user_func)(void *);

  bool                        free;

  // Read/Write/CopyBufferRect
  CLMem*                      buffer;
  size_t                      src_origin[3];
  size_t                      dst_origin[3];
  size_t                      region[3];
  size_t                      src_row_pitch;
  size_t                      src_slice_pitch;
  size_t                      dst_row_pitch;
  size_t                      dst_slice_pitch;

  // FillBuffer
  void *                      pattern;
  size_t                      pattern_size;
  size_t                      offset;                     
  size_t                      size;

  // MapBuffer
  cl_map_flags                map_flags;

  // MigrateMemObjects
  cl_uint                     num_mem_objects;
  cl_mem*                     mem_objects;
  cl_mem_migration_flags      flags;
};

/* It has a subset of carbon copy of class CLMem variables. */
class CLImage {
  public:
    void *                ptr;
    size_t                size;
    cl_image_format       image_format;
    cl_image_desc         image_desc;
    size_t                image_row_pitch;
    size_t                image_slice_pitch;
    size_t                image_elem_size;
    size_t                image_channels;
};

class CLMappedInfo {
  public:
    size_t                origin[3];
    size_t                region[3];
    size_t                row_pitch;
    size_t                slice_pitch;
    size_t                offset;
    size_t                size;
};

class CLMem : public CLObject {
public:
  CLMem(CLContext* context, cl_mem_flags flags, size_t size, void* host_ptr, bool is_sub);
  CLMem(CLContext* context, cl_mem_flags flags, const cl_image_format* image_format, size_t image_width, size_t image_height, size_t image_row_pitch, void* host_ptr);
  ~CLMem();
	void SetCommand(CLCommand* command);

  void Init(CLContext* context, cl_mem_flags flags, size_t size, cl_mem_object_type type, void* host_ptr, bool is_sub);

  int Release();

  CLDevice* FrontLatest();
  bool HasLatest(CLDevice* device);
  bool EmptyLatest();
  void AddLatest(CLDevice* device);
  void ClearLatest(CLDevice* device);

  _cl_mem               st_obj;

	vector<CLCommand*>    commands;
  CLContext*            context;
  cl_mem_flags          flags;
  size_t                size;
  cl_mem_object_type    type;
  cl_buffer_create_type create_type;

  vector<CLDevice*>     device_list;
  pthread_mutex_t       mutex_latest;
  bool                  alloc_host;
  void*                 space_host;
  bool                  use_host;
  bool                  space_host_valid;

  map<CLDevice*, void*> dev_specific;
  pthread_mutex_t       mutex_dev_specific;

  bool                  is_sub;
  cl_mem_flags          sub_flags;
  cl_uint               map_count;
  CLMem*                parent;
  cl_buffer_region      buf_create_info;
  size_t                sub_count;

  // Callback function
  vector<MemObjectDestructorCallback> callbackStack;

  bool                  is_image;
  CLImage               image_obj;
  cl_image_format       image_format;
  cl_image_desc         image_desc;
  size_t                image_row_pitch;
  size_t                image_slice_pitch;
  size_t                image_elem_size;
  size_t                image_channels;

  map<void*, CLMappedInfo> mapped_info;
};

class CLSampler : public CLObject {
public:
  CLSampler(CLContext* context, cl_bool normalized_coords, cl_addressing_mode addressing_mode, cl_filter_mode filter_mode);
  virtual ~CLSampler() {};
	void SetCommand(CLCommand* command);

  _cl_sampler           st_obj;

	vector<CLCommand*>    commands;
  CLContext*            context;
  cl_bool               normalized_coords;
  cl_addressing_mode    addressing_mode;
  cl_filter_mode        filter_mode;

  map<CLDevice*, void*> dev_specific;
  pthread_mutex_t       mutex_dev_specific;
};

class CLProgram : public CLObject {
public:
	CLProgram(CLContext* context, const cl_device_id* device_list, cl_uint num_devices);
  ~CLProgram();
	void SetSrc(CLDevice* dev, char* _src);
	void SetBin(CLDevice* dev, CLBinary* _bin, cl_program_binary_type _type);
	void SetObj(CLDevice* dev, char* idx);
	void CheckOptions(const char* options, cl_int* err);
	void CheckLinkOptions(char* newOptions, const char* options, cl_int* err);
	void FreeKernelInfo();

  _cl_program           st_obj;

  CLContext*            context;
  char*                 src;
  char*                 options;

	//kernel information
  size_t                                       num_kernels;
  char**                                       kernel_names;
	cl_uint*                                     kernel_num_args;
	char**                                       kernel_attributes;
	size_t**                                     kernel_work_group_size_hint;
	size_t**                                     kernel_reqd_work_group_size;
	cl_ulong*                                    kernel_local_mem_size;
	cl_ulong*                                    kernel_private_mem_size;
	cl_kernel_arg_address_qualifier**            kernel_arg_address_qualifier;
	cl_kernel_arg_access_qualifier**             kernel_arg_access_qualifier;
	char***                                      kernel_arg_type_name;
	cl_kernel_arg_type_qualifier**               kernel_arg_type_qualifier;
	char***                                      kernel_arg_name;
	size_t                                       preferred_work_group_size_multiple;
	bool*                                        isBuiltInKernel;

  map<CLDevice*, void*> dev_specific;
  pthread_mutex_t       mutex_dev_specific;

	vector<CLKernel*>     kernelObjs;
  map<CLDevice*, bool>  createWithBinary;
  map<CLDevice*, bool>  fromSource;
  map<CLDevice*, bool>  fromBinary;
  map<CLDevice*, bool>  fromObj;
	bool                  optArgInfo;
  bool                  createLibrary;
  uint                  buildVersion;

	vector<CLDevice*>                     devices;
	map<CLDevice*, CLBinary*>             bins;
  map<CLDevice*, cl_build_status>       buildStatus;
  map<CLDevice*, char*>                 buildLog;
  map<CLDevice*, char*>                 compiledObject;
  map<CLDevice*, vector<char*> >        linkObjects;    //target objects for linking
	vector<ProgramCallback*>              buildCallbackStack;
	vector<ProgramCallback*>              compileCallbackStack;
	vector<ProgramCallback*>              linkCallbackStack;
};

class CLBinary : public CLObject {
public: 
	CLBinary(char* _binary, size_t _size, cl_program_binary_type _type);
	~CLBinary() {};

	char* binary;
	size_t size;
	cl_program_binary_type type;
};

class CLKernel : public CLObject {
public:
  CLKernel(CLProgram* program, const char* kernel_name);
  ~CLKernel();
	void SetCommand(CLCommand* command);

  cl_int SetArg(cl_uint arg_index, size_t arg_size, const void* arg_value);

  _cl_kernel                  st_obj;

  CLProgram*                  program;
  char                        name[256];
  map<cl_uint, CLKernelArg*>  args;
  map<cl_uint, CLKernelArg*>* args_last;
  bool                        args_dirty;

	vector<CLCommand*>          commands;
  map<CLDevice*, void*>       dev_specific;
  pthread_mutex_t             mutex_dev_specific;

	int                         kernel_idx;

	size_t                      global_work_size[3]; //for custom devices
};

class CLKernelArg {
public:
  CLKernelArg(size_t arg_size, const void* arg_value);
  ~CLKernelArg();

  void Init(size_t arg_size, const void* arg_value);

  size_t              size;
  char                value[256];
  bool                local;
  CLMem*              mem;
  CLSampler*          sampler;
  cl_mem_flags        flags;
};

class CLEvent : public CLObject {
public:
  CLEvent(cl_context context, cl_command_type type, cl_command_queue queue, CLCommand* command);
  ~CLEvent();

  cl_int Wait();
  void Complete();
  void SetTimestamp(int type);
	void SetStatus(cl_int status);
  void AddCallback(void (CL_CALLBACK *pfn_notify)(cl_event, cl_int, void *), void * user_data, cl_int command_exec_callback_type);

  _cl_event             st_obj;

	cl_context            context;
  cl_command_type       type;
	cl_command_queue      queue;
  CLCommand*            command;
  volatile cl_int       status;
  sem_t                 sem_complete;

  cl_ulong              profiling_info[5];

  bool                  user;
  bool                  disclosed;
	bool                  userSetStatus;
	bool                  waitEventsComplete;

  map<CLDevice*, void*> dev_specific;
  pthread_mutex_t       mutex_dev_specific;
	vector<EventCallback*> callbackStack;
  pthread_mutex_t       mutex_c;
};

class CLDeviceWorker {
public:
  CLDeviceWorker(CLDevice* device);
  ~CLDeviceWorker();

private:
  static void* ThreadFunc(void* argp);
  void Run();

  CLDevice*           device;
  bool                running;
  pthread_t           thread;
};

class CLScheduler : public CLObject {
public:
  CLScheduler(CLPlatform* platform);
  ~CLScheduler();

  void Start();
  void AddDevice(CLDevice* device);
  bool EnqueueReadyQueue(CLCommand* command, CLDevice* device);
  bool EnqueueRunningCommand(CLCommand* command);

private:
  static void* ThreadFunc(void* argp);
  void Run();

  CLCommand* Schedule(CLCommandQueue* command_queue);

  void Issue(CLCommand* command);
  bool IssueLaunchKernel(CLCommand* command);
  bool IssueNativeKernel(CLCommand* command);
  bool IssueReadBuffer(CLCommand* command);
  bool IssueWriteBuffer(CLCommand* command);
  bool IssueCopyBuffer(CLCommand* command);
  bool IssueBroadcastBuffer(CLCommand* command);

  bool CheckRace(CLMem** mem_list, int num_mem, CLCommand* command);

  void UpdateLatest(CLCommand* command, CLDevice* device);
  CLDevice* NearestDevice(CLMem* mem, CLDevice* device);
  CLDevice* PreferCPUDevice(CLMem* mem);

public:
  CLPlatform*                   platform;
  LockFreeQueue*                ready_queue;
  vector<CLCommand*>            running_commands;
  pthread_mutex_t               mutex_rc;
  CLDevice**                    devices;

private:
  int                           num_devices;
  bool                          running;
  pthread_t                     thread;
};

class CLIssuer {
public:
  CLIssuer(CLPlatform* platform);
  ~CLIssuer();
  void Start();
  void EnqueueCompletionQueue(CLCommand* command);
  bool EnqueueReadyQueue(CLCommand* command, CLDevice* device);
  bool EnqueueRunningCommand(CLCommand* command);

private:
  static void* ThreadFunc(void* argp);
  void Run();
  void InitDevices();
  bool IsComplete(CLEvent* event);

public:
  map<unsigned long, CLCommand*>  issue_list;
  vector<CLEvent*>                wait_events;
  list<CLCommand*>                running_commands;
  vector<CLCommand*>              completion_list;
  LockFreeQueue*                  completion_queue;

private:
  CLPlatform*                     platform;
  bool                            running;
  pthread_t                       thread;

  int                             num_devices;
  CLDevice**                      devices;
  map<int, void*>                 ready_queues;

  map<int, int>                   map_size;
  CLIssuerCleaner*                cleaner;
};

class CLIssuerCleaner {
public:
  CLIssuerCleaner(CLIssuer* issuer);
  ~CLIssuerCleaner();
  void Start();

private:
  static void* ThreadFunc(void* argp);
  void Run();

private:
  CLIssuer* issuer;
  bool                            running;
  pthread_t                       thread;
};

class CLWorkGroupAssignment {
public:
  CLWorkGroupAssignment(CLCommand* command);
  ~CLWorkGroupAssignment();

  CLCommand*          command;
  size_t              wg_id_start;
  size_t              wg_id_end;
};

class CLProfiler {
public:
  CLProfiler(bool debug);
  ~CLProfiler();
  
  void PrintSummary();

  void CreateMem(CLMem* mem);
  void ReleaseMem(CLMem* mem);
  void EnqueueCommand(CLCommand* command);
  void CompleteCommand(CLCommand* command);

private:
  bool                debug;
  size_t              cnt_cmd_k;
  size_t              cnt_cmd_m;
  size_t              cnt_cmd_s;

  size_t              mem_size;
  size_t              mem_size_max;
};

// Single Producer & Single Consumer
class LockFreeQueue {
public:
  LockFreeQueue(unsigned long size);
  ~LockFreeQueue();

  virtual bool Enqueue(CLCommand* element);
  bool Dequeue(CLCommand** element);
  bool Peek(CLCommand** element);
  unsigned long Size();

protected:
  unsigned long           size;
  volatile CLCommand**    elements;
  volatile unsigned long  idx_r;
  volatile unsigned long  idx_w;
};

// Multiple Producers & Single Consumer
class LockFreeQueueMS : public LockFreeQueue {
public:
  LockFreeQueueMS(unsigned long size);
  ~LockFreeQueueMS();

  bool Enqueue(CLCommand* element);

private:
  volatile unsigned long  idx_w_cas;
};

class ProgramCallback {
	public:
		ProgramCallback( void (CL_CALLBACK* pfn_notify)(cl_program, void*), void* user_data){
			this->pfn_notify = pfn_notify;
			this->user_data = user_data;
		}

		void (CL_CALLBACK* pfn_notify)(cl_program, void*);
		void* user_data;
};
class EventCallback {
	public:
		EventCallback( void (CL_CALLBACK* pfn_notify)(cl_event, cl_int, void*), void* user_data, cl_int command_exec_callback_type){
			this->pfn_notify = pfn_notify;
			this->user_data = user_data;
			this->command_exec_callback_type = command_exec_callback_type;
		}

		void (CL_CALLBACK* pfn_notify)(cl_event, cl_int, void*);
		void* user_data;
		cl_int command_exec_callback_type;
};
class MemObjectDestructorCallback {
	public:
		MemObjectDestructorCallback( 
        void (CL_CALLBACK* pfn_notify)(cl_mem, void*), void* user_data){
			this->pfn_notify = pfn_notify;
			this->user_data = user_data;
		}

		void (CL_CALLBACK* pfn_notify)(cl_mem, void*);
		void* user_data;
};


class CommandFactory {
public:
  static CommandFactory* s_instance;
  static CommandFactory* instance() {
    return s_instance;
  }

  CommandFactory();
  ~CommandFactory();

  CLCommand* NewCommand(cl_command_type type);
  CLCommand* NewCommand(CLCommandQueue* command_queue, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_command_type type);

  void FreeCommand(CLCommand* command);
  void FreeCommandEvent(CLCommand* command);

private:
  CLCommand**           commands;
  int                   current;
  pthread_mutex_t       mutex_commands;
};

extern CLPlatform g_platform;

#endif

