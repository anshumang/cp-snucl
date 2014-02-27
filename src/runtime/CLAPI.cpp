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

#include <CLAPI.h>
#include <CLObject.h>
#include <Utils.h>
#include <malloc.h>
#include <limits.h>

SNUCL_DEBUG_HEADER("CLAPI");

/* Platform API */
CL_API_ENTRY cl_int CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clGetPlatformIDs
#else
clGetPlatformIDs
#endif
                (cl_uint          num_entries,
                 cl_platform_id * platforms,
                 cl_uint *        num_platforms) CL_API_SUFFIX__VERSION_1_0 {
  if ((num_entries == 0 && platforms != NULL) || (num_platforms == NULL && platforms == NULL))
    return CL_INVALID_VALUE;

  if (num_platforms) *num_platforms = 1;

  if (platforms == NULL) return CL_SUCCESS;

  platforms[0] = &g_platform.st_obj;
  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL 
#ifdef SNUCL_API_WRAP
__wrap_clGetPlatformInfo
#else
clGetPlatformInfo
#endif
                 (cl_platform_id   platform, 
                  cl_platform_info param_name,
                  size_t           param_value_size, 
                  void *           param_value,
                  size_t *         param_value_size_ret) CL_API_SUFFIX__VERSION_1_0 {
	if (platform == NULL) return CL_INVALID_PLATFORM;

  CLPlatform* _platform = platform->c_obj;
  switch (param_name) {
    SNUCL_GetObjectInfoA(CL_PLATFORM_PROFILE, char, _platform->profile, strlen(_platform->profile) + 1);
    SNUCL_GetObjectInfoA(CL_PLATFORM_VERSION, char, _platform->version, strlen(_platform->version) + 1);
    SNUCL_GetObjectInfoA(CL_PLATFORM_NAME, char, _platform->name, strlen(_platform->name) + 1);
    SNUCL_GetObjectInfoA(CL_PLATFORM_VENDOR, char, _platform->vendor, strlen(_platform->vendor) + 1);
    SNUCL_GetObjectInfoA(CL_PLATFORM_EXTENSIONS, char, _platform->extensions, strlen(_platform->extensions) + 1);
    default: return CL_INVALID_VALUE;
  }
  return CL_SUCCESS;
}

/* Device APIs */
CL_API_ENTRY cl_int CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clGetDeviceIDs
#else
clGetDeviceIDs
#endif
              (cl_platform_id   platform,
               cl_device_type   device_type, 
               cl_uint          num_entries, 
               cl_device_id *   devices, 
               cl_uint *        num_devices) CL_API_SUFFIX__VERSION_1_0 {

  if (platform != &g_platform.st_obj) return CL_INVALID_PLATFORM;

  if (device_type != CL_DEVICE_TYPE_ALL && (!(device_type & (CL_DEVICE_TYPE_DEFAULT | CL_DEVICE_TYPE_CPU | CL_DEVICE_TYPE_GPU | CL_DEVICE_TYPE_ACCELERATOR | CL_DEVICE_TYPE_CUSTOM)))) return CL_INVALID_DEVICE_TYPE;

  if ((num_entries == 0 && devices != NULL) || (num_devices == NULL && devices == NULL))
    return CL_INVALID_VALUE;

  cl_uint num_devices_ret = 0;
  
  if (device_type == CL_DEVICE_TYPE_DEFAULT) device_type = CL_DEVICE_TYPE_GPU;

  for (int i = 0; i < (int) g_platform.devices.size(); ++i) {
    if (g_platform.devices[i]->type & device_type) {
			if (device_type == CL_DEVICE_TYPE_ALL && g_platform.devices[i]->type == CL_DEVICE_TYPE_CUSTOM) continue;
      if (devices != NULL && num_devices_ret < num_entries) {
        devices[num_devices_ret] = &g_platform.devices[i]->st_obj;
      }
      num_devices_ret++;
    }
  }

  if (num_devices) {
		*num_devices = num_devices_ret;
		if (num_entries > 0 && num_devices_ret > num_entries) *num_devices = num_entries;
	}

  if (num_devices_ret == 0) return CL_DEVICE_NOT_FOUND;

  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clGetDeviceInfo
#else
clGetDeviceInfo
#endif
               (cl_device_id    device,
                cl_device_info  param_name, 
                size_t          param_value_size, 
                void *          param_value,
                size_t *        param_value_size_ret) CL_API_SUFFIX__VERSION_1_0 {
	if (device == NULL) return CL_INVALID_DEVICE;
  CLDevice* d = device->c_obj;
  switch (param_name) {
    SNUCL_GetObjectInfo(CL_DEVICE_TYPE, cl_device_type, d->type);
    SNUCL_GetObjectInfo(CL_DEVICE_VENDOR_ID, cl_uint, d->vendor_id);
    SNUCL_GetObjectInfo(CL_DEVICE_MAX_COMPUTE_UNITS, cl_uint, d->max_compute_units);
    SNUCL_GetObjectInfo(CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, cl_uint, d->max_work_item_dimensions);
    SNUCL_GetObjectInfoA(CL_DEVICE_MAX_WORK_ITEM_SIZES, size_t, d->max_work_item_sizes, d->max_work_item_dimensions);
    SNUCL_GetObjectInfo(CL_DEVICE_MAX_WORK_GROUP_SIZE, size_t, d->max_work_group_size);
    SNUCL_GetObjectInfo(CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR, cl_uint, d->preferred_vector_width_char);
    SNUCL_GetObjectInfo(CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT, cl_uint, d->preferred_vector_width_short);
    SNUCL_GetObjectInfo(CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT, cl_uint, d->preferred_vector_width_int);
    SNUCL_GetObjectInfo(CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG, cl_uint, d->preferred_vector_width_long);
    SNUCL_GetObjectInfo(CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT, cl_uint, d->preferred_vector_width_float);
    SNUCL_GetObjectInfo(CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE, cl_uint, d->preferred_vector_width_double);
    SNUCL_GetObjectInfo(CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF, cl_uint, d->preferred_vector_width_half);
    SNUCL_GetObjectInfo(CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR, cl_uint, d->native_vector_width_char);
    SNUCL_GetObjectInfo(CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT, cl_uint, d->native_vector_width_short);
    SNUCL_GetObjectInfo(CL_DEVICE_NATIVE_VECTOR_WIDTH_INT, cl_uint, d->native_vector_width_int);
    SNUCL_GetObjectInfo(CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG, cl_uint, d->native_vector_width_long);
    SNUCL_GetObjectInfo(CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT, cl_uint, d->native_vector_width_float);
    SNUCL_GetObjectInfo(CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE, cl_uint, d->native_vector_width_double);
    SNUCL_GetObjectInfo(CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF, cl_uint, d->native_vector_width_half);
    SNUCL_GetObjectInfo(CL_DEVICE_MAX_CLOCK_FREQUENCY, cl_uint, d->max_clock_frequency);
    SNUCL_GetObjectInfo(CL_DEVICE_ADDRESS_BITS, cl_uint, d->address_bits);
    SNUCL_GetObjectInfo(CL_DEVICE_MAX_MEM_ALLOC_SIZE, cl_ulong, d->max_mem_alloc_size);
    SNUCL_GetObjectInfo(CL_DEVICE_IMAGE_SUPPORT, cl_bool, d->image_support);
    SNUCL_GetObjectInfo(CL_DEVICE_MAX_READ_IMAGE_ARGS, cl_uint, d->max_read_image_args);
    SNUCL_GetObjectInfo(CL_DEVICE_MAX_WRITE_IMAGE_ARGS, cl_uint, d->max_write_image_args);
    SNUCL_GetObjectInfo(CL_DEVICE_IMAGE2D_MAX_WIDTH , size_t, d->image2d_max_width );
    SNUCL_GetObjectInfo(CL_DEVICE_IMAGE2D_MAX_HEIGHT, size_t, d->image2d_max_height);
    SNUCL_GetObjectInfo(CL_DEVICE_IMAGE3D_MAX_WIDTH , size_t, d->image3d_max_width );
    SNUCL_GetObjectInfo(CL_DEVICE_IMAGE3D_MAX_HEIGHT, size_t, d->image3d_max_height);
    SNUCL_GetObjectInfo(CL_DEVICE_IMAGE3D_MAX_DEPTH , size_t, d->image3d_max_depth );
    SNUCL_GetObjectInfo(CL_DEVICE_IMAGE_MAX_BUFFER_SIZE, size_t, d->image_max_buffer_size);
    SNUCL_GetObjectInfo(CL_DEVICE_IMAGE_MAX_ARRAY_SIZE , size_t, d->image_max_array_size );
    SNUCL_GetObjectInfo(CL_DEVICE_MAX_SAMPLERS, cl_uint, d->max_samplers);
    SNUCL_GetObjectInfo(CL_DEVICE_MAX_PARAMETER_SIZE, size_t, d->max_parameter_size);
    SNUCL_GetObjectInfo(CL_DEVICE_MEM_BASE_ADDR_ALIGN, cl_uint, d->mem_base_addr_align);
    SNUCL_GetObjectInfo(CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE, cl_uint, d->min_data_type_align_size);
    SNUCL_GetObjectInfo(CL_DEVICE_SINGLE_FP_CONFIG, cl_device_fp_config, d->single_fp_config);
    SNUCL_GetObjectInfo(CL_DEVICE_DOUBLE_FP_CONFIG, cl_device_fp_config, d->double_fp_config);
    SNUCL_GetObjectInfo(CL_DEVICE_GLOBAL_MEM_CACHE_TYPE, cl_device_mem_cache_type, d->global_mem_cache_type);
    SNUCL_GetObjectInfo(CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE, cl_uint, d->global_mem_cacheline_size);
    SNUCL_GetObjectInfo(CL_DEVICE_GLOBAL_MEM_CACHE_SIZE, cl_ulong, d->global_mem_cache_size);
    SNUCL_GetObjectInfo(CL_DEVICE_GLOBAL_MEM_SIZE, cl_ulong, d->global_mem_size);
    SNUCL_GetObjectInfo(CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, cl_ulong, d->max_constant_buffer_size);
    SNUCL_GetObjectInfo(CL_DEVICE_MAX_CONSTANT_ARGS, cl_uint, d->max_constant_args);
    SNUCL_GetObjectInfo(CL_DEVICE_LOCAL_MEM_TYPE, cl_device_local_mem_type, d->local_mem_type);
    SNUCL_GetObjectInfo(CL_DEVICE_LOCAL_MEM_SIZE, cl_ulong, d->local_mem_size);
    SNUCL_GetObjectInfo(CL_DEVICE_ERROR_CORRECTION_SUPPORT, cl_bool, d->error_correction_support);
    SNUCL_GetObjectInfo(CL_DEVICE_HOST_UNIFIED_MEMORY, cl_bool, d->host_unified_memory);
    SNUCL_GetObjectInfo(CL_DEVICE_PROFILING_TIMER_RESOLUTION, size_t, d->profiling_timer_resolution);
    SNUCL_GetObjectInfo(CL_DEVICE_ENDIAN_LITTLE, cl_bool, d->endian_little);
    SNUCL_GetObjectInfo(CL_DEVICE_AVAILABLE, cl_bool, d->available);
    SNUCL_GetObjectInfo(CL_DEVICE_COMPILER_AVAILABLE, cl_bool, d->compiler_available);
    SNUCL_GetObjectInfo(CL_DEVICE_LINKER_AVAILABLE, cl_bool, d->linker_available);
    SNUCL_GetObjectInfo(CL_DEVICE_EXECUTION_CAPABILITIES, cl_device_exec_capabilities, d->execution_capabilities);
    SNUCL_GetObjectInfo(CL_DEVICE_QUEUE_PROPERTIES, cl_command_queue_properties, d->queue_properties);
    SNUCL_GetObjectInfoA(CL_DEVICE_BUILT_IN_KERNELS, char, d->built_in_kernels, strlen(d->built_in_kernels) + 1);
    SNUCL_GetObjectInfoV(CL_DEVICE_PLATFORM, cl_platform_id, &d->platform->st_obj);
    SNUCL_GetObjectInfoA(CL_DEVICE_NAME, char, d->name, strlen(d->name) + 1);
    SNUCL_GetObjectInfoA(CL_DEVICE_VENDOR, char, d->vendor, strlen(d->vendor) + 1);
    SNUCL_GetObjectInfoA(CL_DRIVER_VERSION, char, d->driver_version, strlen(d->driver_version) + 1);
    SNUCL_GetObjectInfoA(CL_DEVICE_PROFILE, char, d->profile, strlen(d->profile) + 1);
    SNUCL_GetObjectInfoA(CL_DEVICE_VERSION, char, d->device_version, strlen(d->device_version) + 1);
    SNUCL_GetObjectInfoA(CL_DEVICE_OPENCL_C_VERSION, char, d->openclc_version, strlen(d->openclc_version) + 1);
    SNUCL_GetObjectInfoA(CL_DEVICE_EXTENSIONS, char, d->device_extensions, strlen(d->device_extensions) + 1);
    SNUCL_GetObjectInfo(CL_DEVICE_PRINTF_BUFFER_SIZE, size_t, d->printf_buffer_size);
    SNUCL_GetObjectInfo(CL_DEVICE_PREFERRED_INTEROP_USER_SYNC, cl_bool, d->preferred_interop_user_sync);
    SNUCL_GetObjectInfo(CL_DEVICE_PARENT_DEVICE, cl_device_id, d->parent_device);
    SNUCL_GetObjectInfo(CL_DEVICE_PARTITION_MAX_SUB_DEVICES, cl_uint, d->partition_max_sub_devices);
    SNUCL_GetObjectInfoA(CL_DEVICE_PARTITION_PROPERTIES, cl_device_partition_property, d->partition_properties, d->num_partition_properties);
    SNUCL_GetObjectInfo(CL_DEVICE_PARTITION_AFFINITY_DOMAIN, cl_device_affinity_domain, d->affinity_domain);
    SNUCL_GetObjectInfoA(CL_DEVICE_PARTITION_TYPE, cl_device_partition_property, d->partition_type, d->partition_type_len);
    SNUCL_GetObjectInfo(CL_DEVICE_REFERENCE_COUNT, cl_uint, d->ref_cnt);
    default: return CL_INVALID_VALUE;
  }
  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clCreateSubDevices
#else
clCreateSubDevices
#endif
                  (cl_device_id                         in_device,
                   const cl_device_partition_property * properties,
                   cl_uint                              num_devices,
                   cl_device_id *                       out_devices,
                   cl_uint *                            num_devices_ret) CL_API_SUFFIX__VERSION_1_2 {
	cl_int err;

	if (in_device == NULL) return CL_INVALID_DEVICE;
	if (properties == NULL) return CL_INVALID_VALUE;
  CLDevice* dev = in_device->c_obj;

	if (dev->subDevices.size() > 0) return CL_DEVICE_PARTITION_FAILED;

	err = dev->DivideDevices(properties, num_devices, out_devices, num_devices_ret);
	if(err != CL_SUCCESS) return err;

	if (out_devices != NULL && num_devices < dev->numSubDevices) return CL_INVALID_VALUE;

	return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clRetainDevice
#else
clRetainDevice
#endif
               (cl_device_id    device) CL_API_SUFFIX__VERSION_1_2 {

	if (!device) return CL_INVALID_DEVICE;
	if (device->c_obj->parent_device) device->c_obj->Retain();
  return CL_SUCCESS;
}
    
CL_API_ENTRY cl_int CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clReleaseDevice
#else
clReleaseDevice
#endif
               (cl_device_id    device) CL_API_SUFFIX__VERSION_1_2 {
	bool attachedObjects;
	if (!device) return CL_INVALID_DEVICE;
	CLDevice* d = device->c_obj;

	if (d->parent_device != NULL) {
		if (d->Release() == 0) {
			attachedObjects = false;
			for (uint i=0; i<d->num_command_queues; i++) {
				if (d->command_queue_retained[i]) {
					attachedObjects = true;
					break;
				}
			}

			if (!attachedObjects) {
				CLDevice* p = d->parent_device->c_obj;
				for (vector<CLDevice*>::iterator it = p->subDevices.begin(); it != p->subDevices.end(); it++) {
					if (*it == d) {
						p->subDevices.erase(it);
						break;
					}
				}
				delete d; 
			}
		}
	}

  return CL_SUCCESS;
}

//SNS_ to be changed. Constant device number at start. Get the number of devices and then use that each time to create a context
/* Context APIs */
CL_API_ENTRY cl_context CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clCreateContext
#else
clCreateContext
#endif
               (const cl_context_properties * properties,
                cl_uint                       num_devices,
                const cl_device_id *          devices,
                void (CL_CALLBACK *pfn_notify)(const char *, const void *, size_t, void *),
                void *                        user_data,
                cl_int *                      errcode_ret) CL_API_SUFFIX__VERSION_1_0 {
  cl_int err = CL_SUCCESS;
	size_t num_properties = 0;

  if (devices == NULL) err = CL_INVALID_VALUE;
  if (num_devices == 0) err = CL_INVALID_VALUE;
  if (pfn_notify == NULL && user_data != NULL) err = CL_INVALID_VALUE;

  if (errcode_ret) *errcode_ret = err;
  if (err != CL_SUCCESS) return NULL;

	SNUCL_DevicesVerification(g_platform.devices, g_platform.devices.size(), devices, num_devices, err);

	for (uint i=0; i<num_devices; i++) {
		if (!devices[i]->c_obj->available) {
			err = CL_DEVICE_NOT_AVAILABLE;
			break;
		}
	}

  if (errcode_ret) *errcode_ret = err;
  if (err != CL_SUCCESS) return NULL;

  CLContext* context = new CLContext(num_devices, devices);
	err = context->SetProperties(properties);

  if (errcode_ret) *errcode_ret = err;
  if (err != CL_SUCCESS) {
		delete context;
		return NULL;
	}

  return &context->st_obj;
}

CL_API_ENTRY cl_context CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clCreateContextFromType
#else
clCreateContextFromType
#endif
                       (const cl_context_properties * properties,
                        cl_device_type                device_type,
                        void (CL_CALLBACK *pfn_notify)(const char *, const void *, size_t, void *),
                        void *                        user_data,
                        cl_int *                      errcode_ret) CL_API_SUFFIX__VERSION_1_0 {
	vector<CLDevice*> devices;
  cl_int err = CL_SUCCESS;
	size_t num_properties;

  CLContext* context = new CLContext(0, NULL);
	err = context->SetDevices(g_platform.devices, device_type);
	err = context->SetProperties(properties);

  if (pfn_notify == NULL && user_data != NULL) err = CL_INVALID_VALUE;

  if (errcode_ret) *errcode_ret = err;
  if (err != CL_SUCCESS) {
		delete context;
		return NULL;
	}

  return &context->st_obj;
}

CL_API_ENTRY cl_int CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clRetainContext
#else
clRetainContext
#endif
                 (cl_context context) CL_API_SUFFIX__VERSION_1_0 {

	if(!context) return CL_INVALID_CONTEXT;
  context->c_obj->Retain();
  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clReleaseContext
#else
clReleaseContext
#endif
                 (cl_context context) CL_API_SUFFIX__VERSION_1_0 {
	if(!context) return CL_INVALID_CONTEXT;
	CLContext* c = context->c_obj;

	bool allReleased = true;
	if (c->Release() == 0) {
		if (c->mems.size() == 0) {
			for (uint i = 0; i < c->devices.size(); i++) {
				if (c->devices[i]->command_queues != NULL) {
					allReleased = false;
					break;
				}
			}
			if (allReleased) delete c;
		}
	}

  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clGetContextInfo
#else
clGetContextInfo
#endif
                (cl_context         context, 
                 cl_context_info    param_name, 
                 size_t             param_value_size, 
                 void *             param_value, 
                 size_t *           param_value_size_ret) CL_API_SUFFIX__VERSION_1_0 {
	if (context == NULL) return CL_INVALID_CONTEXT;
  CLContext* c = context->c_obj;

	switch (param_name) {
		SNUCL_GetObjectInfo(CL_CONTEXT_REFERENCE_COUNT, cl_uint, c->ref_cnt);
		SNUCL_GetObjectInfo(CL_CONTEXT_NUM_DEVICES, cl_uint, c->num_devices);
    case CL_CONTEXT_DEVICES:
    {
      size_t size = sizeof(cl_device_id) * c->devices.size();
      if (param_value) {
        if (param_value_size < size) return CL_INVALID_VALUE;
        for (int i = 0; i < (int) c->devices.size(); ++i) {
          ((cl_device_id*) param_value)[i] = &c->devices[i]->st_obj;
        }
      }
      if (param_value_size_ret) *param_value_size_ret = size;
      break;
    }
    SNUCL_GetObjectInfoA(CL_CONTEXT_PROPERTIES, cl_context_properties, c->properties, c->num_properties);
    default: return CL_INVALID_VALUE;
  }
  return CL_SUCCESS;
}

/* Command Queue APIs */
CL_API_ENTRY cl_command_queue CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clCreateCommandQueue
#else
clCreateCommandQueue
#endif
                    (cl_context                     context, 
                     cl_device_id                   device, 
                     cl_command_queue_properties    properties,
                     cl_int *                       errcode_ret) CL_API_SUFFIX__VERSION_1_0 {
  cl_int err = CL_SUCCESS;
	if (context == NULL) err = CL_INVALID_CONTEXT;
	SNUCL_DevicesVerification(context->c_obj->devices, context->c_obj->devices.size(), &device, 1, err);

  if (errcode_ret) *errcode_ret = err;
  if (err != CL_SUCCESS) return NULL;

  CLCommandQueue* command_queue = new CLCommandQueue(context->c_obj, device->c_obj, properties);

  return &command_queue->st_obj;
}

CL_API_ENTRY cl_int CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clRetainCommandQueue
#else
clRetainCommandQueue
#endif
                   (cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0 {

	if (command_queue == NULL) return CL_INVALID_COMMAND_QUEUE;
  command_queue->c_obj->Retain();
  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clReleaseCommandQueue
#else
clReleaseCommandQueue
#endif
                   (cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0 {
	if (command_queue == NULL) return CL_INVALID_COMMAND_QUEUE;
	clFlush(command_queue);

	CLCommandQueue* q = command_queue->c_obj;

	if (q->Release() == 0) {
		if (q->last_event && q->last_event->status != CL_COMPLETE) return CL_SUCCESS;

		q->device->command_queue_retained[q->queueIdx] = false;
		if (q->device->ref_cnt == 0) clReleaseDevice(&q->device->st_obj);
	}

  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clGetCommandQueueInfo
#else
clGetCommandQueueInfo
#endif
                     (cl_command_queue      command_queue,
                      cl_command_queue_info param_name,
                      size_t                param_value_size,
                      void *                param_value,
                      size_t *              param_value_size_ret) CL_API_SUFFIX__VERSION_1_0 {
	if (command_queue == NULL) return CL_INVALID_COMMAND_QUEUE;
  CLCommandQueue* q = command_queue->c_obj;

  switch (param_name) {
    SNUCL_GetObjectInfoV(CL_QUEUE_CONTEXT, cl_context, &q->context->st_obj);
    SNUCL_GetObjectInfoV(CL_QUEUE_DEVICE, cl_device_id, &q->device->st_obj);
    SNUCL_GetObjectInfo(CL_QUEUE_REFERENCE_COUNT, cl_uint, q->ref_cnt);
    SNUCL_GetObjectInfo(CL_QUEUE_PROPERTIES, cl_command_queue_properties, q->properties);
    default: return CL_INVALID_VALUE;
	}
  return CL_SUCCESS;
}


/* Memory Object APIs */
CL_API_ENTRY cl_mem CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clCreateBuffer
#else
clCreateBuffer
#endif
              (cl_context   context,
               cl_mem_flags flags,
               size_t       size,
               void *       host_ptr,
               cl_int *     errcode_ret) CL_API_SUFFIX__VERSION_1_0 {
  cl_int err = CL_SUCCESS;
  if (context == NULL) err = CL_INVALID_CONTEXT;
  else if (size == 0) err = CL_INVALID_BUFFER_SIZE;
  else if ((host_ptr == NULL) && (flags & (CL_MEM_USE_HOST_PTR | CL_MEM_COPY_HOST_PTR))) err = CL_INVALID_HOST_PTR;
  else if ((host_ptr != NULL) && (!(flags & (CL_MEM_USE_HOST_PTR | CL_MEM_COPY_HOST_PTR)))) err = CL_INVALID_HOST_PTR;
  if (g_platform.cluster && (flags & CL_MEM_USE_HOST_PTR)) err = CL_INVALID_OPERATION;

  if (errcode_ret) *errcode_ret = err;
  if (err != CL_SUCCESS) return NULL;

  CLMem* mem = new CLMem(context->c_obj, flags, size, host_ptr, false);
  g_platform.profiler->CreateMem(mem);

  return &mem->st_obj;
}

/* Memory Object APIs */
CL_API_ENTRY cl_mem CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clCreateSubBuffer
#else
clCreateSubBuffer
#endif
              (cl_mem                  buffer,
              cl_mem_flags             flags,
              cl_buffer_create_type    types,
              const void *             buf_create_info,
              cl_int *                 errcode_ret) CL_API_SUFFIX__VERSION_1_1 {
  cl_int err = CL_SUCCESS;
  cl_buffer_region *buf_region = (cl_buffer_region*) buf_create_info;

  if (buffer == NULL) err = CL_INVALID_MEM_OBJECT;
  else if (buffer->c_obj->is_sub) err = CL_INVALID_MEM_OBJECT;
  else if (types != CL_BUFFER_CREATE_TYPE_REGION) {
    err = CL_INVALID_VALUE;
  }
  else if (buf_create_info == NULL) {
    err = CL_INVALID_VALUE;
  }
  else if (buf_region->size == 0) err = CL_INVALID_BUFFER_SIZE;
  else if ((buf_region->origin+buf_region->size) > buffer->c_obj->size) {
    err = CL_INVALID_VALUE;
  }
  else if ((buffer->c_obj->flags & CL_MEM_WRITE_ONLY) && ((flags & CL_MEM_READ_WRITE) || (flags & CL_MEM_READ_ONLY))) {
    err = CL_INVALID_VALUE;
  }
  else if ((buffer->c_obj->flags & CL_MEM_READ_ONLY) && ((flags & CL_MEM_READ_WRITE) || (flags & CL_MEM_WRITE_ONLY))) {
    err = CL_INVALID_VALUE;
  }
  else if ((buffer->c_obj->flags & CL_MEM_HOST_WRITE_ONLY) && (flags & CL_MEM_HOST_READ_ONLY)) {
    err = CL_INVALID_VALUE;
  }
  else if ((buffer->c_obj->flags & CL_MEM_HOST_READ_ONLY) && (flags & CL_MEM_HOST_WRITE_ONLY)) {
    err = CL_INVALID_VALUE;
  }
  else if ((buffer->c_obj->flags & CL_MEM_HOST_NO_ACCESS) && ((flags & CL_MEM_HOST_READ_ONLY) || (flags & CL_MEM_HOST_WRITE_ONLY))) {
    err = CL_INVALID_VALUE;
  }
  if (g_platform.cluster) err = CL_INVALID_OPERATION;

  if (errcode_ret) *errcode_ret = err;
  if (err != CL_SUCCESS) return NULL;

  cl_mem_flags new_flags = flags;
  cl_mem_flags parent_flags = buffer->c_obj->flags;

  if ((flags&(CL_MEM_READ_WRITE | CL_MEM_READ_ONLY | CL_MEM_WRITE_ONLY)) == 0)
    new_flags |= parent_flags &(CL_MEM_READ_WRITE | CL_MEM_READ_ONLY | CL_MEM_WRITE_ONLY);
  new_flags |= parent_flags & (CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR | CL_MEM_USE_HOST_PTR);
  if ( (flags & (CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_WRITE_ONLY | CL_MEM_HOST_NO_ACCESS)) == 0)
    new_flags |= parent_flags & (CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_WRITE_ONLY | CL_MEM_HOST_NO_ACCESS);

  CLMem* new_mem = new CLMem(buffer->c_obj->context, new_flags, buf_region->size, NULL, true);
  buffer->c_obj->sub_count++;
  new_mem->parent = buffer->c_obj;
  new_mem->is_sub = true;
  new_mem->sub_flags = flags;
  new_mem->create_type = types;
  new_mem->buf_create_info = *buf_region;

  new_mem->alloc_host = buffer->c_obj->alloc_host;
  new_mem->space_host = (void*)((size_t)(buffer->c_obj->space_host) + buf_region->origin);
  new_mem->use_host = buffer->c_obj->use_host;

  return &new_mem->st_obj;
}

CL_API_ENTRY cl_mem CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clCreateImage
#else
clCreateImage
#endif
              (cl_context               context,
              cl_mem_flags              flags,
              const cl_image_format *   image_format,
              const cl_image_desc *     image_desc, 
              void *                    host_ptr,
              cl_int *                  errcode_ret) CL_API_SUFFIX__VERSION_1_2 {
  cl_int err = CL_SUCCESS;
  if (context == NULL) err = CL_INVALID_CONTEXT;
  else if (image_format == NULL) err = CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
  else if (image_desc == NULL) err = CL_INVALID_IMAGE_DESCRIPTOR;
  else if ((host_ptr == NULL) && ((flags & CL_MEM_USE_HOST_PTR) || (flags & CL_MEM_COPY_HOST_PTR))) err = CL_INVALID_HOST_PTR;
  if (g_platform.cluster && (flags & CL_MEM_USE_HOST_PTR)) err = CL_INVALID_OPERATION;

  if (errcode_ret) *errcode_ret = err;
  if (err != CL_SUCCESS) return NULL;

  size_t size = 0;
  size_t elem_size = 0;
  size_t row_pitch = 0;
  size_t slice_pitch = 0;
  int channels = 4;

  GetImageDesc(image_format, image_desc, &size, &elem_size, &row_pitch, &slice_pitch, &channels, errcode_ret);
  if (errcode_ret) *errcode_ret = err;
  if (err != CL_SUCCESS) return NULL;

  CLMem *mem = new CLMem(context->c_obj, flags, size, host_ptr, false);
  mem->is_image = true;
  mem->image_format = *image_format;
  mem->image_desc = *image_desc;
  mem->image_row_pitch = row_pitch;
  mem->image_slice_pitch = slice_pitch;
  mem->image_elem_size = elem_size;
  mem->image_channels = channels;

  return &mem->st_obj;
}

CL_API_ENTRY cl_int CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clRetainMemObject
#else
clRetainMemObject
#endif
               (cl_mem memobj) CL_API_SUFFIX__VERSION_1_0 {

	if (memobj == NULL) return CL_INVALID_MEM_OBJECT;
  memobj->c_obj->Retain();
  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clReleaseMemObject
#else
clReleaseMemObject
#endif
               (cl_mem memobj) CL_API_SUFFIX__VERSION_1_0 {
	if (memobj == NULL) return CL_INVALID_MEM_OBJECT;
	CLMem* m = memobj->c_obj;

  g_platform.profiler->ReleaseMem(m);

	if (m->Release() == 0) {
		for (int i = 0; i < m->commands.size(); i++) {
			if (m->commands[i]->event->status != CL_COMPLETE) return CL_SUCCESS;
		}
		for (vector<CLMem*>::iterator it = m->context->mems.begin(); it != m->context->mems.end(); ++it) {
			if ((*it) == m) {
				m->context->mems.erase(it);
				if (m->context->ref_cnt == 0 && m->context->mems.size() == 0) clReleaseContext(&m->context->st_obj);
				delete m; 
				break;
			}
		}
	}
	return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clGetSupportedImageFormats
#else
clGetSupportedImageFormats
#endif
                          (cl_context           context,
                           cl_mem_flags         flags,
                           cl_mem_object_type   image_type,
                           cl_uint              num_entries,
                           cl_image_format *    image_formats,
                           cl_uint *            num_image_formats) CL_API_SUFFIX__VERSION_1_0 {
  if (context == NULL) return CL_INVALID_CONTEXT;

  if(num_image_formats)
    *num_image_formats = 0;
  vector<CLDevice*>* devices = &context->c_obj->devices;
  for (vector<CLDevice*>::iterator it = devices->begin(); it != devices->end(); ++it) {
    cl_int err;
    cl_uint local_num_image_formats;
    err = (*it)->GetSupportedImageFormats(flags, image_type, num_entries, image_formats, &local_num_image_formats);
    if (err != CL_SUCCESS) {
      return err;
    }
    num_entries -= local_num_image_formats;
    if(image_formats)
      image_formats += local_num_image_formats;
    if(num_image_formats) {
      *num_image_formats += local_num_image_formats;
    }
  }

  return CL_SUCCESS;
}
                                    
CL_API_ENTRY cl_int CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clGetMemObjectInfo
#else
clGetMemObjectInfo
#endif
                  (cl_mem           memobj,
                   cl_mem_info      param_name, 
                   size_t           param_value_size,
                   void *           param_value,
                   size_t *         param_value_size_ret) CL_API_SUFFIX__VERSION_1_0 {
  if (memobj == NULL) return CL_INVALID_MEM_OBJECT;

  size_t size = 0;
  size_t tmp = 0;
  char *null_val = 0;
  cl_context context = &memobj->c_obj->context->st_obj;
  cl_mem parent = &memobj->c_obj->parent->st_obj;

  switch (param_name) {
    case CL_MEM_TYPE:
      if (memobj->c_obj->is_image) {
        size = sizeof(cl_mem_object_type);
        if (param_value_size < size) return CL_INVALID_VALUE;
        memcpy(param_value, &memobj->c_obj->image_desc.image_type, size);
        if(param_value_size_ret)
          *param_value_size_ret = size;
      }
      else {
        size = sizeof(cl_mem_object_type);
        if (param_value_size < size) return CL_INVALID_VALUE;
        cl_mem_object_type type = CL_MEM_OBJECT_BUFFER;
        memcpy(param_value, &type, size);
        if(param_value_size_ret)
          *param_value_size_ret = size;
      }
      break;
    SNUCL_GetObjectInfo(CL_MEM_FLAGS, cl_mem_flags, memobj->c_obj->flags);
    SNUCL_GetObjectInfo(CL_MEM_SIZE, size_t, memobj->c_obj->size);
    case CL_MEM_HOST_PTR:
      size = sizeof(void*);
      if (memobj->c_obj->flags & CL_MEM_USE_HOST_PTR) {
        if (param_value_size < size) return CL_INVALID_VALUE;
        memcpy(param_value, &memobj->c_obj->space_host, size);
        *param_value_size_ret = size;
      }
      else {
        if (param_value_size < size) return CL_INVALID_VALUE;
        tmp = 0; // as NULL value
        memcpy(param_value, &tmp, size);
        *param_value_size_ret = size;
      }
      break;
    SNUCL_GetObjectInfo(CL_MEM_MAP_COUNT, cl_uint, memobj->c_obj->map_count);
    SNUCL_GetObjectInfo(CL_MEM_REFERENCE_COUNT, int, memobj->c_obj->ref_cnt);
    SNUCL_GetObjectInfo(CL_MEM_CONTEXT, cl_context, context);
    case CL_MEM_ASSOCIATED_MEMOBJECT:
      size = sizeof(cl_mem);
      if (memobj->c_obj->is_sub) {
        if (param_value_size < size) return CL_INVALID_VALUE;
        memcpy(param_value, &parent, size);
        *param_value_size_ret = size;
      }
      else {
        if(param_value_size < size) return CL_INVALID_VALUE;
        tmp = 0; // as NULL value
        memcpy(param_value, &tmp, size);
        *param_value_size_ret = size;
      }
      break;
    case CL_MEM_OFFSET:
      size = sizeof(size_t);
      if (memobj->c_obj->is_sub) {
        tmp = memobj->c_obj->buf_create_info.origin;
        if (param_value_size < size) return CL_INVALID_VALUE;
        memcpy(param_value, &tmp, size);
        *param_value_size_ret = size;
      }
      else {
        if (param_value_size < size) return CL_INVALID_VALUE;
        tmp = 0; // as NULL value
        memcpy(param_value, &tmp, size);
        *param_value_size_ret = size;
      }
      break;
    default:
      return CL_INVALID_VALUE;
  }

  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clGetImageInfo
#else
clGetImageInfo
#endif
              (cl_mem           image,
               cl_image_info    param_name, 
               size_t           param_value_size,
               void *           param_value,
               size_t *         param_value_size_ret) CL_API_SUFFIX__VERSION_1_0 {
  if (image == NULL) return CL_INVALID_MEM_OBJECT;

  size_t size = 0;
  size_t tmp = 0;

  switch (param_name) {
    SNUCL_GetObjectInfo(CL_IMAGE_FORMAT, cl_image_format, image->c_obj->image_format);
    SNUCL_GetObjectInfo(CL_IMAGE_ELEMENT_SIZE, size_t, image->c_obj->image_elem_size);
    SNUCL_GetObjectInfo(CL_IMAGE_ROW_PITCH, size_t, image->c_obj->image_row_pitch);
    SNUCL_GetObjectInfo(CL_IMAGE_SLICE_PITCH, size_t, image->c_obj->image_slice_pitch);
    SNUCL_GetObjectInfo(CL_IMAGE_WIDTH, size_t, image->c_obj->image_desc.image_width);
    SNUCL_GetObjectInfo(CL_IMAGE_HEIGHT, size_t, image->c_obj->image_desc.image_height);
    SNUCL_GetObjectInfo(CL_IMAGE_DEPTH, size_t, image->c_obj->image_desc.image_depth);
    SNUCL_GetObjectInfo(CL_IMAGE_ARRAY_SIZE, size_t, image->c_obj->image_desc.image_array_size);
    SNUCL_GetObjectInfo(CL_IMAGE_BUFFER, cl_mem, image->c_obj->image_desc.buffer);
    SNUCL_GetObjectInfo(CL_IMAGE_NUM_MIP_LEVELS, cl_uint, image->c_obj->image_desc.num_mip_levels);
    SNUCL_GetObjectInfo(CL_IMAGE_NUM_SAMPLES, cl_uint, image->c_obj->image_desc.num_samples);
    default: return CL_INVALID_VALUE;
  }

  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clSetMemObjectDestructorCallback
#else
clSetMemObjectDestructorCallback
#endif
( cl_mem memobj, 
  void (CL_CALLBACK * pfn_notify)( cl_mem memobj, void* user_data), 
  void * user_data
) CL_API_SUFFIX__VERSION_1_1 {

  if (memobj == NULL) return CL_INVALID_MEM_OBJECT;
  if (pfn_notify == NULL) return CL_INVALID_VALUE;

  MemObjectDestructorCallback callback(pfn_notify, user_data);
  memobj->c_obj->callbackStack.push_back(callback);
	return CL_SUCCESS;
}

/* Sampler APIs */
CL_API_ENTRY cl_sampler CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clCreateSampler
#else
clCreateSampler
#endif
               (cl_context          context,
                cl_bool             normalized_coords, 
                cl_addressing_mode  addressing_mode, 
                cl_filter_mode      filter_mode,
                cl_int *            errcode_ret) CL_API_SUFFIX__VERSION_1_0 {
  cl_int err = CL_SUCCESS;
  if (context == NULL) err = CL_INVALID_CONTEXT;
  if (errcode_ret) *errcode_ret = err;
  if (err != CL_SUCCESS) return NULL;
  
  CLSampler* sampler = new CLSampler(context->c_obj, normalized_coords, addressing_mode, filter_mode);

  return &sampler->st_obj;
}

CL_API_ENTRY cl_int CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clRetainSampler
#else
clRetainSampler
#endif
               (cl_sampler sampler) CL_API_SUFFIX__VERSION_1_0 {
  if (sampler == NULL) return CL_INVALID_SAMPLER; 
  sampler->c_obj->Retain();
  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clReleaseSampler
#else
clReleaseSampler
#endif
               (cl_sampler sampler) CL_API_SUFFIX__VERSION_1_0 {
  if (sampler == NULL) return CL_INVALID_SAMPLER; 
	CLSampler* s = sampler->c_obj;

	if (s->Release() == 0) {
		for (int i = 0; i < s->commands.size(); i++) {
			if (s->commands[i]->event->status != CL_COMPLETE) return CL_SUCCESS;
		}
		for (vector<CLSampler*>::iterator it = s->context->samplers.begin(); it != s->context->samplers.end(); ++it) {
			if ((*it) == s) {
				s->context->samplers.erase(it);
				if (s->context->ref_cnt == 0 && s->context->samplers.size() == 0) clReleaseContext(&s->context->st_obj);
				delete s; 
				break;
			}
		}
	}
  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clGetSamplerInfo
#else
clGetSamplerInfo
#endif
                (cl_sampler         sampler,
                 cl_sampler_info    param_name,
                 size_t             param_value_size,
                 void *             param_value,
                 size_t *           param_value_size_ret) CL_API_SUFFIX__VERSION_1_0 {
  if (sampler == NULL) return CL_INVALID_SAMPLER;

  cl_context context = &sampler->c_obj->context->st_obj;

  switch (param_name) {
    SNUCL_GetObjectInfo(CL_SAMPLER_REFERENCE_COUNT, cl_uint, sampler->c_obj->ref_cnt);
    SNUCL_GetObjectInfo(CL_SAMPLER_CONTEXT, cl_context, context);
    SNUCL_GetObjectInfo(CL_SAMPLER_NORMALIZED_COORDS, cl_bool, sampler->c_obj->normalized_coords);
    SNUCL_GetObjectInfo(CL_SAMPLER_ADDRESSING_MODE, cl_addressing_mode, sampler->c_obj->addressing_mode);
    SNUCL_GetObjectInfo(CL_SAMPLER_FILTER_MODE, cl_filter_mode, sampler->c_obj->filter_mode);
    default: return CL_INVALID_VALUE;
  }

  return CL_SUCCESS;
}
                            
/* Program Object APIs */
CL_API_ENTRY cl_program CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clCreateProgramWithSource
#else
clCreateProgramWithSource
#endif
                         (cl_context        context,
                          cl_uint           count,
                          const char **     strings,
                          const size_t *    lengths,
                          cl_int *          errcode_ret) CL_API_SUFFIX__VERSION_1_0 {
  cl_int err = CL_SUCCESS;

  if (context == NULL) err = CL_INVALID_CONTEXT;
  else if ((count == 0) || (strings == NULL)) err = CL_INVALID_VALUE;
  else if (strings) {
		for (uint i = 0; i < count; ++i) {
			if (strings[i] == NULL) {
				err = CL_INVALID_VALUE;
				break;
			}
		}
	}

  if (errcode_ret) *errcode_ret = err;

  if (err != CL_SUCCESS) return NULL;

  size_t buf_size = SNUCL_GetStringsLength(count, strings, lengths);

  if (buf_size == 1) err = CL_INVALID_VALUE;

  if (errcode_ret) *errcode_ret = err;

  if (err != CL_SUCCESS) return NULL;

  char* buf = (char*) calloc(buf_size, sizeof(char));
  SNUCL_CopyStringsToBuf(buf, count, strings, lengths);
  CLProgram* program = new CLProgram(context->c_obj, NULL, 0);

	for (int i=0; i<program->devices.size(); i++)
		program->SetSrc(program->devices[i], buf);
  free(buf);

  return &program->st_obj;
}

CL_API_ENTRY cl_program CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clCreateProgramWithBinary
#else
clCreateProgramWithBinary
#endif
                         (cl_context                     context,
                          cl_uint                        num_devices,
                          const cl_device_id *           device_list,
                          const size_t *                 lengths,
                          const unsigned char **         binaries,
                          cl_int *                       binary_status,
                          cl_int *                       errcode_ret) CL_API_SUFFIX__VERSION_1_0 {
  cl_int err = CL_SUCCESS;

  if (context == NULL) err = CL_INVALID_CONTEXT;
  else if (num_devices == 0 || device_list == NULL || lengths == NULL || binaries == NULL) err = CL_INVALID_VALUE;
	else {
		for (uint i = 0; i < num_devices; i++) {
			if (lengths[i] == 0 || binaries[i] == NULL) {
				err = CL_INVALID_VALUE;
        break;
			}
		}
	}

	SNUCL_DevicesVerification(context->c_obj->devices, context->c_obj->devices.size(), device_list, num_devices, err);
  
  if (g_platform.cluster) err = CL_INVALID_OPERATION;
  if (errcode_ret) *errcode_ret = err;

  if (err != CL_SUCCESS) return NULL;

  CLProgram* program = new CLProgram(context->c_obj, device_list, num_devices);

	for (uint i = 0; i < num_devices; i++) {
		CLBinary* bin = new CLBinary((char*)binaries[i], (size_t)lengths[i], CL_PROGRAM_BINARY_TYPE_NONE);
		bin->type = (binaries[i][0] == 'C')? CL_PROGRAM_BINARY_TYPE_COMPILED_OBJECT : CL_PROGRAM_BINARY_TYPE_EXECUTABLE;
		program->SetBin(device_list[i]->c_obj, bin, bin->type);
		program->createWithBinary[device_list[i]->c_obj] = true;
		device_list[i]->c_obj->EnqueueBuildProgram(program, NULL);
		if (binary_status) binary_status[i] = CL_SUCCESS;
	}

  return &program->st_obj;
}

CL_API_ENTRY cl_program CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clCreateProgramWithBuiltInKernels
#else
clCreateProgramWithBuiltInKernels
#endif
                                 (cl_context             context,
                                  cl_uint                num_devices,
                                  const cl_device_id *   device_list,
                                  const char *           kernel_names,
                                  cl_int *               errcode_ret) CL_API_SUFFIX__VERSION_1_2 {
  cl_int err = CL_SUCCESS;

  if (context == NULL) err = CL_INVALID_CONTEXT;
  else if (num_devices == 0 || device_list == NULL || kernel_names == NULL) err = CL_INVALID_VALUE;

	CheckBuiltInKernels(num_devices, device_list, kernel_names, &err);
	SNUCL_DevicesVerification(context->c_obj->devices, context->c_obj->devices.size(), device_list, num_devices, err);

  if (errcode_ret) *errcode_ret = err;

  if (err != CL_SUCCESS) return NULL;

  CLProgram* program = new CLProgram(context->c_obj, device_list, num_devices);
	// create program if the device supports any built-in kernel.

  return &program->st_obj;
}

CL_API_ENTRY cl_int CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clRetainProgram
#else
clRetainProgram
#endif
                 (cl_program program) CL_API_SUFFIX__VERSION_1_0 {
	if (program == NULL) return CL_INVALID_PROGRAM;

  program->c_obj->Retain();

  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clReleaseProgram
#else
clReleaseProgram
#endif
                 (cl_program program) CL_API_SUFFIX__VERSION_1_0 {
	if (program == NULL) return CL_INVALID_PROGRAM;
	CLProgram* p = program->c_obj;

	bool buildDone = true;
	for (uint i = 0; i < p->devices.size(); i++)
		if (p->buildStatus[p->devices[i]] == CL_BUILD_IN_PROGRESS)
			buildDone = false;

	if (p->Release() == 0 && p->kernelObjs.size() == 0 && buildDone) {
		for (uint i = 0; i < p->devices.size(); i++) {
			for (vector<CLProgram*>::iterator it = p->devices[i]->programs.begin(); it != p->devices[i]->programs.end(); it++) {
				if ((*it) == p) {
					p->devices[i]->programs.erase(it);
					break;
				}
			}
		}
		delete p;
	}

  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clBuildProgram
#else
clBuildProgram
#endif
              (cl_program           program,
               cl_uint              num_devices,
               const cl_device_id * device_list,
               const char *         options, 
               void (CL_CALLBACK * pfn_notify)(cl_program program, void * user_data),
               void *               user_data) CL_API_SUFFIX__VERSION_1_0 {
  if (program == NULL) return CL_INVALID_PROGRAM;
	CLProgram* p = program->c_obj;
  if ((device_list == NULL && num_devices > 0) || (device_list != NULL && num_devices == 0)) return CL_INVALID_VALUE;
  if (pfn_notify == NULL && user_data != NULL) return CL_INVALID_VALUE;

	vector<CLDevice*>* devices = &p->context->devices;
	cl_int err = CL_SUCCESS;

	SNUCL_DevicesVerification(*devices, devices->size(), device_list, num_devices, err);
	if (err != CL_SUCCESS) return err;

	p->CheckOptions(options, &err);
	if (err != CL_SUCCESS) return err;

	if (device_list) {
		for (uint i = 0; i < num_devices; i++) {
			if (p->fromBinary[device_list[i]->c_obj])  {
				if (p->bins[device_list[i]->c_obj]->binary == NULL) return CL_INVALID_BINARY;
			}
			else if (p->fromSource[device_list[i]->c_obj]) {
				if (device_list[i]->c_obj->compiler_available == CL_FALSE) return CL_COMPILER_NOT_AVAILABLE;
			}
			else {
				return CL_INVALID_OPERATION;
			}

		}

	} else {
		for (vector<CLDevice*>::iterator it = devices->begin(); it != devices->end(); ++it) {
			if (p->fromBinary[(*it)]) {
				if (p->bins[(*it)]->binary == NULL) return CL_INVALID_BINARY;
			}
			else if (p->fromSource[(*it)]) {
				if ((*it)->compiler_available == CL_FALSE) return CL_COMPILER_NOT_AVAILABLE;
			}
			else {
				return CL_INVALID_OPERATION;
			}
		}
	}

	if (p->kernelObjs.size() > 0) return CL_INVALID_OPERATION;

	if (pfn_notify) {
		ProgramCallback* pc = new ProgramCallback(pfn_notify, user_data);
		p->buildCallbackStack.push_back(pc);
	}

  if (device_list) {
    for (uint i = 0; i < num_devices; ++i) {
			if (p->fromBinary[device_list[i]->c_obj] && p->buildStatus.count(device_list[i]->c_obj) > 0) {
				if (p->buildStatus[device_list[i]->c_obj] == CL_BUILD_SUCCESS) continue;
      }
			p->buildStatus[device_list[i]->c_obj] = CL_BUILD_IN_PROGRESS;
      device_list[i]->c_obj->EnqueueBuildProgram(p, options);
			if(p->buildStatus[device_list[i]->c_obj] == CL_BUILD_ERROR)
				return CL_BUILD_PROGRAM_FAILURE;
    }
  } else {
    for (vector<CLDevice*>::iterator it = devices->begin(); it != devices->end(); ++it) {
			if (p->fromBinary[(*it)] && p->buildStatus.count((*it)) > 0) {
				if (p->buildStatus[(*it)] == CL_BUILD_SUCCESS) continue;
      }
			p->buildStatus[(*it)] = CL_BUILD_IN_PROGRESS;
      (*it)->EnqueueBuildProgram(p, options);
			if(p->buildStatus[(*it)] == CL_BUILD_ERROR)
				return CL_BUILD_PROGRAM_FAILURE;
    }
  }

  if (g_platform.cluster) {
    SNUCL_GetProgramInfo(p, options);
  }

  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clCompileProgram
#else
clCompileProgram
#endif
                (cl_program              program,
                 cl_uint                 num_devices,
                 const cl_device_id *    device_list,
                 const char *            options, 
                 cl_uint                 num_input_headers,
                 const cl_program *      input_headers,
                 const char **           header_include_names,
                 void (CL_CALLBACK *     pfn_notify)(cl_program /* program */, void * /* user_data */),
                 void *                  user_data) CL_API_SUFFIX__VERSION_1_2 {
  if (program == NULL) return CL_INVALID_PROGRAM;
  if ((device_list == NULL && num_devices > 0) || (device_list != NULL && num_devices == 0)) return CL_INVALID_VALUE;
	if (num_input_headers == 0 && (header_include_names != NULL || input_headers != NULL)) return CL_INVALID_VALUE;
	if (num_input_headers != 0 && (header_include_names == NULL || input_headers == NULL)) return CL_INVALID_VALUE;
	if (pfn_notify == NULL && user_data != NULL) return CL_INVALID_VALUE;

	CLProgram* p = program->c_obj;
	cl_int err = CL_SUCCESS;

	SNUCL_DevicesVerification(p->context->devices, p->context->devices.size(), device_list, num_devices, err);
  if (g_platform.cluster) err = CL_INVALID_OPERATION;
	if (err != CL_SUCCESS) return err;

	p->CheckOptions(options, &err);
	if (err != CL_SUCCESS) return err;

	if (p->kernelObjs.size() > 0) return CL_INVALID_OPERATION;
	for (uint i = 0; i < num_devices; ++i) {
		if (!p->fromSource[device_list[i]->c_obj]) return CL_INVALID_OPERATION;
	}

	if (pfn_notify) {
		ProgramCallback* pc = new ProgramCallback(pfn_notify, user_data);
		p->compileCallbackStack.push_back(pc);
	}

	char* newOptions = NULL;
	if (num_input_headers) newOptions = GenHeaders(num_input_headers, input_headers, header_include_names, options);
	else if (options) {
		newOptions = (char*)calloc(strlen(options) + 1, sizeof(char));
		strncpy(newOptions, options, strlen(options));
	}

  if (device_list) {
    for (uint i = 0; i < num_devices; ++i) {
			p->buildStatus[device_list[i]->c_obj] = CL_BUILD_IN_PROGRESS;
      device_list[i]->c_obj->EnqueueCompileProgram(p, newOptions);
    }
  } else {
    for (vector<CLDevice*>::iterator it = p->context->devices.begin(); it != p->context->devices.end(); ++it) {
			p->buildStatus[(*it)] = CL_BUILD_IN_PROGRESS;
      (*it)->EnqueueCompileProgram(p, newOptions);
    }
  }
	if (newOptions != NULL) free(newOptions);

	return CL_SUCCESS;
}

CL_API_ENTRY cl_program CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clLinkProgram
#else
clLinkProgram
#endif
             (cl_context              context,
              cl_uint                 num_devices,
              const cl_device_id *    device_list,
              const char *            options, 
              cl_uint                 num_input_programs,
              const cl_program *      input_programs,
              void (CL_CALLBACK *     pfn_notify)(cl_program /* program */, void * /* user_data */),
              void *                  user_data,
              cl_int *                errcode_ret) CL_API_SUFFIX__VERSION_1_2 {
	cl_int err = CL_SUCCESS;
  if (context == NULL) err = CL_INVALID_CONTEXT;
  if ((device_list == NULL && num_devices > 0) || (device_list != NULL && num_devices == 0)) err = CL_INVALID_VALUE;
	if (num_input_programs == 0 || input_programs == NULL) err = CL_INVALID_VALUE;

	for (uint i=0; i<num_input_programs; i++) {
		if (input_programs[i] == NULL) {
			err = CL_INVALID_PROGRAM;
			break;
		}
	}
	if (pfn_notify == NULL && user_data != NULL) err = CL_INVALID_VALUE;

	if (errcode_ret) *errcode_ret = err;
	if (err != CL_SUCCESS) return NULL;

	CLContext* c = context->c_obj;

	SNUCL_DevicesVerification(c->devices, c->devices.size(), device_list, num_devices, err);
  if (g_platform.cluster) err = CL_INVALID_OPERATION;
	if (errcode_ret) *errcode_ret = err;
	if (err != CL_SUCCESS) return NULL;

	if (errcode_ret) *errcode_ret = err;
	if (err != CL_SUCCESS) return NULL;

	bool performLink = false;
	for (uint i=0; i<num_input_programs; i++) {
		if (device_list) {
			for (cl_uint j = 0; j < num_devices; ++j) {
				if (input_programs[i]->c_obj->compiledObject.count(device_list[j]->c_obj) > 0 || input_programs[i]->c_obj->linkObjects.count(device_list[j]->c_obj) > 0)
					performLink = true;
				else if(performLink) {
					err = CL_INVALID_OPERATION;
					if (errcode_ret) *errcode_ret = err;
					if (err != CL_SUCCESS) return NULL;
				}
			}
		}
		else {
			for (vector<CLDevice*>::iterator it = c->devices.begin(); it != c->devices.end(); ++it) {
				if (input_programs[i]->c_obj->compiledObject.count(*it) > 0 || input_programs[i]->c_obj->linkObjects.count(*it) > 0)
					performLink = true;
				else if(performLink) {
					err = CL_INVALID_OPERATION;
					if (errcode_ret) *errcode_ret = err;
					if (err != CL_SUCCESS) return NULL;
				}
			}
		}
	}

  CLProgram* p = new CLProgram(c, device_list, num_devices);

	char* newOptions = NULL;
	if (options) {
		newOptions = (char*)calloc(strlen(options) + 1, sizeof(char));
		p->CheckLinkOptions(newOptions, options, &err);
	}


	if (pfn_notify) {
		ProgramCallback* pc = new ProgramCallback(pfn_notify, user_data);
		p->linkCallbackStack.push_back(pc);
	}

	if (performLink) {
		if (device_list) {
			for (uint i = 0; i < num_devices; ++i) {
				for (uint j=0; j<num_input_programs; j++) {
					if (input_programs[j]->c_obj->compiledObject.count(device_list[i]->c_obj) > 0) {
						p->linkObjects[device_list[i]->c_obj].push_back(input_programs[j]->c_obj->compiledObject[device_list[i]->c_obj]);
					}
					else { // (input_programs[i]->c_obj->linkObjects.count(device_list[j]->c_obj) > 0)
						for (uint k=0; k<input_programs[j]->c_obj->linkObjects[device_list[i]->c_obj].size(); k++)
							p->linkObjects[device_list[i]->c_obj].push_back(input_programs[j]->c_obj->linkObjects[device_list[i]->c_obj][k]);
					}
				}
				p->buildStatus[device_list[i]->c_obj] = CL_BUILD_IN_PROGRESS;
				device_list[i]->c_obj->EnqueueLinkProgram(p, newOptions);
			}
		}
		else {
			for (vector<CLDevice*>::iterator it = c->devices.begin(); it != c->devices.end(); ++it) {
				for (uint j=0; j<num_input_programs; j++) {
					if (input_programs[j]->c_obj->compiledObject.count(*it) > 0) {
						p->linkObjects[(*it)].push_back(input_programs[j]->c_obj->compiledObject[(*it)]);
					}
					else { // (input_programs[i]->c_obj->linkObjects.count(*it)) > 0)
						for (uint k=0; k<input_programs[j]->c_obj->linkObjects[(*it)].size(); k++)
							p->linkObjects[(*it)].push_back(input_programs[j]->c_obj->linkObjects[(*it)][k]);
					}
				}
				p->buildStatus[(*it)] = CL_BUILD_IN_PROGRESS;
				(*it)->EnqueueLinkProgram(p, newOptions);
			}
		}
	}

	if (newOptions != NULL) free(newOptions);

	return &p->st_obj;
}


CL_API_ENTRY cl_int CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clUnloadPlatformCompiler
#else
clUnloadPlatformCompiler
#endif
                (cl_platform_id    platform) CL_API_SUFFIX__VERSION_1_2 {
  if (platform != &g_platform.st_obj) return CL_INVALID_PLATFORM;
  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clGetProgramInfo
#else
clGetProgramInfo
#endif
                (cl_program         program,
                 cl_program_info    param_name,
                 size_t             param_value_size,
                 void *             param_value,
                 size_t *           param_value_size_ret) CL_API_SUFFIX__VERSION_1_0 {
  if (program == NULL) return CL_INVALID_PROGRAM;
	CLProgram* p = program->c_obj;

	if (param_name == CL_PROGRAM_NUM_KERNELS || param_name == CL_PROGRAM_KERNEL_NAMES) {
		cl_int err = CL_INVALID_PROGRAM_EXECUTABLE;
		for(map<CLDevice*, cl_build_status>::iterator it = p->buildStatus.begin(); it != p->buildStatus.end(); ++it) {
			if (it->second == CL_BUILD_SUCCESS) err = CL_SUCCESS;
			break;
		}
		if (err != CL_SUCCESS) return err;
	}

	int srcLen = (p->src) ? strlen(p->src) + 1: 0;

  cl_context context = &p->context->st_obj;
  cl_uint dev_size = p->devices.size();

  switch (param_name) {
    SNUCL_GetObjectInfo(CL_PROGRAM_REFERENCE_COUNT, cl_uint, p->ref_cnt);
    SNUCL_GetObjectInfo(CL_PROGRAM_CONTEXT, cl_context, context);
    SNUCL_GetObjectInfo(CL_PROGRAM_NUM_DEVICES, cl_uint, dev_size);
		case CL_PROGRAM_DEVICES: {
			size_t size = sizeof(cl_device_id) * p->devices.size();
			if (p->devices.size() == 0) {
				if (param_value_size_ret) *param_value_size_ret = 0;
				break;
			}
			if (param_value) {
				if (param_value_size < size) return CL_INVALID_VALUE;
				int idx=0;
				for (vector<CLDevice*>::iterator it = p->devices.begin(); it != p->devices.end(); ++it)  {
					((cl_device_id*)param_value)[idx++] = &((*it)->st_obj);
				}
			}
			if (param_value_size_ret) *param_value_size_ret = size;
			break;
		}

    SNUCL_GetObjectInfoA(CL_PROGRAM_SOURCE, char, p->src, srcLen);
		case CL_PROGRAM_BINARY_SIZES: {
			size_t size = sizeof(size_t) * p->bins.size();
			if (p->bins.size() == 0) {
				if (param_value_size_ret) *param_value_size_ret = 0;
				break;
			}
			if (param_value) {
				if (param_value_size < size) return CL_INVALID_VALUE;
				int idx=0;
				for (map<CLDevice*, CLBinary*>::iterator it = p->bins.begin(); it != p->bins.end(); ++it) 
					((size_t*)param_value)[idx++] = it->second->size;
			}
			if (param_value_size_ret) *param_value_size_ret = size;
			break;
		}
		case CL_PROGRAM_BINARIES: {
			size_t size = sizeof(unsigned char*) * p->bins.size();
			if (p->bins.size() == 0) {
				if (param_value_size_ret) *param_value_size_ret = 0;
				break;
			}

			if (param_value) {
				if (param_value_size < size) return CL_INVALID_VALUE;
				int idx=0;
				for (map<CLDevice*, CLBinary*>::iterator it = p->bins.begin(); it != p->bins.end(); ++it) {
          if (((char**)param_value)[idx]) memcpy((void*)(((char**)param_value)[idx]), (void*)it->second->binary, it->second->size);
          idx++;
				}
			}
			if (param_value_size_ret) *param_value_size_ret = size;
			break;
		}
    SNUCL_GetObjectInfo(CL_PROGRAM_NUM_KERNELS, size_t, p->num_kernels);
		case CL_PROGRAM_KERNEL_NAMES: {
			size_t size = 0;
			for (uint i = 0; i < p->num_kernels; i++) {
				size += strlen(p->kernel_names[i]);
				size++; //for ';' delimiter or the null-terminate character
			}
			if (size == 0) {
				if (param_value_size_ret) *param_value_size_ret = 0;
				break;
			}
			if (param_value) {
				if (param_value_size < size) return CL_INVALID_VALUE;
				size_t len = 0;
				for (uint i=0; i<p->num_kernels; i++) {
					strncpy( &(((char*)param_value)[len]), p->kernel_names[i], strlen(p->kernel_names[i]));
					len += strlen(p->kernel_names[i]);
					((char*)param_value)[len] = ';';
					len++;
				}
				((char*)param_value)[len-1] = '\0';
			}
			if (param_value_size_ret) *param_value_size_ret = size;
			break;
		}
    default: return CL_INVALID_VALUE;
  }

  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clGetProgramBuildInfo
#else
clGetProgramBuildInfo
#endif
                     (cl_program            program,
                      cl_device_id          device,
                      cl_program_build_info param_name,
                      size_t                param_value_size,
                      void *                param_value,
                      size_t *              param_value_size_ret) CL_API_SUFFIX__VERSION_1_0 {
  if (program == NULL) return CL_INVALID_PROGRAM;
	CLProgram* p = program->c_obj;

	cl_int err = CL_SUCCESS;
	SNUCL_DevicesVerification(p->context->devices, p->context->devices.size(), &device, 1, err);
	if (err != CL_SUCCESS) return err;

	int optLen = 0;
	if(p->options)
		optLen = strlen(p->options);

  switch (param_name) {
		case CL_PROGRAM_BUILD_STATUS: {                                            
			CLDevice* d = device->c_obj;
			size_t size = sizeof(cl_build_status);                            
			if (param_value) {                                     

				if (p->buildStatus[d] == CL_BUILD_NONE) {
					if (d->parent_device != NULL) // sub device
						d = d->parent_device->c_obj;
					else if (d->subDevices.size() > 0) {//parent device
						for (int i=0; i<d->subDevices.size(); i++) {
							if (p->buildStatus[d->subDevices[i]] != CL_BUILD_NONE) {
								d = d->subDevices[i];
								break;
							}

						}
					}
				}

				if (param_value_size < size) return CL_INVALID_VALUE;
				memcpy(param_value, &(p->buildStatus[d]), size);                 
			}                                                      
			if (param_value_size_ret) *param_value_size_ret = size;
			break;                                                 
		}
    SNUCL_GetObjectInfoA(CL_PROGRAM_BUILD_OPTIONS, char, p->options, optLen);
		case CL_PROGRAM_BUILD_LOG: {
			CLDevice* d = device->c_obj;

			if (p->buildLog.count(d) <= 0) {
				if (d->parent_device != NULL) // sub device
					d = d->parent_device->c_obj;
				else if (d->subDevices.size() > 0) {//parent device
					for (int i=0; i<d->subDevices.size(); i++) {
						if (p->buildLog.count(d->subDevices[i]) > 0) {
							d = d->subDevices[i];
							break;
						}
					}
				}
			}

			size_t size =  (p->buildLog.count(d) > 0)? strlen(p->buildLog[d])+1 : 0;
			if (size == 0) {
				if (param_value_size_ret) *param_value_size_ret = 0;
				break;
			}
			if (param_value) {
				if (param_value_size < size) return CL_INVALID_VALUE;
				memcpy(param_value, p->buildLog[d], strlen(p->buildLog[d])+1);
			}
			if (param_value_size_ret) *param_value_size_ret = size;
			break;
		}
		case CL_PROGRAM_BINARY_TYPE: {                                            
			CLDevice* d = device->c_obj;
			size_t size = sizeof(cl_program_binary_type);                            
			if (param_value) {

				if (p->bins.count(d) <= 0) {
					if (d->parent_device != NULL) // sub device
						d = d->parent_device->c_obj;
					else if (d->subDevices.size() > 0) {//parent device
						for (int i=0; i<d->subDevices.size(); i++) {
							if (p->bins.count(d->subDevices[i]) > 0) {
								d = d->subDevices[i];
								break;
							}
						}
					}
				}

				if (param_value_size < size) return CL_INVALID_VALUE;
				memcpy(param_value, &(p->bins[d]->type), size);                 
			}                                                      
			if (param_value_size_ret) *param_value_size_ret = size;
			break;                                                 
		}
    default: return CL_INVALID_VALUE;
  }

  return CL_SUCCESS;
}
                            
/* Kernel Object APIs */
CL_API_ENTRY cl_kernel CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clCreateKernel
#else
clCreateKernel
#endif
              (cl_program      program,
               const char *    kernel_name,
               cl_int *        errcode_ret) CL_API_SUFFIX__VERSION_1_0 {
  cl_int err = CL_SUCCESS;
	int kernel_idx = -1;
	CLProgram* p = program->c_obj;
  
  if (program == NULL) err = CL_INVALID_PROGRAM;

  else if (kernel_name == NULL) err = CL_INVALID_VALUE;

	else {
		for(uint i=0; i<p->num_kernels; i++) {
			if(strcmp(kernel_name, p->kernel_names[i]) == 0) {
				kernel_idx = i;
				break;
			}
		}
		if(kernel_idx == -1) err = CL_INVALID_KERNEL_NAME;
	}

  if (errcode_ret) *errcode_ret = err;

  if (err != CL_SUCCESS) return NULL;
  

	err = CL_INVALID_PROGRAM_EXECUTABLE;
	for(map<CLDevice*, cl_build_status>::iterator it = p->buildStatus.begin(); it != p->buildStatus.end(); ++it) {
		if (it->second == CL_BUILD_SUCCESS) {
      err = CL_SUCCESS;
      break;
    }
	}

  if (errcode_ret) *errcode_ret = err;

  if (err != CL_SUCCESS) return NULL;


  CLKernel* kernel = new CLKernel(p, kernel_name);
	kernel->kernel_idx = kernel_idx;
	p->kernelObjs.push_back(kernel);

  return &kernel->st_obj;
}

CL_API_ENTRY cl_int CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clCreateKernelsInProgram
#else
clCreateKernelsInProgram
#endif
                        (cl_program     program,
                         cl_uint        num_kernels,
                         cl_kernel *    kernels,
                         cl_uint *      num_kernels_ret) CL_API_SUFFIX__VERSION_1_0 {

  if (program == NULL) return CL_INVALID_PROGRAM;
	CLProgram* p = program->c_obj;

	cl_int err = CL_INVALID_PROGRAM_EXECUTABLE;
	for(map<CLDevice*, cl_build_status>::iterator it = p->buildStatus.begin(); it != p->buildStatus.end(); ++it) {
		if (it->second == CL_BUILD_SUCCESS) {
      err = CL_SUCCESS;
      break;
    }
	}

  if (err != CL_SUCCESS) return err;

	if(kernels != NULL && num_kernels < p->num_kernels) return CL_INVALID_VALUE;

	for(uint i=0; i<p->num_kernels; i++) {
		CLKernel* kernel = new CLKernel(p, p->kernel_names[i]);
		kernel->kernel_idx = i;
		p->kernelObjs.push_back(kernel);

		if(kernels)
			kernels[i] = &kernel->st_obj;
	}

	if(num_kernels_ret) *num_kernels_ret = p->num_kernels;

  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clRetainKernel
#else
clRetainKernel
#endif
                 (cl_kernel kernel) CL_API_SUFFIX__VERSION_1_0 {
	if(!kernel) return CL_INVALID_KERNEL;

  kernel->c_obj->Retain();

  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clReleaseKernel
#else
clReleaseKernel
#endif
                 (cl_kernel kernel) CL_API_SUFFIX__VERSION_1_0 {
	if(!kernel) return CL_INVALID_KERNEL;
	CLKernel* k = kernel->c_obj;
	
	bool allCompleted = true;
	if (k->Release() == 0) {
		for (uint i=0; i<k->commands.size(); i++) {
			if (k->commands[i]->event->status != CL_COMPLETE) {
				allCompleted = false;
				break;
			}
		}
		if(allCompleted) {
			for (vector<CLKernel*>::iterator it = k->program->kernelObjs.begin(); it != k->program->kernelObjs.end(); it++) {
				if ((*it) == k) {
					k->program->kernelObjs.erase(it);
					break;
				}
			}
			if (k->program->ref_cnt == 0 && k->program->kernelObjs.size() == 0)
				clReleaseProgram(&k->program->st_obj);
			delete k; 
		}
	}

  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clSetKernelArg
#else
clSetKernelArg
#endif
              (cl_kernel    kernel,
               cl_uint      arg_index,
               size_t       arg_size,
               const void * arg_value) CL_API_SUFFIX__VERSION_1_0 {
	if(!kernel) return CL_INVALID_KERNEL;
  return kernel->c_obj->SetArg(arg_index, arg_size, arg_value);
}

CL_API_ENTRY cl_int CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clGetKernelInfo
#else
clGetKernelInfo
#endif
               (cl_kernel       kernel,
                cl_kernel_info  param_name,
                size_t          param_value_size,
                void *          param_value,
                size_t *        param_value_size_ret) CL_API_SUFFIX__VERSION_1_0 {
	if(kernel == NULL) return CL_INVALID_KERNEL;
	CLKernel* k = kernel->c_obj;

  switch (param_name) {
    SNUCL_GetObjectInfoA(CL_KERNEL_FUNCTION_NAME, char, k->name, strlen(k->name) + 1);
    SNUCL_GetObjectInfoV(CL_KERNEL_NUM_ARGS, cl_uint, k->program->kernel_num_args[k->kernel_idx]);
    SNUCL_GetObjectInfo(CL_KERNEL_REFERENCE_COUNT, cl_uint, k->ref_cnt);
    SNUCL_GetObjectInfoV(CL_KERNEL_CONTEXT, cl_context, &k->program->context->st_obj);
    SNUCL_GetObjectInfoV(CL_KERNEL_PROGRAM, cl_program, &k->program->st_obj);
    SNUCL_GetObjectInfoA(CL_KERNEL_ATTRIBUTES, char, k->program->kernel_attributes[k->kernel_idx], strlen(k->program->kernel_attributes[k->kernel_idx]) + 1);
    default: return CL_INVALID_VALUE;
  }
  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clGetKernelArgInfo
#else
clGetKernelArgInfo
#endif
                  (cl_kernel              kernel,
                   cl_uint                arg_indx,
                   cl_kernel_arg_info     param_name,
                   size_t                 param_value_size,
                   void *                 param_value,
                   size_t *               param_value_size_ret) CL_API_SUFFIX__VERSION_1_2 {
	if (kernel == NULL) return CL_INVALID_KERNEL;
  CLKernel* k = kernel->c_obj;

	if (arg_indx >= k->program->kernel_num_args[k->kernel_idx]) return CL_INVALID_ARG_INDEX;

	if (!k->program->optArgInfo) return CL_KERNEL_ARG_INFO_NOT_AVAILABLE;

  switch (param_name) {
    SNUCL_GetObjectInfo(CL_KERNEL_ARG_ADDRESS_QUALIFIER, cl_kernel_arg_address_qualifier, k->program->kernel_arg_address_qualifier[k->kernel_idx][arg_indx]);
    SNUCL_GetObjectInfo(CL_KERNEL_ARG_ACCESS_QUALIFIER, cl_kernel_arg_access_qualifier, k->program->kernel_arg_access_qualifier[k->kernel_idx][arg_indx]);
    SNUCL_GetObjectInfoA(CL_KERNEL_ARG_TYPE_NAME, char, k->program->kernel_arg_type_name[k->kernel_idx][arg_indx], strlen(k->program->kernel_arg_type_name[k->kernel_idx][arg_indx]));
    SNUCL_GetObjectInfo(CL_KERNEL_ARG_TYPE_QUALIFIER, cl_kernel_arg_type_qualifier, k->program->kernel_arg_type_qualifier[k->kernel_idx][arg_indx]);
    SNUCL_GetObjectInfoA(CL_KERNEL_ARG_NAME, char, k->program->kernel_arg_name[k->kernel_idx][arg_indx], strlen(k->program->kernel_arg_name[k->kernel_idx][arg_indx]) );
    default: return CL_INVALID_VALUE;
  }
	return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clGetKernelWorkGroupInfo
#else
clGetKernelWorkGroupInfo
#endif
                        (cl_kernel                  kernel,
                         cl_device_id               device,
                         cl_kernel_work_group_info  param_name,
                         size_t                     param_value_size,
                         void *                     param_value,
                         size_t *                   param_value_size_ret) CL_API_SUFFIX__VERSION_1_0 {
	if(kernel == NULL) return CL_INVALID_KERNEL;
  CLKernel* k = kernel->c_obj;
  CLDevice* d;

	if (device == NULL && k->program->context->devices.size() != 1) return CL_INVALID_DEVICE;

  cl_device_id dev = device ? device : &k->program->context->devices[0]->st_obj;

	cl_int err = CL_SUCCESS;
	SNUCL_DevicesVerification(k->program->context->devices, k->program->context->devices.size(), &dev, 1, err);
	if (err != CL_SUCCESS) return err;

	d = dev->c_obj;

	if(param_name == CL_KERNEL_GLOBAL_WORK_SIZE && ( d->type != CL_DEVICE_TYPE_CUSTOM || !k->program->isBuiltInKernel[k->kernel_idx]))
		return CL_INVALID_VALUE;
	

  map<cl_uint, CLKernelArg*>* args = &k->args;
  cl_ulong local_mem_size = 0;
  for (map<cl_uint, CLKernelArg*>::iterator it = args->begin(); it != args->end(); ++it) {
    CLKernelArg* arg = it->second;
    if (arg->local) local_mem_size += arg->size;
  }

  switch (param_name) {
    SNUCL_GetObjectInfoA(CL_KERNEL_GLOBAL_WORK_SIZE, size_t, k->global_work_size, 3);
    SNUCL_GetObjectInfo(CL_KERNEL_WORK_GROUP_SIZE, size_t, d->max_work_group_size);
    SNUCL_GetObjectInfoA(CL_KERNEL_COMPILE_WORK_GROUP_SIZE, size_t, k->program->kernel_reqd_work_group_size[k->kernel_idx], 3);
    SNUCL_GetObjectInfo(CL_KERNEL_LOCAL_MEM_SIZE, cl_ulong, local_mem_size);
    SNUCL_GetObjectInfo(CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE, size_t, d->max_work_group_size);
    SNUCL_GetObjectInfoV(CL_KERNEL_PRIVATE_MEM_SIZE, cl_ulong, 0);
    default: return CL_INVALID_VALUE;
  }
  return CL_SUCCESS;
}


/*Event Object APIs */
CL_API_ENTRY cl_int CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clWaitForEvents
#else
clWaitForEvents
#endif
               (cl_uint           num_events,
                const cl_event *  event_list) CL_API_SUFFIX__VERSION_1_0 {

  if (num_events == 0 || event_list == NULL) return CL_INVALID_VALUE;

  for (uint i = 1; i < num_events; ++i) {
		if (event_list[i] == NULL)
			return CL_INVALID_EVENT;
		if (event_list[i]->c_obj->context != event_list[0]->c_obj->context )
			return CL_INVALID_CONTEXT;
	}

	cl_int ret = CL_SUCCESS;
  for (uint i = 0; i < num_events; ++i) {
    CLEvent* event = event_list[i]->c_obj;
    if (event->Wait() < 0) ret = CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
  }

  return ret;
}

CL_API_ENTRY cl_int CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clGetEventInfo
#else
clGetEventInfo
#endif
              (cl_event         event,
               cl_event_info    param_name,
               size_t           param_value_size,
               void *           param_value,
               size_t *         param_value_size_ret) CL_API_SUFFIX__VERSION_1_0 {
	if(event == NULL) return CL_INVALID_EVENT;

	CLEvent* e = event->c_obj;

  switch (param_name) {
    SNUCL_GetObjectInfo(CL_EVENT_COMMAND_QUEUE, cl_command_queue, e->queue);
    SNUCL_GetObjectInfo(CL_EVENT_CONTEXT, cl_context, e->context);
    SNUCL_GetObjectInfo(CL_EVENT_COMMAND_TYPE, cl_command_type, e->type);
    SNUCL_GetObjectInfoV(CL_EVENT_COMMAND_EXECUTION_STATUS, cl_int, e->status);
    SNUCL_GetObjectInfo(CL_EVENT_REFERENCE_COUNT, cl_uint, e->ref_cnt);
    default: return CL_INVALID_VALUE;
  }
  return CL_SUCCESS;
}

CL_API_ENTRY cl_event CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clCreateUserEvent
#else
clCreateUserEvent
#endif 
                 (cl_context       context,
                  cl_int *         errcode_ret) CL_API_SUFFIX__VERSION_1_1 {
	cl_int err = CL_SUCCESS;
	if(!context) err = CL_INVALID_CONTEXT;

  if (errcode_ret) *errcode_ret = err;

  if (err != CL_SUCCESS) return NULL;

	CLEvent* event = new CLEvent(context, CL_COMMAND_USER, NULL, NULL);
	event->user = true;
	event->disclosed = true;
	event->SetStatus(CL_SUBMITTED);

  return &event->st_obj;
}
                            
CL_API_ENTRY cl_int CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clRetainEvent
#else
clRetainEvent
#endif
               (cl_event event) CL_API_SUFFIX__VERSION_1_0 {
	if(!event) return CL_INVALID_EVENT;

  event->c_obj->Retain();

  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clReleaseEvent
#else
clReleaseEvent
#endif
             (cl_event event) CL_API_SUFFIX__VERSION_1_0 {
	if(!event) return CL_INVALID_EVENT;

  //TODO
	if(event->c_obj->type == CL_COMMAND_USER && event->c_obj->Release() == 0)
		delete event->c_obj;

  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clSetUserEventStatus
#else
clSetUserEventStatus
#endif
             (cl_event      event,
              cl_int        execution_status) CL_API_SUFFIX__VERSION_1_1 {
	if(event == NULL) return CL_INVALID_EVENT;

	CLEvent* e = event->c_obj;

	if(!e->user) return CL_INVALID_EVENT;

	if(execution_status != CL_COMPLETE && execution_status >= 0) return CL_INVALID_VALUE;

	if(e->userSetStatus) return CL_INVALID_OPERATION;

	e->userSetStatus = true;
	e->SetStatus(execution_status);

  return CL_SUCCESS;

}
CL_API_ENTRY cl_int CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clSetEventCallback
#else
clSetEventCallback
#endif
                   (cl_event           event,
                    cl_int             command_exec_callback_type,
                    void (CL_CALLBACK *pfn_notify)(cl_event, cl_int, void *),
                    void *             user_data) CL_API_SUFFIX__VERSION_1_1 {
	if(event == NULL) return CL_INVALID_EVENT;
	if(pfn_notify == NULL || (command_exec_callback_type != CL_SUBMITTED && command_exec_callback_type != CL_RUNNING && command_exec_callback_type != CL_COMPLETE)) return CL_INVALID_VALUE;

	event->c_obj->AddCallback(pfn_notify, user_data, command_exec_callback_type);

	return CL_SUCCESS;
}

/* Profiling APIs */
CL_API_ENTRY cl_int CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clGetEventProfilingInfo
#else
clGetEventProfilingInfo
#endif
                       (cl_event            event,
                        cl_profiling_info   param_name,
                        size_t              param_value_size,
                        void *              param_value,
                        size_t *            param_value_size_ret) CL_API_SUFFIX__VERSION_1_0 {
	if(event == NULL) return CL_INVALID_EVENT;

  CLEvent* e = event->c_obj;

	if (!e->queue) return CL_PROFILING_INFO_NOT_AVAILABLE;
	if (!e->queue->c_obj->IsProfiled()) return CL_PROFILING_INFO_NOT_AVAILABLE;
	if (e->status != CL_COMPLETE || e->user) return CL_PROFILING_INFO_NOT_AVAILABLE;

  switch (param_name) {
    SNUCL_GetObjectInfo(CL_PROFILING_COMMAND_QUEUED, cl_ulong, e->profiling_info[0]);
    SNUCL_GetObjectInfo(CL_PROFILING_COMMAND_SUBMIT, cl_ulong, e->profiling_info[1]);
    SNUCL_GetObjectInfo(CL_PROFILING_COMMAND_START, cl_ulong, e->profiling_info[2]);
    SNUCL_GetObjectInfo(CL_PROFILING_COMMAND_END, cl_ulong, e->profiling_info[3]);
    default: return CL_INVALID_VALUE;
  }
  return CL_SUCCESS;
}
                                
/* Flush and Finish APIs */
CL_API_ENTRY cl_int CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clFlush
#else
clFlush
#endif
           (cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0 {
  if (command_queue == NULL) return CL_INVALID_COMMAND_QUEUE;

  //TODO
  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clFinish
#else
clFinish
#endif
           (cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0 {
  if (command_queue == NULL) return CL_INVALID_COMMAND_QUEUE;
  CLCommandQueue* cmq = command_queue->c_obj;

  CLCommand* command = new CLCommand(cmq, 0, NULL, CL_COMMAND_MARKER);
  command->DisclosedToUser();
  cmq->Enqueue(command);
  command->event->Wait();

  return CL_SUCCESS;
}

//TODO:change cb -> size
//TODO new Command -> CommandFactory
/* Enqueued Commands APIs */
CL_API_ENTRY cl_int CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clEnqueueReadBuffer
#else
clEnqueueReadBuffer
#endif
                   (cl_command_queue    command_queue,
                    cl_mem              buffer,
                    cl_bool             blocking_read,
                    size_t              offset,
                    size_t              cb, 
                    void *              ptr,
                    cl_uint             num_events_in_wait_list,
                    const cl_event *    event_wait_list,
                    cl_event *          event) CL_API_SUFFIX__VERSION_1_0 {
  if (command_queue == NULL) return CL_INVALID_COMMAND_QUEUE;
  if (buffer == NULL) return CL_INVALID_MEM_OBJECT;
  CLCommandQueue* cmq = command_queue->c_obj;

	if (cmq->context != buffer->c_obj->context) return CL_INVALID_CONTEXT;

  for (uint i = 0; i < num_events_in_wait_list; ++i) {
		if (!event_wait_list[i])
			return CL_INVALID_EVENT_WAIT_LIST;

		if (event_wait_list[i]->c_obj->context->c_obj != cmq->context )
			return CL_INVALID_CONTEXT;
	}

	CLMem* b = buffer->c_obj;
	if (cb + offset > b->size || offset > b->size || ptr == NULL) return CL_INVALID_VALUE;

  if ((event_wait_list == NULL && num_events_in_wait_list > 0) || (event_wait_list != NULL && num_events_in_wait_list == 0)) return CL_INVALID_EVENT_WAIT_LIST;

	if (b->flags & (CL_MEM_HOST_WRITE_ONLY | CL_MEM_HOST_NO_ACCESS))
		return CL_INVALID_OPERATION;

  //TODO::SetCommand
  CLCommand* command = new CLCommand(cmq, num_events_in_wait_list, event_wait_list, CL_COMMAND_READ_BUFFER);
  command->mem_src = buffer->c_obj;
	command->mem_src->SetCommand(command);
  command->off_src = offset;
  command->cb = cb;
  command->ptr = ptr;

  if (event) *event = command->DisclosedToUser();
  if (g_platform.cluster) command->DisclosedToUser();

  cmq->Enqueue(command);

  if (blocking_read == CL_TRUE) 
		if(command->event->Wait() < 0)
			return CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;

  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clEnqueueReadBufferRect
#else
clEnqueueReadBufferRect
#endif
                       (cl_command_queue    command_queue,
                        cl_mem              buffer,
                        cl_bool             blocking_read,
                        const size_t *      buffer_origin,
                        const size_t *      host_origin, 
                        const size_t *      region,
                        size_t              buffer_row_pitch,
                        size_t              buffer_slice_pitch,
                        size_t              host_row_pitch,
                        size_t              host_slice_pitch,
                        void *              ptr,
                        cl_uint             num_events_in_wait_list,
                        const cl_event *    event_wait_list,
                        cl_event *          event) CL_API_SUFFIX__VERSION_1_1
{
  if (command_queue == NULL) return CL_INVALID_COMMAND_QUEUE;
  if (buffer == NULL) return CL_INVALID_MEM_OBJECT;
  if (command_queue->c_obj->context != buffer->c_obj->context)
    return CL_INVALID_CONTEXT;
  if ((event_wait_list == NULL && num_events_in_wait_list > 0) || 
      (event_wait_list != NULL && num_events_in_wait_list == 0)) 
    return CL_INVALID_EVENT_WAIT_LIST;
  if (buffer->c_obj->context != command_queue->c_obj->context) 
    return CL_INVALID_CONTEXT;
  if (ptr == NULL) return CL_INVALID_VALUE;
  for (int i = 0; i < 3; ++i) {
    if (region[i] == 0) return CL_INVALID_VALUE;
  }
  if (buffer_row_pitch != 0)
    if (buffer_row_pitch < region[0]) return CL_INVALID_VALUE;
  if (host_row_pitch != 0)
    if (host_row_pitch < region[0]) return CL_INVALID_VALUE;
  if (buffer_slice_pitch != 0)
    if (buffer_slice_pitch < region[1]*buffer_row_pitch)
      if ((buffer_slice_pitch % buffer_row_pitch) != 0)
        return CL_INVALID_VALUE;
  if (host_slice_pitch != 0)
    if (host_slice_pitch < region[1]*host_row_pitch)
      if ((host_slice_pitch % host_row_pitch) != 0)
        return CL_INVALID_VALUE;
  for (int i = 0; i < num_events_in_wait_list; ++i) {
    if (command_queue->c_obj->context != event_wait_list[i]->c_obj->context->c_obj) 
      return CL_INVALID_CONTEXT;
  }
  if (buffer->c_obj->is_sub) {
//    CLCommandQueue *cmq = command_queue->c_obj;
//    cmq->device->CheckBuffer(buffer->c_obj);
//    size_t dev_mem = (size_t)buffer->c_obj->dev_specific[cmq->device];
//    if ((dev_mem % cmq->device->mem_base_addr_align) != 0) { 
//      return CL_MISALIGNED_SUB_BUFFER_OFFSET;
//    }    
  }
	if (buffer->c_obj->flags & (CL_MEM_HOST_WRITE_ONLY | CL_MEM_HOST_NO_ACCESS))
		return CL_INVALID_OPERATION;

  CLCommandQueue* cmq = command_queue->c_obj;
  CLCommand* command = new CLCommand(cmq, num_events_in_wait_list, event_wait_list, CL_COMMAND_READ_BUFFER_RECT);
  command->buffer = buffer->c_obj;
	command->buffer->SetCommand(command);
  command->src_origin[0] = buffer_origin[0];
  command->src_origin[1] = buffer_origin[1];
  command->src_origin[2] = buffer_origin[2];
  command->dst_origin[0] = host_origin[0];
  command->dst_origin[1] = host_origin[1];
  command->dst_origin[2] = host_origin[2];
  command->region[0] = region[0];
  command->region[1] = region[1];
  command->region[2] = region[2];
  command->src_row_pitch = buffer_row_pitch;
  command->src_slice_pitch = buffer_slice_pitch;
  command->dst_row_pitch = host_row_pitch;
  command->dst_slice_pitch = host_slice_pitch;
  command->ptr = ptr;

  if (event) *event = command->DisclosedToUser();
  if (g_platform.cluster) command->DisclosedToUser();

  cmq->Enqueue(command);

  if (blocking_read == CL_TRUE) 
		if(command->event->Wait() < 0)
			return CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;

  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clEnqueueWriteBuffer
#else
clEnqueueWriteBuffer
#endif
                    (cl_command_queue   command_queue, 
                     cl_mem             buffer, 
                     cl_bool            blocking_write, 
                     size_t             offset, 
                     size_t             cb, 
                     const void *       ptr, 
                     cl_uint            num_events_in_wait_list, 
                     const cl_event *   event_wait_list, 
                     cl_event *         event) CL_API_SUFFIX__VERSION_1_0 {
  if (command_queue == NULL) return CL_INVALID_COMMAND_QUEUE;
  CLCommandQueue* cmq = command_queue->c_obj;
  if (buffer == NULL) return CL_INVALID_MEM_OBJECT;

	if (cmq->context != buffer->c_obj->context) return CL_INVALID_CONTEXT;

  for (uint i = 0; i < num_events_in_wait_list; ++i) {
		if (!event_wait_list[i])
			return CL_INVALID_EVENT_WAIT_LIST;

		if (event_wait_list[i]->c_obj->context->c_obj != cmq->context )
			return CL_INVALID_CONTEXT;
	}

	CLMem* b = buffer->c_obj;
	if (cb + offset > b->size || offset > b->size || ptr == NULL) return CL_INVALID_VALUE;

  if ((event_wait_list == NULL && num_events_in_wait_list > 0) || (event_wait_list != NULL && num_events_in_wait_list == 0)) return CL_INVALID_EVENT_WAIT_LIST;

	if (b->flags & (CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_NO_ACCESS))
		return CL_INVALID_OPERATION;

  CLCommand* command = new CLCommand(cmq, num_events_in_wait_list, event_wait_list, CL_COMMAND_WRITE_BUFFER);
  command->mem_dst = buffer->c_obj;
	command->mem_dst->SetCommand(command);
  command->off_dst = offset;
  command->cb = cb;
  command->ptr = (void*) ptr;

  if (event) *event = command->DisclosedToUser();
  if (g_platform.cluster) command->DisclosedToUser();

  cmq->Enqueue(command);

  if (blocking_write == CL_TRUE) 
		if(command->event->Wait() < 0)
			return CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;

  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clEnqueueWriteBufferRect
#else
clEnqueueWriteBufferRect
#endif
                       (cl_command_queue    command_queue,
                        cl_mem              buffer,
                        cl_bool             blocking_read,
                        const size_t *      buffer_origin,
                        const size_t *      host_origin, 
                        const size_t *      region,
                        size_t              buffer_row_pitch,
                        size_t              buffer_slice_pitch,
                        size_t              host_row_pitch,
                        size_t              host_slice_pitch,
                        const void *        ptr,
                        cl_uint             num_events_in_wait_list,
                        const cl_event *    event_wait_list,
                        cl_event *          event) CL_API_SUFFIX__VERSION_1_1
{
  if (command_queue == NULL) return CL_INVALID_COMMAND_QUEUE;
  if (buffer == NULL) return CL_INVALID_MEM_OBJECT;
  if ((event_wait_list == NULL && num_events_in_wait_list > 0) || 
      (event_wait_list != NULL && num_events_in_wait_list == 0)) 
    return CL_INVALID_EVENT_WAIT_LIST;
  if (buffer->c_obj->context != command_queue->c_obj->context) 
    return CL_INVALID_CONTEXT;
  for (uint i = 0; i < num_events_in_wait_list; ++i) {
		if (!event_wait_list[i])
			return CL_INVALID_EVENT_WAIT_LIST;

		if (event_wait_list[i]->c_obj->context->c_obj != command_queue->c_obj->context )
			return CL_INVALID_CONTEXT;
	}
	if (buffer->c_obj->flags & (CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_NO_ACCESS))
		return CL_INVALID_OPERATION;
  if (buffer->c_obj->is_sub) {
//    CLCommandQueue *cmq = command_queue->c_obj;
//    cmq->device->CheckBuffer(buffer->c_obj);
//    size_t dev_mem = (size_t)buffer->c_obj->dev_specific[cmq->device];
//    if ((dev_mem % cmq->device->mem_base_addr_align) != 0) {
//      return CL_MISALIGNED_SUB_BUFFER_OFFSET;
//    }
  }

  CLCommandQueue* cmq = command_queue->c_obj;
  CLCommand* command = new CLCommand(cmq, num_events_in_wait_list, event_wait_list, CL_COMMAND_WRITE_BUFFER_RECT);
  command->buffer = buffer->c_obj;
	command->buffer->SetCommand(command);
  command->src_origin[0] = host_origin[0];
  command->src_origin[1] = host_origin[1];
  command->src_origin[2] = host_origin[2];
  command->dst_origin[0] = buffer_origin[0];
  command->dst_origin[1] = buffer_origin[1];
  command->dst_origin[2] = buffer_origin[2];
  command->region[0] = region[0];
  command->region[1] = region[1];
  command->region[2] = region[2];
  command->src_row_pitch = host_row_pitch;
  command->src_slice_pitch = host_slice_pitch;
  command->dst_row_pitch = buffer_row_pitch;
  command->dst_slice_pitch = buffer_slice_pitch;
  command->ptr = (void*)ptr;

  if (event) *event = command->DisclosedToUser();
  if (g_platform.cluster) command->DisclosedToUser();

  cmq->Enqueue(command);

  if (blocking_read == CL_TRUE) 
		if(command->event->Wait() < 0)
			return CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;

  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clEnqueueFillBuffer
#else
clEnqueueFillBuffer
#endif
                   (cl_command_queue   command_queue,
                    cl_mem             buffer, 
                    const void *       pattern, 
                    size_t             pattern_size, 
                    size_t             offset, 
                    size_t             size, 
                    cl_uint            num_events_in_wait_list, 
                    const cl_event *   event_wait_list, 
                    cl_event *         event) CL_API_SUFFIX__VERSION_1_2
{
  if (command_queue == NULL) return CL_INVALID_COMMAND_QUEUE;
  if (buffer == NULL) return CL_INVALID_MEM_OBJECT;
  if ((event_wait_list == NULL && num_events_in_wait_list > 0) || 
      (event_wait_list != NULL && num_events_in_wait_list == 0)) 
    return CL_INVALID_EVENT_WAIT_LIST;
  if (buffer->c_obj->context != command_queue->c_obj->context) 
    return CL_INVALID_CONTEXT;
  if (pattern == NULL) return CL_INVALID_MEM_OBJECT;
  if (pattern_size == 0) return CL_INVALID_MEM_OBJECT;
  if (pattern_size != 1  && pattern_size != 2   && pattern_size != 4  &&
      pattern_size != 8  && pattern_size != 16  && pattern_size != 32 &&
      pattern_size != 64 && pattern_size != 128) {
    return CL_INVALID_VALUE;
  }
  if (((offset+size)%pattern_size) != 0) return CL_INVALID_VALUE;
  if (offset >= buffer->c_obj->size) return CL_INVALID_VALUE;
  if (offset+size > buffer->c_obj->size) return CL_INVALID_VALUE;
  if (g_platform.cluster) return CL_INVALID_OPERATION;

  CLCommandQueue* cmq = command_queue->c_obj;
  CLCommand* command = new CLCommand(cmq, num_events_in_wait_list, event_wait_list, CL_COMMAND_FILL_BUFFER);
  command->buffer = buffer->c_obj;
	command->buffer->SetCommand(command);
  command->pattern = malloc(pattern_size);
  memcpy(command->pattern, pattern, pattern_size);
  command->pattern_size = pattern_size;
  command->offset = offset;
  command->size = size;

  if (event) *event = command->DisclosedToUser();
  if (g_platform.cluster) command->DisclosedToUser();

  cmq->Enqueue(command);

  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clEnqueueCopyBuffer
#else
clEnqueueCopyBuffer
#endif
                   (cl_command_queue    command_queue, 
                    cl_mem              src_buffer,
                    cl_mem              dst_buffer, 
                    size_t              src_offset,
                    size_t              dst_offset,
                    size_t              cb, 
                    cl_uint             num_events_in_wait_list,
                    const cl_event *    event_wait_list,
                    cl_event *          event) CL_API_SUFFIX__VERSION_1_0 {
  if (command_queue == NULL) return CL_INVALID_COMMAND_QUEUE;
  if (src_buffer == NULL || dst_buffer == NULL) return CL_INVALID_MEM_OBJECT;
  CLCommandQueue* cmq = command_queue->c_obj;

	if (cmq->context != src_buffer->c_obj->context || cmq->context != dst_buffer->c_obj->context) return CL_INVALID_CONTEXT;

  for (uint i = 0; i < num_events_in_wait_list; ++i) {
		if (!event_wait_list[i])
			return CL_INVALID_EVENT_WAIT_LIST;

		if (event_wait_list[i]->c_obj->context->c_obj != cmq->context )
			return CL_INVALID_CONTEXT;
	}

	CLMem* sb = src_buffer->c_obj;
	CLMem* db = dst_buffer->c_obj;
	if (cb + src_offset > sb->size || cb + dst_offset > db->size || src_offset > sb->size || dst_offset > db->size || cb == 0) return CL_INVALID_VALUE;

  if ((event_wait_list == NULL && num_events_in_wait_list > 0) || (event_wait_list != NULL && num_events_in_wait_list == 0)) return CL_INVALID_EVENT_WAIT_LIST;


  if (sb == db) 
    if ((src_offset <= dst_offset && dst_offset <= src_offset + cb -1 ) || (dst_offset <= src_offset && src_offset <= dst_offset + cb - 1) ) return CL_MEM_COPY_OVERLAP;

  CLCommand* command = new CLCommand(cmq, num_events_in_wait_list, event_wait_list, CL_COMMAND_COPY_BUFFER);
  command->mem_src = src_buffer->c_obj;
	command->mem_src->SetCommand(command);
  command->mem_dst = dst_buffer->c_obj;
	command->mem_dst->SetCommand(command);
  command->off_src = src_offset;
  command->off_dst = dst_offset;
  command->cb = cb;

  if (event) *event = command->DisclosedToUser();
  if (g_platform.cluster) command->DisclosedToUser();

  cmq->Enqueue(command);

  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clEnqueueCopyBufferRect
#else
clEnqueueCopyBufferRect
#endif
                   (cl_command_queue    command_queue, 
                    cl_mem              src_buffer,
                    cl_mem              dst_buffer, 
                    const size_t *      src_origin,
                    const size_t *      dst_origin,
                    const size_t *      region, 
                    size_t              src_row_pitch,
                    size_t              src_slice_pitch,
                    size_t              dst_row_pitch,
                    size_t              dst_slice_pitch,
                    cl_uint             num_events_in_wait_list,
                    const cl_event *    event_wait_list,
                    cl_event *          event) CL_API_SUFFIX__VERSION_1_1
{
  if (command_queue == NULL) return CL_INVALID_COMMAND_QUEUE;
  if (src_buffer == NULL || dst_buffer == NULL) return CL_INVALID_MEM_OBJECT;
  if ((event_wait_list == NULL && num_events_in_wait_list > 0) ||
      (event_wait_list != NULL && num_events_in_wait_list == 0))
    return CL_INVALID_EVENT_WAIT_LIST;
  if ((src_buffer->c_obj->context != command_queue->c_obj->context) || 
      (dst_buffer->c_obj->context != command_queue->c_obj->context))
    return CL_INVALID_CONTEXT;
  // Error handling for overflow: CL_INVALID_VALUE
  if (src_buffer == dst_buffer) {
    // Error handling for CL_MEM_COPY_OVERLAP
  }

  CLCommandQueue* cmq = command_queue->c_obj;
  CLCommand* command = new CLCommand(cmq, num_events_in_wait_list, event_wait_list, CL_COMMAND_COPY_BUFFER_RECT);
  command->mem_src = src_buffer->c_obj;
	command->mem_src->SetCommand(command);
  command->mem_dst = dst_buffer->c_obj;
	command->mem_dst->SetCommand(command);
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

  if (event) *event = command->DisclosedToUser();
  if (g_platform.cluster) command->DisclosedToUser();

  cmq->Enqueue(command);

  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clEnqueueReadImage
#else
clEnqueueReadImage
#endif
                  (cl_command_queue     command_queue,
                   cl_mem               image,
                   cl_bool              blocking_read, 
                   const size_t *       origin,
                   const size_t *       region,
                   size_t               row_pitch,
                   size_t               slice_pitch, 
                   void *               ptr,
                   cl_uint              num_events_in_wait_list,
                   const cl_event *     event_wait_list,
                   cl_event *           event) CL_API_SUFFIX__VERSION_1_0 {
  if (command_queue == NULL) return CL_INVALID_COMMAND_QUEUE;
  if (image == NULL) return CL_INVALID_MEM_OBJECT;
  if ((event_wait_list == NULL && num_events_in_wait_list > 0) ||
      (event_wait_list != NULL && num_events_in_wait_list == 0))
    return CL_INVALID_EVENT_WAIT_LIST;
  if (image->c_obj->context != command_queue->c_obj->context) 
    return CL_INVALID_CONTEXT;
	if (image->c_obj->flags & (CL_MEM_HOST_WRITE_ONLY | CL_MEM_HOST_NO_ACCESS))
		return CL_INVALID_OPERATION;

  CLCommandQueue* cmq = command_queue->c_obj;
  CLCommand* command = new CLCommand(cmq, num_events_in_wait_list, event_wait_list, CL_COMMAND_READ_IMAGE);
  command->mem_src = image->c_obj;
	command->mem_src->SetCommand(command);
  command->src_origin[0] = origin[0];
  command->src_origin[1] = origin[1];
  command->src_origin[2] = origin[2];
  command->region[0] = region[0];
  command->region[1] = region[1];
  command->region[2] = region[2];
  command->dst_row_pitch = row_pitch;
  command->dst_slice_pitch = slice_pitch;
  command->ptr = ptr;

  if (event) *event = command->DisclosedToUser();
  if (g_platform.cluster) command->DisclosedToUser();

  cmq->Enqueue(command);

  if (blocking_read == CL_TRUE) command->event->Wait();

  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clEnqueueWriteImage
#else
clEnqueueWriteImage
#endif
                   (cl_command_queue    command_queue,
                    cl_mem              image,
                    cl_bool             blocking_write, 
                    const size_t *      origin,
                    const size_t *      region,
                    size_t              input_row_pitch,
                    size_t              input_slice_pitch, 
                    const void *        ptr,
                    cl_uint             num_events_in_wait_list,
                    const cl_event *    event_wait_list,
                    cl_event *          event) CL_API_SUFFIX__VERSION_1_0 {
  if (command_queue == NULL) return CL_INVALID_COMMAND_QUEUE;
  if (image == NULL) return CL_INVALID_MEM_OBJECT;
  if ((event_wait_list == NULL && num_events_in_wait_list > 0) ||
      (event_wait_list != NULL && num_events_in_wait_list == 0))
    return CL_INVALID_EVENT_WAIT_LIST;
  if (image->c_obj->context != command_queue->c_obj->context) 
    return CL_INVALID_CONTEXT;
	if (image->c_obj->flags & (CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_NO_ACCESS))
		return CL_INVALID_OPERATION;

  CLCommandQueue* cmq = command_queue->c_obj;
  CLCommand* command = new CLCommand(cmq, num_events_in_wait_list, event_wait_list, CL_COMMAND_WRITE_IMAGE);
  command->mem_dst = image->c_obj;
	command->mem_dst->SetCommand(command);
  command->src_origin[0] = 0;
  command->src_origin[1] = 0;
  command->src_origin[2] = 0;
  command->dst_origin[0] = origin[0];
  command->dst_origin[1] = origin[1];
  command->dst_origin[2] = origin[2];
  command->region[0] = region[0];
  command->region[1] = region[1];
  command->region[2] = region[2];
  command->src_row_pitch = input_row_pitch;
  command->src_slice_pitch = input_slice_pitch;
  command->ptr = (void*)ptr;

  if (event) *event = command->DisclosedToUser();
  if (g_platform.cluster) command->DisclosedToUser();

  cmq->Enqueue(command);

  if (blocking_write== CL_TRUE) command->event->Wait();

  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clEnqueueFillImage
#else
clEnqueueFillImage
#endif
                  (cl_command_queue   command_queue,
                   cl_mem             image, 
                   const void *       fill_color, 
                   const size_t *     origin, 
                   const size_t *     region, 
                   cl_uint            num_events_in_wait_list, 
                   const cl_event *   event_wait_list, 
                   cl_event *         event) CL_API_SUFFIX__VERSION_1_2
{
  if (command_queue == NULL) return CL_INVALID_COMMAND_QUEUE;
  if (image == NULL) return CL_INVALID_MEM_OBJECT;
  if ((event_wait_list == NULL && num_events_in_wait_list > 0) || 
      (event_wait_list != NULL && num_events_in_wait_list == 0)) 
    return CL_INVALID_EVENT_WAIT_LIST;
  if (image->c_obj->context != command_queue->c_obj->context) 
    return CL_INVALID_CONTEXT;
  if (g_platform.cluster) return CL_INVALID_OPERATION;

  CLCommandQueue* cmq = command_queue->c_obj;
  CLCommand* command = new CLCommand(cmq, num_events_in_wait_list, event_wait_list, CL_COMMAND_FILL_IMAGE);
  const int CHANNELS = 4;
  size_t pattern_size = 0;
  cl_channel_type image_type = image->c_obj->image_format.image_channel_data_type;
  if(image_type==CL_SIGNED_INT8||image_type==CL_SIGNED_INT16||image_type==CL_SIGNED_INT32) {
    pattern_size = CHANNELS*sizeof(int);
  }
  else if(image_type==CL_UNSIGNED_INT8||image_type==CL_UNSIGNED_INT16||image_type==CL_UNSIGNED_INT32) {
    pattern_size = CHANNELS*sizeof(unsigned int);
  }
  else {
    pattern_size = CHANNELS*sizeof(float);
  }

  command->buffer = image->c_obj;
	command->buffer->SetCommand(command);
  command->pattern = malloc(pattern_size);
  memcpy(command->pattern, fill_color, pattern_size);
  command->pattern_size = pattern_size;
  command->dst_origin[0] = origin[0];
  command->dst_origin[1] = origin[1];
  command->dst_origin[2] = origin[2];
  command->region[0] = region[0];
  command->region[1] = region[1];
  command->region[2] = region[2];

  if (event) *event = command->DisclosedToUser();
  if (g_platform.cluster) command->DisclosedToUser();

  cmq->Enqueue(command);

  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clEnqueueCopyImage
#else
clEnqueueCopyImage
#endif
                  (cl_command_queue     command_queue,
                   cl_mem               src_image,
                   cl_mem               dst_image, 
                   const size_t *       src_origin,
                   const size_t *       dst_origin,
                   const size_t *       region, 
                   cl_uint              num_events_in_wait_list,
                   const cl_event *     event_wait_list,
                   cl_event *           event) CL_API_SUFFIX__VERSION_1_0 {
  if (command_queue == NULL) return CL_INVALID_COMMAND_QUEUE;
  if (src_image == NULL || dst_image == NULL) return CL_INVALID_MEM_OBJECT;
  CLMem *src_img = src_image->c_obj;
  CLMem *dst_img = dst_image->c_obj;
  if ((event_wait_list == NULL && num_events_in_wait_list > 0) ||
      (event_wait_list != NULL && num_events_in_wait_list == 0))
    return CL_INVALID_EVENT_WAIT_LIST;
  if ((src_img->context != command_queue->c_obj->context) || 
      (dst_img->context != command_queue->c_obj->context))
    return CL_INVALID_CONTEXT;

  // Error handling for overflow: CL_INVALID_VALUE
  if (src_img->image_desc.image_width < src_origin[0]) {
    return CL_INVALID_VALUE;
  }

  CLCommandQueue* cmq = command_queue->c_obj;
  CLCommand* command = new CLCommand(cmq, num_events_in_wait_list, event_wait_list, CL_COMMAND_COPY_IMAGE);
  command->mem_src = src_image->c_obj;
	command->mem_src->SetCommand(command);
  command->mem_dst = dst_image->c_obj;
	command->mem_dst->SetCommand(command);
  command->src_origin[0] = src_origin[0];
  command->src_origin[1] = src_origin[1];
  command->src_origin[2] = src_origin[2];
  command->dst_origin[0] = dst_origin[0];
  command->dst_origin[1] = dst_origin[1];
  command->dst_origin[2] = dst_origin[2];
  command->region[0] = region[0];
  command->region[1] = region[1];
  command->region[2] = region[2];

  if (event) *event = command->DisclosedToUser();
  if (g_platform.cluster) command->DisclosedToUser();

  cmq->Enqueue(command);

  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clEnqueueCopyImageToBuffer
#else
clEnqueueCopyImageToBuffer
#endif
                          (cl_command_queue command_queue,
                           cl_mem           src_image,
                           cl_mem           dst_buffer, 
                           const size_t *   src_origin,
                           const size_t *   region, 
                           size_t           dst_offset,
                           cl_uint          num_events_in_wait_list,
                           const cl_event * event_wait_list,
                           cl_event *       event) CL_API_SUFFIX__VERSION_1_0 {
  if (command_queue == NULL) return CL_INVALID_COMMAND_QUEUE;
  if (src_image == NULL || dst_buffer == NULL) return CL_INVALID_MEM_OBJECT;
  if ((event_wait_list == NULL && num_events_in_wait_list > 0) ||
      (event_wait_list != NULL && num_events_in_wait_list == 0))
    return CL_INVALID_EVENT_WAIT_LIST;
  if ((src_image->c_obj->context != command_queue->c_obj->context) || 
      (dst_buffer->c_obj->context != command_queue->c_obj->context))
    return CL_INVALID_CONTEXT;
  // Error handling for overflow: CL_INVALID_VALUE

  CLCommandQueue* cmq = command_queue->c_obj;
  CLCommand* command = new CLCommand(cmq, num_events_in_wait_list, event_wait_list, CL_COMMAND_COPY_IMAGE_TO_BUFFER);
  command->mem_src = src_image->c_obj;
	command->mem_src->SetCommand(command);
  command->mem_dst = dst_buffer->c_obj;
	command->mem_dst->SetCommand(command);
  command->src_origin[0] = src_origin[0];
  command->src_origin[1] = src_origin[1];
  command->src_origin[2] = src_origin[2];
  command->region[0] = region[0];
  command->region[1] = region[1];
  command->region[2] = region[2];
  command->off_dst = dst_offset;

  if (event) *event = command->DisclosedToUser();
  if (g_platform.cluster) command->DisclosedToUser();

  cmq->Enqueue(command);

  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clEnqueueCopyBufferToImage
#else
clEnqueueCopyBufferToImage
#endif
                          (cl_command_queue command_queue,
                           cl_mem           src_buffer,
                           cl_mem           dst_image, 
                           size_t           src_offset,
                           const size_t *   dst_origin,
                           const size_t *   region, 
                           cl_uint          num_events_in_wait_list,
                           const cl_event * event_wait_list,
                           cl_event *       event) CL_API_SUFFIX__VERSION_1_0 {
  if (command_queue == NULL) return CL_INVALID_COMMAND_QUEUE;
  if (src_buffer == NULL || dst_image == NULL) return CL_INVALID_MEM_OBJECT;
  if ((event_wait_list == NULL && num_events_in_wait_list > 0) ||
      (event_wait_list != NULL && num_events_in_wait_list == 0))
    return CL_INVALID_EVENT_WAIT_LIST;
  if ((src_buffer->c_obj->context != command_queue->c_obj->context) || 
      (dst_image->c_obj->context != command_queue->c_obj->context))
    return CL_INVALID_CONTEXT;
  // Error handling for overflow: CL_INVALID_VALUE

  CLCommandQueue* cmq = command_queue->c_obj;
  CLCommand* command = new CLCommand(cmq, num_events_in_wait_list, event_wait_list, CL_COMMAND_COPY_BUFFER_TO_IMAGE);
  command->mem_src = src_buffer->c_obj;
	command->mem_src->SetCommand(command);
  command->mem_dst = dst_image->c_obj;
	command->mem_dst->SetCommand(command);
  command->dst_origin[0] = dst_origin[0];
  command->dst_origin[1] = dst_origin[1];
  command->dst_origin[2] = dst_origin[2];
  command->region[0] = region[0];
  command->region[1] = region[1];
  command->region[2] = region[2];
  command->off_src = src_offset;

  if (event) *event = command->DisclosedToUser();
  if (g_platform.cluster) command->DisclosedToUser();

  cmq->Enqueue(command);

  return CL_SUCCESS;
}

CL_API_ENTRY void * CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clEnqueueMapBuffer
#else
clEnqueueMapBuffer
#endif
                  (cl_command_queue command_queue,
                   cl_mem           buffer,
                   cl_bool          blocking_map, 
                   cl_map_flags     map_flags,
                   size_t           offset,
                   size_t           size,
                   cl_uint          num_events_in_wait_list,
                   const cl_event * event_wait_list,
                   cl_event *       event,
                   cl_int *         errcode_ret) CL_API_SUFFIX__VERSION_1_0 {
  cl_int err = CL_SUCCESS;
  if (command_queue == NULL) err = CL_INVALID_COMMAND_QUEUE;
  if (buffer == NULL) err = CL_INVALID_MEM_OBJECT;
  if ((event_wait_list == NULL && num_events_in_wait_list > 0) ||
      (event_wait_list != NULL && num_events_in_wait_list == 0))
    err = CL_INVALID_EVENT_WAIT_LIST;
  if (buffer->c_obj->context != command_queue->c_obj->context) 
    err = CL_INVALID_CONTEXT;
  if (g_platform.cluster) err = CL_INVALID_OPERATION;
  if ((buffer->c_obj->flags & (CL_MEM_HOST_NO_ACCESS | CL_MEM_HOST_WRITE_ONLY)) && (map_flags & (CL_MAP_READ)))
      err = CL_INVALID_OPERATION;
  if ((buffer->c_obj->flags & (CL_MEM_HOST_NO_ACCESS | CL_MEM_HOST_READ_ONLY)) 
      && (map_flags & (CL_MAP_WRITE|CL_MAP_WRITE_INVALIDATE_REGION)))
      err = CL_INVALID_OPERATION;
  if (errcode_ret) *errcode_ret = err;
  if (err != CL_SUCCESS) return NULL;

  CLCommandQueue* cmq = command_queue->c_obj;
  CLCommand* command = new CLCommand(cmq, num_events_in_wait_list, event_wait_list, CL_COMMAND_MAP_BUFFER);
  command->buffer = buffer->c_obj;
	command->buffer->SetCommand(command);
  command->map_flags = map_flags;
  command->off_src = offset;
  command->cb = size;

  if(buffer->c_obj->use_host) {
    command->ptr = ((char*)buffer->c_obj->space_host)+offset;
  }
  else {
    if (cmq->device->type == CL_DEVICE_TYPE_CPU) {
      command->ptr = malloc(size);
    }
  }
  buffer->c_obj->map_count++;
  buffer->c_obj->mapped_info[command->ptr].size = size;
  buffer->c_obj->mapped_info[command->ptr].offset = offset;

  if (event) *event = command->DisclosedToUser();
  if (g_platform.cluster) command->DisclosedToUser();

  cmq->Enqueue(command);

  if (blocking_map == CL_TRUE || cmq->device->type == CL_DEVICE_TYPE_GPU) {
    command->event->Wait();
  }

  return command->ptr;
}

CL_API_ENTRY void * CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clEnqueueMapImage
#else
clEnqueueMapImage
#endif
                 (cl_command_queue  command_queue,
                  cl_mem            image, 
                  cl_bool           blocking_map, 
                  cl_map_flags      map_flags, 
                  const size_t *    origin,
                  const size_t *    region,
                  size_t *          image_row_pitch,
                  size_t *          image_slice_pitch,
                  cl_uint           num_events_in_wait_list,
                  const cl_event *  event_wait_list,
                  cl_event *        event,
                  cl_int *          errcode_ret) CL_API_SUFFIX__VERSION_1_0 {
  cl_int err = CL_SUCCESS;
  if (command_queue == NULL) err = CL_INVALID_COMMAND_QUEUE;
  if (image == NULL) err = CL_INVALID_MEM_OBJECT;
  if ((event_wait_list == NULL && num_events_in_wait_list > 0) ||
      (event_wait_list != NULL && num_events_in_wait_list == 0))
    err = CL_INVALID_EVENT_WAIT_LIST;
  if (image->c_obj->context != command_queue->c_obj->context) 
    err = CL_INVALID_CONTEXT;
  if (g_platform.cluster) err = CL_INVALID_OPERATION;
  if ((image->c_obj->flags & (CL_MEM_HOST_NO_ACCESS | CL_MEM_HOST_WRITE_ONLY)) && (map_flags & (CL_MAP_READ)))
      err = CL_INVALID_OPERATION;
  if ((image->c_obj->flags & (CL_MEM_HOST_NO_ACCESS | CL_MEM_HOST_READ_ONLY)) 
      && (map_flags & (CL_MAP_WRITE|CL_MAP_WRITE_INVALIDATE_REGION)))
      err = CL_INVALID_OPERATION;
  if (errcode_ret) *errcode_ret = err;
  if (err != CL_SUCCESS) return NULL;

  CLCommandQueue* cmq = command_queue->c_obj;
  CLCommand* command = new CLCommand(cmq, num_events_in_wait_list, event_wait_list, CL_COMMAND_MAP_IMAGE);
  
  CLMem* _image = (CLMem*)image->c_obj;
  cl_image_desc *image_desc = &_image->image_desc;
  size_t row_pitch = region[0] * _image->image_elem_size;
  size_t slice_pitch = row_pitch * region[1];
  size_t size = 0;
  if(region[2]>1 && region[1]>1) {
    size = region[2]*slice_pitch;
  }
  else if(region[2]==1 && region[1]>1) {
    size = region[1]*row_pitch;
  }
  else if(region[2]==1 && region[1]==1) {
    size = row_pitch;
  }
  else {
  }

  command->buffer = image->c_obj;
	command->buffer->SetCommand(command);
  command->map_flags = map_flags;
  command->src_origin[0] = origin[0];
  command->src_origin[1] = origin[1];
  command->src_origin[2] = origin[2];
  command->region[0] = region[0];
  command->region[1] = region[1];
  command->region[2] = region[2];
  command->dst_row_pitch = row_pitch;
  command->dst_slice_pitch = slice_pitch;
  if(image_row_pitch)
    *image_row_pitch = row_pitch;
  if(image_slice_pitch)
    *image_slice_pitch = slice_pitch;

  if (_image->flags & CL_MEM_USE_HOST_PTR) {
    if(_image->use_host)
    {
      size_t offset = 0;
      cl_image_desc image_desc = _image->image_desc;
      if(image_desc.image_type == CL_MEM_OBJECT_IMAGE3D ||
          image_desc.image_type == CL_MEM_OBJECT_IMAGE2D_ARRAY) {
        offset = origin[2]*_image->image_slice_pitch
          + origin[1]*_image->image_row_pitch
          + origin[0]*_image->image_elem_size;
      }
      else if(image_desc.image_type == CL_MEM_OBJECT_IMAGE2D) {
        offset = origin[1]*_image->image_row_pitch
          + origin[0]*_image->image_elem_size;
      }
      else if(image_desc.image_type == CL_MEM_OBJECT_IMAGE1D_ARRAY) {
        offset = origin[1]*_image->image_slice_pitch
          + origin[0]*_image->image_elem_size;
      }
      else {
        offset = origin[0]*_image->image_elem_size;
      }

      command->ptr = ((char*)_image->space_host)+offset;
    }
    else {
      command->ptr = malloc(size);
    }
  }
  else {
    if (cmq->device->type == CL_DEVICE_TYPE_CPU) {
      command->ptr = malloc(size);
    }
  }

  _image->map_count++;
  _image->mapped_info[command->ptr].origin[0] = origin[0];
  _image->mapped_info[command->ptr].origin[1] = origin[1];
  _image->mapped_info[command->ptr].origin[2] = origin[2];
  _image->mapped_info[command->ptr].region[0] = region[0];
  _image->mapped_info[command->ptr].region[1] = region[1];
  _image->mapped_info[command->ptr].region[2] = region[2];
  _image->mapped_info[command->ptr].row_pitch = row_pitch;
  _image->mapped_info[command->ptr].slice_pitch = slice_pitch;

  if (event) *event = command->DisclosedToUser();
  if (g_platform.cluster) command->DisclosedToUser();

  cmq->Enqueue(command);

  if (blocking_map == CL_TRUE || cmq->device->type == CL_DEVICE_TYPE_GPU) {
    command->event->Wait();
    if (cmq->device->type == CL_DEVICE_TYPE_GPU) {
      if (image_row_pitch) *image_row_pitch = command->dst_row_pitch;
      if (image_slice_pitch) *image_slice_pitch = command->dst_slice_pitch;
    }
  }

  return command->ptr;
}

CL_API_ENTRY cl_int CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clEnqueueUnmapMemObject
#else
clEnqueueUnmapMemObject
#endif
                       (cl_command_queue command_queue,
                        cl_mem           memobj,
                        void *           mapped_ptr,
                        cl_uint          num_events_in_wait_list,
                        const cl_event * event_wait_list,
                        cl_event *       event) CL_API_SUFFIX__VERSION_1_0 {
  if (command_queue == NULL) return CL_INVALID_COMMAND_QUEUE;
  if (memobj == NULL) return CL_INVALID_MEM_OBJECT;
  if ((event_wait_list == NULL && num_events_in_wait_list > 0) ||
      (event_wait_list != NULL && num_events_in_wait_list == 0))
    return CL_INVALID_EVENT_WAIT_LIST;
  if (memobj->c_obj->context != command_queue->c_obj->context) 
    return CL_INVALID_CONTEXT;

  CLCommandQueue* cmq = command_queue->c_obj;
  CLCommand* command = new CLCommand(cmq, num_events_in_wait_list, event_wait_list, CL_COMMAND_UNMAP_MEM_OBJECT);
  command->buffer = memobj->c_obj;
	command->buffer->SetCommand(command);
  command->ptr = mapped_ptr;
  command->cb = memobj->c_obj->mapped_info[mapped_ptr].size;
  command->off_dst = memobj->c_obj->mapped_info[mapped_ptr].offset;
  memobj->c_obj->map_count--;

  if (event) *event = command->DisclosedToUser();
  if (g_platform.cluster) command->DisclosedToUser();

  cmq->Enqueue(command);

  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clEnqueueMigrateMemObjects
#else
clEnqueueMigrateMemObjects
#endif
                          (cl_command_queue       command_queue,
                           cl_uint                num_mem_objects,
                           const cl_mem *         mem_objects,
                           cl_mem_migration_flags flags,
                           cl_uint                num_events_in_wait_list,
                           const cl_event *       event_wait_list,
                           cl_event *             event) CL_API_SUFFIX__VERSION_1_2
{
  if (command_queue == NULL) return CL_INVALID_COMMAND_QUEUE;
  if ((event_wait_list == NULL && num_events_in_wait_list > 0) ||
      (event_wait_list != NULL && num_events_in_wait_list == 0))
    return CL_INVALID_EVENT_WAIT_LIST;
  for(int i=0; i<num_mem_objects; ++i) {
    if (mem_objects[i]->c_obj->context != command_queue->c_obj->context) 
      return CL_INVALID_CONTEXT;
  }

  CLCommandQueue* cmq = command_queue->c_obj;
  CLCommand* command = new CLCommand(cmq, num_events_in_wait_list, event_wait_list, CL_COMMAND_MIGRATE_MEM_OBJECTS);
  command->num_mem_objects = num_mem_objects;
  command->mem_objects = (cl_mem*)malloc(sizeof(cl_mem)*num_mem_objects);
  memcpy(command->mem_objects, mem_objects, sizeof(cl_mem)*num_mem_objects);
  command->flags = flags;

  if (event) *event = command->DisclosedToUser();
  if (g_platform.cluster) command->DisclosedToUser();

  cmq->Enqueue(command);

  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clEnqueueNDRangeKernel
#else
clEnqueueNDRangeKernel
#endif
                      (cl_command_queue command_queue,
                       cl_kernel        kernel,
                       cl_uint          work_dim,
                       const size_t *   global_work_offset,
                       const size_t *   global_work_size,
                       const size_t *   local_work_size,
                       cl_uint          num_events_in_wait_list,
                       const cl_event * event_wait_list,
                       cl_event *       event) CL_API_SUFFIX__VERSION_1_0 {
  if (command_queue == NULL) return CL_INVALID_COMMAND_QUEUE;
  CLCommandQueue* cmq = command_queue->c_obj;

  if (!cmq->device->ContainsProgram(kernel->c_obj->program)) {
    return CL_INVALID_PROGRAM_EXECUTABLE;
  }

  if (kernel == NULL) return CL_INVALID_KERNEL;
  if (kernel->c_obj->program->context != cmq->context) return CL_INVALID_CONTEXT;

  for (uint i = 0; i < num_events_in_wait_list; ++i) {
		if (!event_wait_list[i])
			return CL_INVALID_EVENT_WAIT_LIST;

		if (event_wait_list[i]->c_obj->context->c_obj != cmq->context )
			return CL_INVALID_CONTEXT;
	}

  if (work_dim < 1 || work_dim > 3) return CL_INVALID_WORK_DIMENSION;

	if (global_work_size == NULL) return CL_INVALID_GLOBAL_WORK_SIZE;

	for (uint i = 0; i < work_dim; ++i) {
		if (global_work_size[i] > ULONG_MAX) return CL_INVALID_GLOBAL_WORK_SIZE;
	}

	if(global_work_offset) {
		for (uint i = 0; i < work_dim; ++i) {
			if (global_work_size[i] + global_work_offset[i] > ULONG_MAX)
				return CL_INVALID_GLOBAL_OFFSET;
		}
	}

  if (local_work_size) {
    for (uint i = 0; i < work_dim; ++i) {
      if (global_work_size[i] % local_work_size[i] > 0) return CL_INVALID_WORK_GROUP_SIZE;
    }

		size_t work_group_size = 1;
    for (uint i = 0; i < work_dim; ++i) {
			work_group_size *= local_work_size[i];
		}
		if (work_group_size > cmq->device->max_work_group_size) return CL_INVALID_WORK_GROUP_SIZE;

    for (uint i = 0; i < work_dim; ++i) {
			if (local_work_size[i] > cmq->device->max_work_item_sizes[i]) return CL_INVALID_WORK_ITEM_SIZE;
		}
  }

  if ((event_wait_list == NULL && num_events_in_wait_list > 0) || (event_wait_list != NULL && num_events_in_wait_list == 0)) return CL_INVALID_EVENT_WAIT_LIST;

  CLCommand* command = new CLCommand(cmq, num_events_in_wait_list, event_wait_list, CL_COMMAND_NDRANGE_KERNEL);
  command->kernel = kernel->c_obj;
	command->kernel->SetCommand(command);
  command->program = kernel->c_obj->program;
  command->work_dim = work_dim;

  for (uint i = 0; i < work_dim; ++i) {
    command->gwo[i] = global_work_offset ? global_work_offset[i] : 0;
    command->gws[i] = global_work_size[i];
    command->lws[i] = local_work_size ? local_work_size[i] : 1;
  }
  for (uint i = work_dim; i < 3; ++i) {
    command->gwo[i] = 0;
    command->gws[i] = 1;
    command->lws[i] = 1;
  }
  for (uint i = 0; i < 3; ++i) {
    command->nwg[i] = command->gws[i] / command->lws[i];
  }

  command->SetKernelArgs();

	if(!command->kernel_args) return CL_INVALID_KERNEL_ARGS;

  if (event) *event = command->DisclosedToUser();
#ifdef ALWAYS_NOTIFY_COMPLETE
  if (g_platform.cluster) command->DisclosedToUser();
#endif

  cmq->Enqueue(command);

  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clEnqueueTask
#else
clEnqueueTask
#endif
             (cl_command_queue  command_queue,
              cl_kernel         kernel,
              cl_uint           num_events_in_wait_list,
              const cl_event *  event_wait_list,
              cl_event *        event) CL_API_SUFFIX__VERSION_1_0 {
  size_t one = 1;
  return clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &one, &one, num_events_in_wait_list, event_wait_list, event);
}

CL_API_ENTRY cl_int CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clEnqueueNativeKernel
#else
clEnqueueNativeKernel
#endif
                     (cl_command_queue  command_queue,
          					  void (*user_func)(void *), 
                      void *            args,
                      size_t            cb_args, 
                      cl_uint           num_mem_objects,
                      const cl_mem *    mem_list,
                      const void **     args_mem_loc,
                      cl_uint           num_events_in_wait_list,
                      const cl_event *  event_wait_list,
                      cl_event *        event) CL_API_SUFFIX__VERSION_1_0 {
  if (command_queue == NULL) return CL_INVALID_COMMAND_QUEUE;
  if (user_func == NULL) return CL_INVALID_VALUE;
  if (args == NULL && (cb_args > 0 || num_mem_objects > 0)) return CL_INVALID_VALUE;
  if (args != NULL && cb_args == 0) return CL_INVALID_VALUE;
  if (num_mem_objects > 0 && (mem_list == NULL || args_mem_loc == NULL)) return CL_INVALID_VALUE;
  if (num_mem_objects == 0 && (mem_list != NULL || args_mem_loc != NULL)) return CL_INVALID_VALUE;
  for (int i = 0; i < num_mem_objects; ++i) if (mem_list[i] == NULL) return CL_INVALID_MEM_OBJECT;
  for (int i = 0; i < num_events_in_wait_list; ++i) {
    if (command_queue->c_obj->context != event_wait_list[i]->c_obj->context->c_obj) 
      return CL_INVALID_CONTEXT;
  }

  CLCommandQueue* cmq = command_queue->c_obj;
  CLCommand* command = CommandFactory::instance()->NewCommand(cmq, num_events_in_wait_list, event_wait_list, CL_COMMAND_NATIVE_KERNEL);
  command->user_func = user_func;
  command->ptr = args;
  command->kernel_args = new map<cl_uint, CLKernelArg*>();
  (*command->kernel_args)[0] = new CLKernelArg(cb_args, args);
  (*command->kernel_args)[1] = new CLKernelArg(num_mem_objects * sizeof(cl_mem), mem_list);
  (*command->kernel_args)[2] = new CLKernelArg(num_mem_objects * sizeof(void*), args_mem_loc);

  for (int i = 0; i < num_mem_objects; ++i)
    if (mem_list[i])
      mem_list[i]->c_obj->SetCommand(command);

  if (event) *event = command->DisclosedToUser();
#ifdef ALWAYS_NOTIFY_COMPLETE
  if (g_platform.cluster) command->DisclosedToUser();
#endif

  cmq->Enqueue(command);

  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clEnqueueMarkerWithWaitList
#else
clEnqueueMarkerWithWaitList
#endif
                           (cl_command_queue command_queue,
                            cl_uint          num_events_in_wait_list,
                            const cl_event * event_wait_list,
                            cl_event *       event) CL_API_SUFFIX__VERSION_1_2
{
  if (command_queue == NULL) return CL_INVALID_COMMAND_QUEUE;

  CLCommandQueue* cmq = command_queue->c_obj;
  CLCommand* command = new CLCommand(cmq, num_events_in_wait_list, event_wait_list, CL_COMMAND_MARKER);

  if (event) *event = command->DisclosedToUser();

  cmq->Enqueue(command);

  command->event->Wait();

  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clEnqueueBarrierWithWaitList
#else
clEnqueueBarrierWithWaitList
#endif
                            (cl_command_queue command_queue,
                             cl_uint          num_events_in_wait_list,
                             const cl_event * event_wait_list,
                             cl_event *       event) CL_API_SUFFIX__VERSION_1_2
{
  if (command_queue == NULL) return CL_INVALID_COMMAND_QUEUE;

  CLCommandQueue* cmq = command_queue->c_obj;
  CLCommand* command = new CLCommand(cmq, num_events_in_wait_list, event_wait_list, CL_COMMAND_BARRIER);

  if (event) *event = command->DisclosedToUser();

  cmq->Enqueue(command);

  command->event->Wait();

  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clSetCommandQueueProperty
#else
clSetCommandQueueProperty
#endif
                         (cl_command_queue              command_queue,
                          cl_command_queue_properties   properties, 
                          cl_bool                       enable,
                          cl_command_queue_properties * old_properties) CL_API_SUFFIX__VERSION_1_0 {
  return CL_SUCCESS;
}

CL_API_ENTRY cl_mem CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clCreateImage2D
#else
clCreateImage2D
#endif
               (cl_context              context,
                cl_mem_flags            flags,
                const cl_image_format * image_format,
                size_t                  image_width,
                size_t                  image_height,
                size_t                  image_row_pitch, 
                void *                  host_ptr,
                cl_int *                errcode_ret) CL_API_SUFFIX__VERSION_1_0 {
  cl_image_desc img_desc;
  img_desc.image_type = CL_MEM_OBJECT_IMAGE2D;
  img_desc.image_width = image_width;
  img_desc.image_height = image_height;
  img_desc.image_depth = 1;
  img_desc.image_array_size = 0;
  img_desc.image_row_pitch = image_row_pitch;
  img_desc.image_slice_pitch = 0;
  img_desc.buffer = 0;

  return clCreateImage(context, flags, image_format, &img_desc, host_ptr, errcode_ret);
}
                        
CL_API_ENTRY cl_mem CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clCreateImage3D
#else
clCreateImage3D
#endif
               (cl_context              context,
                cl_mem_flags            flags,
                const cl_image_format * image_format,
                size_t                  image_width, 
                size_t                  image_height,
                size_t                  image_depth, 
                size_t                  image_row_pitch, 
                size_t                  image_slice_pitch, 
                void *                  host_ptr,
                cl_int *                errcode_ret) CL_API_SUFFIX__VERSION_1_0 {
  cl_image_desc img_desc;
  img_desc.image_type = CL_MEM_OBJECT_IMAGE3D;
  img_desc.image_width = image_width;
  img_desc.image_height = image_height;
  img_desc.image_depth = image_depth;
  img_desc.image_array_size = 0;
  img_desc.image_row_pitch = image_row_pitch;
  img_desc.image_slice_pitch = image_slice_pitch;
  img_desc.buffer = 0;

  return clCreateImage(context, flags, image_format, &img_desc, host_ptr, errcode_ret);
}

CL_API_ENTRY cl_int CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clEnqueueMarker
#else
clEnqueueMarker
#endif
               (cl_command_queue    command_queue,
                cl_event *          event) CL_API_SUFFIX__VERSION_1_0 {
  if (command_queue == NULL) return CL_INVALID_COMMAND_QUEUE;
  if (event == NULL) return CL_INVALID_VALUE;

  CLCommandQueue* cmq = command_queue->c_obj;
  CLCommand* command = CommandFactory::instance()->NewCommand(cmq, 0, NULL, CL_COMMAND_MARKER);
  cmq->Enqueue(command);
  if (event) *event = command->DisclosedToUser();
  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clEnqueueWaitForEvents
#else
clEnqueueWaitForEvents
#endif
                      (cl_command_queue command_queue,
                       cl_uint          num_events,
                       const cl_event * event_list) CL_API_SUFFIX__VERSION_1_0 {
  if (command_queue == NULL) return CL_INVALID_COMMAND_QUEUE;
  if (num_events == 0 || event_list == NULL) return CL_INVALID_VALUE;

  CLCommandQueue* cmq = command_queue->c_obj;
  CLCommand* command = CommandFactory::instance()->NewCommand(cmq, num_events, event_list, CL_COMMAND_WAIT_FOR_EVENTS);
  cmq->Enqueue(command);
  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clEnqueueBarrier
#else
clEnqueueBarrier
#endif
                   (cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0 {
  if (command_queue == NULL) return CL_INVALID_COMMAND_QUEUE;

  CLCommandQueue* cmq = command_queue->c_obj;
  CLCommand* command = CommandFactory::instance()->NewCommand(cmq, 0, NULL, CL_COMMAND_BARRIER);
  cmq->Enqueue(command);
  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
#ifdef SNUCL_API_WRAP
__wrap_clUnloadCompiler
#else
clUnloadCompiler
#endif
                 (void) CL_API_SUFFIX__VERSION_1_0 {
  return CL_SUCCESS;
}


CL_API_ENTRY cl_int CL_API_CALL
clEnqueueBroadcastBuffer(cl_command_queue * command_queue_list,
                         cl_mem             src_buffer,
                         cl_uint            num_dst_buffers,
                         cl_mem *           dst_buffer_list,
                         size_t             src_offset,
                         size_t *           dst_offset_list,
                         size_t             cb,
                         cl_uint            num_events_in_wait_list,
                         const cl_event *   event_wait_list,
                         cl_event *         event_list) {
  CLCommand *p_command = CommandFactory::instance()->NewCommand(CL_COMMAND_BROADCAST_BUFFER);

  for (uint i = 0; i < num_dst_buffers; i++) {
    CLCommandQueue* command_queue = command_queue_list[i]->c_obj;
    CLCommand* command = CommandFactory::instance()->NewCommand(command_queue, num_events_in_wait_list, event_wait_list, CL_COMMAND_BROADCAST_BUFFER);
    command->mem_src = src_buffer->c_obj;
    command->mem_dst = dst_buffer_list[i]->c_obj;
    command->off_src = src_offset;
    command->off_dst = dst_offset_list ? dst_offset_list[i] : 0;
    command->cb = cb;
    command->p_command = p_command;
    if( event_list != NULL ) event_list[i] = &command->event->st_obj;
    command_queue->Enqueue(command);
  }

  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueAlltoAllBuffer(cl_command_queue * command_queue_list,
                        cl_uint            num_buffers,
                        cl_mem *           src_buffer_list,
                        cl_mem *           dst_buffer_list,
                        size_t *           src_offset_list,
                        size_t *           dst_offset_list,
                        size_t             cb,
                        cl_uint            num_events_in_wait_list,
                        const cl_event *   event_wait_list,
                        cl_event *         event_list) {
  CLCommand *p_command = CommandFactory::instance()->NewCommand(CL_COMMAND_ALLTOALL_BUFFER);

  for (uint i = 0; i < num_buffers; i++) {
    CLCommandQueue* command_queue = command_queue_list[i]->c_obj;
    CLCommand* command = CommandFactory::instance()->NewCommand(command_queue, num_events_in_wait_list, event_wait_list, CL_COMMAND_ALLTOALL_BUFFER);
    command->mem_src = src_buffer_list[i]->c_obj;
    command->mem_dst = dst_buffer_list[i]->c_obj;
    command->off_src = src_offset_list ? src_offset_list[i] : 0;
    command->off_dst = dst_offset_list ? dst_offset_list[i] : 0;
    command->cb = cb;
    command->p_command = p_command;
    if( event_list != NULL ) event_list[i] = &command->event->st_obj;
    command_queue->Enqueue(command);
  }

  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueScanBuffer(cl_command_queue * cmd_queue_list,
                    cl_uint num_buffers,
                    cl_mem * src_buffer_list,
                    cl_mem * dst_buffer_list,
                    size_t * src_offset,
                    size_t * dst_offset,
                    size_t cb,
                    cl_channel_type datatype,
                    cl_uint num_events_in_wait_list,
                    const cl_event * event_wait_list,
                    cl_event * event_list)
{
  size_t typesize = sizeof(float);
  if (datatype == CL_DOUBLE) sizeof(double);

  void** temp_buf = (void**) malloc(sizeof(void*) * num_buffers);
  for (int i = 0; i < num_buffers; i++) {
    temp_buf[i] = malloc(cb);
    clEnqueueReadBuffer(cmd_queue_list[i], src_buffer_list[i], CL_TRUE, src_offset ? src_offset[i] : 0, cb, temp_buf[i], num_events_in_wait_list, event_wait_list, NULL);
  }

  double sum_d[num_buffers];
  float  sum_f[num_buffers];
  unsigned int sum_u[num_buffers];
  int    sum_i[num_buffers];
  const void* sum_ptr[num_buffers];

  for (int i = 0; i < num_buffers; i++) {
    sum_d[i] = 0.0;
    sum_f[i] = 0.0f;
    sum_u[i] = 0;
    sum_i[i] = 0;
  }

  for (int i = 0; i < num_buffers; i++) {
    if (datatype == CL_DOUBLE) {
      double* t = (double*) temp_buf[i];
      for (int j = 0; j < cb / sizeof(double); j++) sum_d[j / (cb / sizeof(double)) / num_buffers] += t[j];
      sum_ptr[i] = (const void*) &sum_d[i];
    } else if (datatype == CL_FLOAT) {
      float* t = (float*) temp_buf[i];
      for (int j = 0; j < cb / sizeof(float); j++) sum_f[j / num_buffers] += t[j];
      sum_ptr[i] = (const void*) &sum_f[i];
    } else if (datatype == CL_UNSIGNED_INT32) {
      unsigned int* t = (unsigned int*) temp_buf[i];
      for (int j = 0; j < cb / sizeof(unsigned int); j++) sum_u[j / num_buffers] += t[j];
      sum_ptr[i] = (const void*) &sum_u[i];
    } else if (datatype == CL_SIGNED_INT32) {
      int* t = (int*) temp_buf[i];
      for (int j = 0; j < cb / sizeof(int); j++) sum_i[j / ((cb / sizeof(int)) / num_buffers)] += t[j];
      sum_ptr[i] = (const void*) &sum_i[i];
    }
  }

  for (int i = 1; i < num_buffers; i++) {
    if (datatype == CL_DOUBLE) {
      sum_d[i] += sum_d[i -1];
    } else if (datatype == CL_FLOAT) {
      sum_f[i] += sum_f[i -1];
    } else if (datatype == CL_UNSIGNED_INT32) {
      sum_u[i] += sum_u[i -1];
    } else if (datatype == CL_SIGNED_INT32) {
      sum_i[i] += sum_i[i -1];
    }
  }

  for (int i = 0; i < num_buffers; i++) {
    clEnqueueWriteBuffer(cmd_queue_list[i], dst_buffer_list[i], CL_TRUE, dst_offset ? dst_offset[i] : 0, typesize, sum_ptr[i], 0, NULL, event_list ? &event_list[i] : NULL);
  }

  for (int i = 0; i < num_buffers; i++) {
    free(temp_buf[i]);
  }
  free(temp_buf);

  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueReduceScatterBuffer(cl_command_queue * cmd_queue_list,
                             cl_uint num_buffers,
                             cl_mem * src_buffer_list,
                             cl_mem * dst_buffer_list,
                             size_t * src_offset,
                             size_t * dst_offset,
                             size_t cb,
                             cl_channel_type datatype,
                             cl_uint num_events_in_wait_list,
                             const cl_event * event_wait_list,
                             cl_event * event_list)
{
  size_t typesize = sizeof(float);
  if (datatype == CL_DOUBLE) sizeof(double);

  void** temp_buf = (void**) malloc(sizeof(void*) * num_buffers);
  for (int i = 0; i < num_buffers; i++) {
    temp_buf[i] = malloc(cb);
    clEnqueueReadBuffer(cmd_queue_list[i], src_buffer_list[i], CL_TRUE, src_offset ? src_offset[i] : 0, cb, temp_buf[i], num_events_in_wait_list, event_wait_list, NULL);
  }

  double sum_d[num_buffers];
  float  sum_f[num_buffers];
  unsigned int sum_u[num_buffers];
  int    sum_i[num_buffers];
  const void* sum_ptr[num_buffers];

  for (int i = 0; i < num_buffers; i++) {
    sum_d[i] = 0.0;
    sum_f[i] = 0.0f;
    sum_u[i] = 0;
    sum_i[i] = 0;
  }

  for (int i = 0; i < num_buffers; i++) {
    if (datatype == CL_DOUBLE) {
      double* t = (double*) temp_buf[i];
      for (int j = 0; j < cb / sizeof(double); j++) sum_d[j / (cb / sizeof(double)) / num_buffers] += t[j];
      sum_ptr[i] = (const void*) &sum_d[i];
    } else if (datatype == CL_FLOAT) {
      float* t = (float*) temp_buf[i];
      for (int j = 0; j < cb / sizeof(float); j++) sum_f[j / num_buffers] += t[j];
      sum_ptr[i] = (const void*) &sum_f[i];
    } else if (datatype == CL_UNSIGNED_INT32) {
      unsigned int* t = (unsigned int*) temp_buf[i];
      for (int j = 0; j < cb / sizeof(unsigned int); j++) sum_u[j / num_buffers] += t[j];
      sum_ptr[i] = (const void*) &sum_u[i];
    } else if (datatype == CL_SIGNED_INT32) {
      int* t = (int*) temp_buf[i];
      for (int j = 0; j < cb / sizeof(int); j++) sum_i[j / ((cb / sizeof(int)) / num_buffers)] += t[j];
      sum_ptr[i] = (const void*) &sum_i[i];
    }
  }

  for (int i = 0; i < num_buffers; i++) {
    clEnqueueWriteBuffer(cmd_queue_list[i], dst_buffer_list[i], CL_TRUE, dst_offset ? dst_offset[i] : 0, typesize, sum_ptr[i], 0, NULL, event_list ? &event_list[i] : NULL);
  }

  for (int i = 0; i < num_buffers; i++) {
    free(temp_buf[i]);
  }
  free(temp_buf);

  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueAllReduceBuffer(cl_command_queue * cmd_queue_list,
                         cl_uint num_buffers,
                         cl_mem * src_buffer_list,
                         cl_mem * dst_buffer_list,
                         size_t * src_offset,
                         size_t * dst_offset,
                         size_t cb,
                         cl_channel_type datatype,
                         cl_uint num_events_in_wait_list,
                         const cl_event * event_wait_list,
                         cl_event * event_list)
{
  size_t typesize = sizeof(float);
  if (datatype == CL_DOUBLE) sizeof(double);

  void** temp_buf = (void**) malloc(sizeof(void*) * num_buffers);
  for (int i = 0; i < num_buffers; i++) {
    temp_buf[i] = malloc(cb);
    clEnqueueReadBuffer(cmd_queue_list[i], src_buffer_list[i], CL_TRUE, src_offset ? src_offset[i] : 0, cb, temp_buf[i], num_events_in_wait_list, event_wait_list, NULL);
  }

  double sum_d = 0.0;
  float  sum_f = 0.0;
  unsigned int sum_u = 0;
  int sum_i = 0;
  const void* sum_ptr;

  for (int i = 0; i < num_buffers; i++) {
    if (datatype == CL_DOUBLE) {
      double* t = (double*) temp_buf[i];
      for (int j = 0; j < cb / sizeof(double); j++) sum_d += t[j];
      sum_ptr = (const void*) &sum_d;
    } else if (datatype == CL_FLOAT) {
      float* t = (float*) temp_buf[i];
      for (int j = 0; j < cb / sizeof(float); j++) sum_f += t[j];
      sum_ptr = (const void*) &sum_f;
    } else if (datatype == CL_UNSIGNED_INT32) {
      unsigned int* t = (unsigned int*) temp_buf[i];
      for (int j = 0; j < cb / sizeof(unsigned int); j++) sum_u += t[j];
      sum_ptr = (const void*) &sum_u;
    } else if (datatype == CL_SIGNED_INT32) {
      int* t = (int*) temp_buf[i];
      for (int j = 0; j < cb / sizeof(int); j++) sum_i += t[j];
      sum_ptr = (const void*) &sum_i;
    }
  }

  for (int i = 0; i < num_buffers; i++) {
    clEnqueueWriteBuffer(cmd_queue_list[i], dst_buffer_list[i], CL_TRUE, dst_offset ? dst_offset[i] : 0, typesize, sum_ptr, 0, NULL, event_list ? &event_list[i] : NULL);
  }

  for (int i = 0; i < num_buffers; i++) {
    free(temp_buf[i]);
  }
  free(temp_buf);

  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueReduceBuffer(cl_command_queue cmd_queue,
                      cl_uint num_src_buffers,
                      cl_mem * src_buffer_list,
                      cl_mem dst_buffer,
                      size_t * src_offset,
                      size_t dst_offset,
                      size_t cb,
                      cl_channel_type datatype,
                      cl_uint num_events_in_wait_list,
                      const cl_event * event_wait_list,
                      cl_event * event)
{
  size_t typesize = sizeof(float);
  if (datatype == CL_DOUBLE) sizeof(double);

  void** temp_buf = (void**) malloc(sizeof(void*) * num_src_buffers);
  for (int i = 0; i < num_src_buffers; i++) {
    temp_buf[i] = malloc(cb);
    clEnqueueReadBuffer(cmd_queue, src_buffer_list[i], CL_TRUE, src_offset ? src_offset[i] : 0, cb, temp_buf[i], num_events_in_wait_list, event_wait_list, NULL);
  }

  double sum_d = 0.0;
  float  sum_f = 0.0;
  unsigned int sum_u = 0;
  int sum_i = 0;
  const void* sum_ptr;

  for (int i = 0; i < num_src_buffers; i++) {
    if (datatype == CL_DOUBLE) {
      double* t = (double*) temp_buf[i];
      for (int j = 0; j < cb / sizeof(double); j++) sum_d += t[j];
      sum_ptr = (const void*) &sum_d;
    } else if (datatype == CL_FLOAT) {
      float* t = (float*) temp_buf[i];
      for (int j = 0; j < cb / sizeof(float); j++) sum_f += t[j];
      sum_ptr = (const void*) &sum_f;
    } else if (datatype == CL_UNSIGNED_INT32) {
      unsigned int* t = (unsigned int*) temp_buf[i];
      for (int j = 0; j < cb / sizeof(unsigned int); j++) sum_u += t[j];
      sum_ptr = (const void*) &sum_u;
    } else if (datatype == CL_SIGNED_INT32) {
      int* t = (int*) temp_buf[i];
      for (int j = 0; j < cb / sizeof(int); j++) sum_i += t[j];
      sum_ptr = (const void*) &sum_i;
    }
  }
  clEnqueueWriteBuffer(cmd_queue, dst_buffer, CL_TRUE, dst_offset, typesize, sum_ptr, 0, NULL, event);

  for (int i = 0; i < num_src_buffers; i++) {
    free(temp_buf[i]);
  }
  free(temp_buf);

  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueScatterBuffer(cl_command_queue * cmd_queue_list,
                       cl_mem src_buffer,
                       cl_uint num_dst_buffers,
                       cl_mem * dst_buffer_list,
                       size_t src_offset,
                       size_t * dst_offset,
                       size_t cb, // size sent to each dst buffer
                       cl_uint num_events_in_wait_list,
                       const cl_event * event_wait_list,
                       cl_event * event_list)
{
  for (cl_uint i = 0; i < num_dst_buffers; i++) {
    CLCommand* command = CommandFactory::instance()->NewCommand(cmd_queue_list[i]->c_obj, num_events_in_wait_list, event_wait_list, CL_COMMAND_COPY_BUFFER);
    command->mem_src = src_buffer->c_obj;
    command->mem_dst = dst_buffer_list[i]->c_obj;
    command->off_src = src_offset + (cb * i);
    command->off_dst = dst_offset ? dst_offset[i] : 0;
    command->cb = cb;
    command->device = cmd_queue_list[i]->c_obj->device;
    if( event_list != NULL )
      event_list[i] = &command->event->st_obj;

    cmd_queue_list[i]->c_obj->Enqueue(command);
  }
  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueGatherBuffer(cl_command_queue cmd_queue,
                      cl_uint num_src_buffers,
                      cl_mem * src_buffer_list,
                      cl_mem dst_buffer,
                      size_t * src_offset,
                      size_t dst_offset,
                      size_t cb, // size for any single receive
                      cl_uint num_events_in_wait_list,
                      const cl_event * event_wait_list,
                      cl_event * event) 
{
  CLCommand* p_command = CommandFactory::instance()->NewCommand(cmd_queue->c_obj, 0, NULL, CL_COMMAND_GATHER_BUFFER);

  for (cl_uint i = 0; i < num_src_buffers; i++) {
    CLCommand* command = CommandFactory::instance()->NewCommand(cmd_queue->c_obj, num_events_in_wait_list, event_wait_list, CL_COMMAND_COPY_BUFFER);
    command->mem_src = src_buffer_list[i]->c_obj;
    command->mem_dst = dst_buffer->c_obj;
    command->off_src = src_offset ? src_offset[i] : 0;
    command->off_dst = dst_offset + (cb * i);
    command->cb = cb;
    command->device = cmd_queue->c_obj->device;
    command->p_command = p_command;
    p_command->children.push_back(command);

    cmd_queue->c_obj->Enqueue(command);
  }
	if (event) *event = &p_command->event->st_obj;
  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueAllGatherBuffer(cl_command_queue * cmd_queue_list,
                         cl_uint num_buffers,
                         cl_mem * src_buffer_list,
                         cl_mem * dst_buffer_list,
                         size_t * src_offset,
                         size_t * dst_offset,
                         size_t cb,
                         cl_uint num_events_in_wait_list,
                         const cl_event * event_wait_list,
                         cl_event * event_list)
{
  for (cl_uint i = 0; i < num_buffers; i++) {
    clEnqueueGatherBuffer(cmd_queue_list[i], num_buffers, src_buffer_list, dst_buffer_list[i], src_offset, dst_offset ? dst_offset[i] : 0, cb, num_events_in_wait_list, event_wait_list, event_list ? event_list + i : NULL);
  }
  return CL_SUCCESS;
}

