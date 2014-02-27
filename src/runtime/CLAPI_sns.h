#ifdef __CLAPI_SNS_H
#define __CLAPI_SNS_H

#define CL_USE_DEPRECATED_OPENCL_1_1_APIS

#include <CL/cl.h>

#ifdef __cplusplus
extern "C" {
#endif

/* TODO prune ones not needed by split n spray implementation */
/* Platform API */
extern CL_API_ENTRY cl_int CL_API_CALL
__sns_clGetPlatformIDs(cl_uint          /* num_entries */,
                 cl_platform_id * /* platforms */,
                 cl_uint *        /* num_platforms */) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL 
__sns_clGetPlatformInfo(cl_platform_id   /* platform */, 
                  cl_platform_info /* param_name */,
                  size_t           /* param_value_size */, 
                  void *           /* param_value */,
                  size_t *         /* param_value_size_ret */) CL_API_SUFFIX__VERSION_1_0;

/* Device APIs */
extern CL_API_ENTRY cl_int CL_API_CALL
__sns_clGetDeviceIDs(cl_platform_id   /* platform */,
               cl_device_type   /* device_type */, 
               cl_uint          /* num_entries */, 
               cl_device_id *   /* devices */, 
               cl_uint *        /* num_devices */) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
__sns_clGetDeviceInfo(cl_device_id    /* device */,
                cl_device_info  /* param_name */, 
                size_t          /* param_value_size */, 
                void *          /* param_value */,
                size_t *        /* param_value_size_ret */) CL_API_SUFFIX__VERSION_1_0;
    
extern CL_API_ENTRY cl_int CL_API_CALL
__sns_clCreateSubDevices(cl_device_id                         /* in_device */,
                   const cl_device_partition_property * /* properties */,
                   cl_uint                              /* num_devices */,
                   cl_device_id *                       /* out_devices */,
                   cl_uint *                            /* num_devices_ret */) CL_API_SUFFIX__VERSION_1_2;

extern CL_API_ENTRY cl_int CL_API_CALL
__sns_clRetainDevice(cl_device_id /* device */) CL_API_SUFFIX__VERSION_1_2;
    
extern CL_API_ENTRY cl_int CL_API_CALL
__sns_clReleaseDevice(cl_device_id /* device */) CL_API_SUFFIX__VERSION_1_2;
    
/* Context APIs  */
/* TODO sns -- create context with all available devices (?) regardless of request for now. Implment a get device num call before creation of context. (Proabably the first call only {some global data can contain it.})*/
extern CL_API_ENTRY cl_context CL_API_CALL
__sns_clCreateContext(const cl_context_properties * /* properties */,		
                cl_uint                 /* num_devices */,
                const cl_device_id *    /* devices */,
                void (CL_CALLBACK * /* pfn_notify */)(const char *, const void *, size_t, void *),
                void *                  /* user_data */,
                cl_int *                /* errcode_ret */) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_context CL_API_CALL
__sns_clCreateContextFromType(const cl_context_properties * /* properties */,
                        cl_device_type          /* device_type */,
                        void (CL_CALLBACK *     /* pfn_notify*/ )(const char *, const void *, size_t, void *),
                        void *                  /* user_data */,
                        cl_int *                /* errcode_ret */) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
__sns_clRetainContext(cl_context /* context */) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
__sns_clReleaseContext(cl_context /* context */) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
__sns_clGetContextInfo(cl_context         /* context */, 
                 cl_context_info    /* param_name */, 
                 size_t             /* param_value_size */, 
                 void *             /* param_value */, 
                 size_t *           /* param_value_size_ret */) CL_API_SUFFIX__VERSION_1_0;

/* Create a command queue for each device */
/* Command Queue APIs */
extern CL_API_ENTRY cl_command_queue CL_API_CALL
__sns_clCreateCommandQueue(cl_context                     /* context */, 
                     cl_device_id                   /* device */, 
                     cl_command_queue_properties    /* properties */,
                     cl_int *                       /* errcode_ret */) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
__sns_clRetainCommandQueue(cl_command_queue /* command_queue */) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
__sns_clReleaseCommandQueue(cl_command_queue /* command_queue */) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
__sns_clGetCommandQueueInfo(cl_command_queue      /* command_queue */,
                      cl_command_queue_info /* param_name */,
                      size_t                /* param_value_size */,
                      void *                /* param_value */,
                      size_t *              /* param_value_size_ret */) CL_API_SUFFIX__VERSION_1_0;

/* Memory Object APIs */
extern CL_API_ENTRY cl_mem CL_API_CALL
__sns_clCreateBuffer(cl_context   /* context */,
               cl_mem_flags /* flags */,
               size_t       /* size */,
               void *       /* host_ptr */,
               cl_int *     /* errcode_ret */) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_mem CL_API_CALL
__sns_clCreateSubBuffer(cl_mem                   /* buffer */,
                  cl_mem_flags             /* flags */,
                  cl_buffer_create_type    /* buffer_create_type */,
                  const void *             /* buffer_create_info */,
                  cl_int *                 /* errcode_ret */) CL_API_SUFFIX__VERSION_1_1;

extern CL_API_ENTRY cl_mem CL_API_CALL
__sns_clCreateImage(cl_context              /* context */,
              cl_mem_flags            /* flags */,
              const cl_image_format * /* image_format */,
              const cl_image_desc *   /* image_desc */, 
              void *                  /* host_ptr */,
              cl_int *                /* errcode_ret */) CL_API_SUFFIX__VERSION_1_2;
                        
extern CL_API_ENTRY cl_int CL_API_CALL
__sns_clRetainMemObject(cl_mem /* memobj */) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
__sns_clReleaseMemObject(cl_mem /* memobj */) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
__sns_clGetSupportedImageFormats(cl_context           /* context */,
                           cl_mem_flags         /* flags */,
                           cl_mem_object_type   /* image_type */,
                           cl_uint              /* num_entries */,
                           cl_image_format *    /* image_formats */,
                           cl_uint *            /* num_image_formats */) CL_API_SUFFIX__VERSION_1_0;
                                    
extern CL_API_ENTRY cl_int CL_API_CALL
__sns_clGetMemObjectInfo(cl_mem           /* memobj */,
                   cl_mem_info      /* param_name */, 
                   size_t           /* param_value_size */,
                   void *           /* param_value */,
                   size_t *         /* param_value_size_ret */) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
__sns_clGetImageInfo(cl_mem           /* image */,
               cl_image_info    /* param_name */, 
               size_t           /* param_value_size */,
               void *           /* param_value */,
               size_t *         /* param_value_size_ret */) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
__sns_clSetMemObjectDestructorCallback(  cl_mem /* memobj */, 
                                    void (CL_CALLBACK * /*pfn_notify*/)( cl_mem /* memobj */, void* /*user_data*/), 
                                    void * /*user_data */ )             CL_API_SUFFIX__VERSION_1_1;  

/* Sampler APIs */
extern CL_API_ENTRY cl_sampler CL_API_CALL
__sns_clCreateSampler(cl_context          /* context */,
                cl_bool             /* normalized_coords */, 
                cl_addressing_mode  /* addressing_mode */, 
                cl_filter_mode      /* filter_mode */,
                cl_int *            /* errcode_ret */) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
__sns_clRetainSampler(cl_sampler /* sampler */) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
__sns_clReleaseSampler(cl_sampler /* sampler */) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
__sns_clGetSamplerInfo(cl_sampler         /* sampler */,
                 cl_sampler_info    /* param_name */,
                 size_t             /* param_value_size */,
                 void *             /* param_value */,
                 size_t *           /* param_value_size_ret */) CL_API_SUFFIX__VERSION_1_0;
                            
/* Program Object APIs  */
extern CL_API_ENTRY cl_program CL_API_CALL
__sns_clCreateProgramWithSource(cl_context        /* context */,
                          cl_uint           /* count */,
                          const char **     /* strings */,
                          const size_t *    /* lengths */,
                          cl_int *          /* errcode_ret */) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_program CL_API_CALL
__sns_clCreateProgramWithBinary(cl_context                     /* context */,
                          cl_uint                        /* num_devices */,
                          const cl_device_id *           /* device_list */,
                          const size_t *                 /* lengths */,
                          const unsigned char **         /* binaries */,
                          cl_int *                       /* binary_status */,
                          cl_int *                       /* errcode_ret */) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_program CL_API_CALL
__sns_clCreateProgramWithBuiltInKernels(cl_context            /* context */,
                                  cl_uint               /* num_devices */,
                                  const cl_device_id *  /* device_list */,
                                  const char *          /* kernel_names */,
                                  cl_int *              /* errcode_ret */) CL_API_SUFFIX__VERSION_1_2;

extern CL_API_ENTRY cl_int CL_API_CALL
__sns_clRetainProgram(cl_program /* program */) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
__sns_clReleaseProgram(cl_program /* program */) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
__sns_clBuildProgram(cl_program           /* program */,
               cl_uint              /* num_devices */,
               const cl_device_id * /* device_list */,
               const char *         /* options */, 
               void (CL_CALLBACK *  /* pfn_notify */)(cl_program /* program */, void * /* user_data */),
               void *               /* user_data */) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
__sns_clCompileProgram(cl_program           /* program */,
                 cl_uint              /* num_devices */,
                 const cl_device_id * /* device_list */,
                 const char *         /* options */, 
                 cl_uint              /* num_input_headers */,
                 const cl_program *   /* input_headers */,
                 const char **        /* header_include_names */,
                 void (CL_CALLBACK *  /* pfn_notify */)(cl_program /* program */, void * /* user_data */),
                 void *               /* user_data */) CL_API_SUFFIX__VERSION_1_2;

extern CL_API_ENTRY cl_program CL_API_CALL
__sns_clLinkProgram(cl_context           /* context */,
              cl_uint              /* num_devices */,
              const cl_device_id * /* device_list */,
              const char *         /* options */, 
              cl_uint              /* num_input_programs */,
              const cl_program *   /* input_programs */,
              void (CL_CALLBACK *  /* pfn_notify */)(cl_program /* program */, void * /* user_data */),
              void *               /* user_data */,
              cl_int *             /* errcode_ret */ ) CL_API_SUFFIX__VERSION_1_2;


extern CL_API_ENTRY cl_int CL_API_CALL
__sns_clUnloadPlatformCompiler(cl_platform_id /* platform */) CL_API_SUFFIX__VERSION_1_2;

extern CL_API_ENTRY cl_int CL_API_CALL
__sns_clGetProgramInfo(cl_program         /* program */,
                 cl_program_info    /* param_name */,
                 size_t             /* param_value_size */,
                 void *             /* param_value */,
                 size_t *           /* param_value_size_ret */) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
__sns_clGetProgramBuildInfo(cl_program            /* program */,
                      cl_device_id          /* device */,
                      cl_program_build_info /* param_name */,
                      size_t                /* param_value_size */,
                      void *                /* param_value */,
                      size_t *              /* param_value_size_ret */) CL_API_SUFFIX__VERSION_1_0;
                            
/* Kernel Object APIs */
extern CL_API_ENTRY cl_kernel CL_API_CALL
__sns_clCreateKernel(cl_program      /* program */,
               const char *    /* kernel_name */,
               cl_int *        /* errcode_ret */) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
__sns_clCreateKernelsInProgram(cl_program     /* program */,
                         cl_uint        /* num_kernels */,
                         cl_kernel *    /* kernels */,
                         cl_uint *      /* num_kernels_ret */) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
__sns_clRetainKernel(cl_kernel    /* kernel */) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
__sns_clReleaseKernel(cl_kernel   /* kernel */) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
__sns_clSetKernelArg(cl_kernel    /* kernel */,
               cl_uint      /* arg_index */,
               size_t       /* arg_size */,
               const void * /* arg_value */) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
__sns_clGetKernelInfo(cl_kernel       /* kernel */,
                cl_kernel_info  /* param_name */,
                size_t          /* param_value_size */,
                void *          /* param_value */,
                size_t *        /* param_value_size_ret */) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
__sns_clGetKernelArgInfo(cl_kernel       /* kernel */,
                   cl_uint         /* arg_indx */,
                   cl_kernel_arg_info  /* param_name */,
                   size_t          /* param_value_size */,
                   void *          /* param_value */,
                   size_t *        /* param_value_size_ret */) CL_API_SUFFIX__VERSION_1_2;

extern CL_API_ENTRY cl_int CL_API_CALL
__sns_clGetKernelWorkGroupInfo(cl_kernel                  /* kernel */,
                         cl_device_id               /* device */,
                         cl_kernel_work_group_info  /* param_name */,
                         size_t                     /* param_value_size */,
                         void *                     /* param_value */,
                         size_t *                   /* param_value_size_ret */) CL_API_SUFFIX__VERSION_1_0;

/* Event Object APIs */
extern CL_API_ENTRY cl_int CL_API_CALL
__sns_clWaitForEvents(cl_uint             /* num_events */,
                const cl_event *    /* event_list */) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
__sns_clGetEventInfo(cl_event         /* event */,
               cl_event_info    /* param_name */,
               size_t           /* param_value_size */,
               void *           /* param_value */,
               size_t *         /* param_value_size_ret */) CL_API_SUFFIX__VERSION_1_0;
                            
extern CL_API_ENTRY cl_event CL_API_CALL
__sns_clCreateUserEvent(cl_context    /* context */,
                  cl_int *      /* errcode_ret */) CL_API_SUFFIX__VERSION_1_1;               
                            
extern CL_API_ENTRY cl_int CL_API_CALL
__sns_clRetainEvent(cl_event /* event */) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
__sns_clReleaseEvent(cl_event /* event */) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
__sns_clSetUserEventStatus(cl_event   /* event */,
                     cl_int     /* execution_status */) CL_API_SUFFIX__VERSION_1_1;
                     
extern CL_API_ENTRY cl_int CL_API_CALL
__sns_clSetEventCallback( cl_event    /* event */,
                    cl_int      /* command_exec_callback_type */,
                    void (CL_CALLBACK * /* pfn_notify */)(cl_event, cl_int, void *),
                    void *      /* user_data */) CL_API_SUFFIX__VERSION_1_1;

/* Profiling APIs */
extern CL_API_ENTRY cl_int CL_API_CALL
__sns_clGetEventProfilingInfo(cl_event            /* event */,
                        cl_profiling_info   /* param_name */,
                        size_t              /* param_value_size */,
                        void *              /* param_value */,
                        size_t *            /* param_value_size_ret */) CL_API_SUFFIX__VERSION_1_0;
                                
/* Flush and Finish APIs */
extern CL_API_ENTRY cl_int CL_API_CALL
__sns_clFlush(cl_command_queue /* command_queue */) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
__sns_clFinish(cl_command_queue /* command_queue */) CL_API_SUFFIX__VERSION_1_0;


/* Call for each command queue and device */
/* Enqueued Commands APIs */
extern CL_API_ENTRY cl_int CL_API_CALL
__sns_clEnqueueReadBuffer(cl_command_queue    /* command_queue */,
                    cl_mem              /* buffer */,
                    cl_bool             /* blocking_read */,
                    size_t              /* offset */,
                    size_t              /* size */, 
                    void *              /* ptr */,
                    cl_uint             /* num_events_in_wait_list */,
                    const cl_event *    /* event_wait_list */,
                    cl_event *          /* event */) CL_API_SUFFIX__VERSION_1_0;
                            
extern CL_API_ENTRY cl_int CL_API_CALL
__sns_clEnqueueReadBufferRect(cl_command_queue    /* command_queue */,
                        cl_mem              /* buffer */,
                        cl_bool             /* blocking_read */,
                        const size_t *      /* buffer_offset */,
                        const size_t *      /* host_offset */, 
                        const size_t *      /* region */,
                        size_t              /* buffer_row_pitch */,
                        size_t              /* buffer_slice_pitch */,
                        size_t              /* host_row_pitch */,
                        size_t              /* host_slice_pitch */,                        
                        void *              /* ptr */,
                        cl_uint             /* num_events_in_wait_list */,
                        const cl_event *    /* event_wait_list */,
                        cl_event *          /* event */) CL_API_SUFFIX__VERSION_1_1;

/* Call for each command queue and device */
extern CL_API_ENTRY cl_int CL_API_CALL
__sns_clEnqueueWriteBuffer(cl_command_queue   /* command_queue */, 
                     cl_mem             /* buffer */, 
                     cl_bool            /* blocking_write */, 
                     size_t             /* offset */, 
                     size_t             /* size */, 
                     const void *       /* ptr */, 
                     cl_uint            /* num_events_in_wait_list */, 
                     const cl_event *   /* event_wait_list */, 
                     cl_event *         /* event */) CL_API_SUFFIX__VERSION_1_0;
                            
extern CL_API_ENTRY cl_int CL_API_CALL
__sns_clEnqueueWriteBufferRect(cl_command_queue    /* command_queue */,
                         cl_mem              /* buffer */,
                         cl_bool             /* blocking_write */,
                         const size_t *      /* buffer_offset */,
                         const size_t *      /* host_offset */, 
                         const size_t *      /* region */,
                         size_t              /* buffer_row_pitch */,
                         size_t              /* buffer_slice_pitch */,
                         size_t              /* host_row_pitch */,
                         size_t              /* host_slice_pitch */,                        
                         const void *        /* ptr */,
                         cl_uint             /* num_events_in_wait_list */,
                         const cl_event *    /* event_wait_list */,
                         cl_event *          /* event */) CL_API_SUFFIX__VERSION_1_1;
                            
extern CL_API_ENTRY cl_int CL_API_CALL
__sns_clEnqueueFillBuffer(cl_command_queue   /* command_queue */,
                    cl_mem             /* buffer */, 
                    const void *       /* pattern */, 
                    size_t             /* pattern_size */, 
                    size_t             /* offset */, 
                    size_t             /* size */, 
                    cl_uint            /* num_events_in_wait_list */, 
                    const cl_event *   /* event_wait_list */, 
                    cl_event *         /* event */) CL_API_SUFFIX__VERSION_1_2;
                            
extern CL_API_ENTRY cl_int CL_API_CALL
__sns_clEnqueueCopyBuffer(cl_command_queue    /* command_queue */, 
                    cl_mem              /* src_buffer */,
                    cl_mem              /* dst_buffer */, 
                    size_t              /* src_offset */,
                    size_t              /* dst_offset */,
                    size_t              /* size */, 
                    cl_uint             /* num_events_in_wait_list */,
                    const cl_event *    /* event_wait_list */,
                    cl_event *          /* event */) CL_API_SUFFIX__VERSION_1_0;
                            
extern CL_API_ENTRY cl_int CL_API_CALL
__sns_clEnqueueCopyBufferRect(cl_command_queue    /* command_queue */, 
                        cl_mem              /* src_buffer */,
                        cl_mem              /* dst_buffer */, 
                        const size_t *      /* src_origin */,
                        const size_t *      /* dst_origin */,
                        const size_t *      /* region */, 
                        size_t              /* src_row_pitch */,
                        size_t              /* src_slice_pitch */,
                        size_t              /* dst_row_pitch */,
                        size_t              /* dst_slice_pitch */,
                        cl_uint             /* num_events_in_wait_list */,
                        const cl_event *    /* event_wait_list */,
                        cl_event *          /* event */) CL_API_SUFFIX__VERSION_1_1;
                            
extern CL_API_ENTRY cl_int CL_API_CALL
__sns_clEnqueueReadImage(cl_command_queue     /* command_queue */,
                   cl_mem               /* image */,
                   cl_bool              /* blocking_read */, 
                   const size_t *       /* origin[3] */,
                   const size_t *       /* region[3] */,
                   size_t               /* row_pitch */,
                   size_t               /* slice_pitch */, 
                   void *               /* ptr */,
                   cl_uint              /* num_events_in_wait_list */,
                   const cl_event *     /* event_wait_list */,
                   cl_event *           /* event */) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
__sns_clEnqueueWriteImage(cl_command_queue    /* command_queue */,
                    cl_mem              /* image */,
                    cl_bool             /* blocking_write */, 
                    const size_t *      /* origin[3] */,
                    const size_t *      /* region[3] */,
                    size_t              /* input_row_pitch */,
                    size_t              /* input_slice_pitch */, 
                    const void *        /* ptr */,
                    cl_uint             /* num_events_in_wait_list */,
                    const cl_event *    /* event_wait_list */,
                    cl_event *          /* event */) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
__sns_clEnqueueFillImage(cl_command_queue   /* command_queue */,
                   cl_mem             /* image */, 
                   const void *       /* fill_color */, 
                   const size_t *     /* origin[3] */, 
                   const size_t *     /* region[3] */, 
                   cl_uint            /* num_events_in_wait_list */, 
                   const cl_event *   /* event_wait_list */, 
                   cl_event *         /* event */) CL_API_SUFFIX__VERSION_1_2;
                            
extern CL_API_ENTRY cl_int CL_API_CALL
__sns_clEnqueueCopyImage(cl_command_queue     /* command_queue */,
                   cl_mem               /* src_image */,
                   cl_mem               /* dst_image */, 
                   const size_t *       /* src_origin[3] */,
                   const size_t *       /* dst_origin[3] */,
                   const size_t *       /* region[3] */, 
                   cl_uint              /* num_events_in_wait_list */,
                   const cl_event *     /* event_wait_list */,
                   cl_event *           /* event */) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
__sns_clEnqueueCopyImageToBuffer(cl_command_queue /* command_queue */,
                           cl_mem           /* src_image */,
                           cl_mem           /* dst_buffer */, 
                           const size_t *   /* src_origin[3] */,
                           const size_t *   /* region[3] */, 
                           size_t           /* dst_offset */,
                           cl_uint          /* num_events_in_wait_list */,
                           const cl_event * /* event_wait_list */,
                           cl_event *       /* event */) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
__sns_clEnqueueCopyBufferToImage(cl_command_queue /* command_queue */,
                           cl_mem           /* src_buffer */,
                           cl_mem           /* dst_image */, 
                           size_t           /* src_offset */,
                           const size_t *   /* dst_origin[3] */,
                           const size_t *   /* region[3] */, 
                           cl_uint          /* num_events_in_wait_list */,
                           const cl_event * /* event_wait_list */,
                           cl_event *       /* event */) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY void * CL_API_CALL
__sns_clEnqueueMapBuffer(cl_command_queue /* command_queue */,
                   cl_mem           /* buffer */,
                   cl_bool          /* blocking_map */, 
                   cl_map_flags     /* map_flags */,
                   size_t           /* offset */,
                   size_t           /* size */,
                   cl_uint          /* num_events_in_wait_list */,
                   const cl_event * /* event_wait_list */,
                   cl_event *       /* event */,
                   cl_int *         /* errcode_ret */) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY void * CL_API_CALL
__sns_clEnqueueMapImage(cl_command_queue  /* command_queue */,
                  cl_mem            /* image */, 
                  cl_bool           /* blocking_map */, 
                  cl_map_flags      /* map_flags */, 
                  const size_t *    /* origin[3] */,
                  const size_t *    /* region[3] */,
                  size_t *          /* image_row_pitch */,
                  size_t *          /* image_slice_pitch */,
                  cl_uint           /* num_events_in_wait_list */,
                  const cl_event *  /* event_wait_list */,
                  cl_event *        /* event */,
                  cl_int *          /* errcode_ret */) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
__sns_clEnqueueUnmapMemObject(cl_command_queue /* command_queue */,
                        cl_mem           /* memobj */,
                        void *           /* mapped_ptr */,
                        cl_uint          /* num_events_in_wait_list */,
                        const cl_event *  /* event_wait_list */,
                        cl_event *        /* event */) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
__sns_clEnqueueMigrateMemObjects(cl_command_queue       /* command_queue */,
                           cl_uint                /* num_mem_objects */,
                           const cl_mem *         /* mem_objects */,
                           cl_mem_migration_flags /* flags */,
                           cl_uint                /* num_events_in_wait_list */,
                           const cl_event *       /* event_wait_list */,
                           cl_event *             /* event */) CL_API_SUFFIX__VERSION_1_2;

/* TODO fire this on all devices based on the custom kernel and allow capability to return partial results. Changes in parameters needed to inform application of partial failure so it can take corrective order */
/* task splitting mechanism to be implemented*/
extern CL_API_ENTRY cl_int CL_API_CALL
__sns_clEnqueueNDRangeKernel(cl_command_queue /* command_queue */,
                       cl_kernel        /* kernel */,
                       cl_uint          /* work_dim */,
                       const size_t *   /* global_work_offset */,
                       const size_t *   /* global_work_size */,
                       const size_t *   /* local_work_size */,
                       cl_uint          /* num_events_in_wait_list */,
                       const cl_event * /* event_wait_list */,
                       cl_event *       /* event */) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
__sns_clEnqueueTask(cl_command_queue  /* command_queue */,
              cl_kernel         /* kernel */,
              cl_uint           /* num_events_in_wait_list */,
              const cl_event *  /* event_wait_list */,
              cl_event *        /* event */) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
__sns_clEnqueueNativeKernel(cl_command_queue  /* command_queue */,
					  void (CL_CALLBACK * /*user_func*/)(void *), 
                      void *            /* args */,
                      size_t            /* cb_args */, 
                      cl_uint           /* num_mem_objects */,
                      const cl_mem *    /* mem_list */,
                      const void **     /* args_mem_loc */,
                      cl_uint           /* num_events_in_wait_list */,
                      const cl_event *  /* event_wait_list */,
                      cl_event *        /* event */) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
__sns_clEnqueueMarkerWithWaitList(cl_command_queue /* command_queue */,
                            cl_uint           /* num_events_in_wait_list */,
                            const cl_event *  /* event_wait_list */,
                            cl_event *        /* event */) CL_API_SUFFIX__VERSION_1_2;

extern CL_API_ENTRY cl_int CL_API_CALL
__sns_clEnqueueBarrierWithWaitList(cl_command_queue /* command_queue */,
                             cl_uint           /* num_events_in_wait_list */,
                             const cl_event *  /* event_wait_list */,
                             cl_event *        /* event */) CL_API_SUFFIX__VERSION_1_2;

#ifdef CL_USE_DEPRECATED_OPENCL_1_0_APIS
#warning CL_USE_DEPRECATED_OPENCL_1_0_APIS is defined. These APIs are unsupported and untested in OpenCL 1.1!
    /* 
     *  WARNING:
     *     This API introduces mutable state into the OpenCL implementation. It has been REMOVED
     *  to better facilitate thread safety.  The 1.0 API is not thread safe. It is not tested by the
     *  OpenCL 1.1 conformance test, and consequently may not work or may not work dependably.
     *  It is likely to be non-performant. Use of this API is not advised. Use at your own risk.
     *
     *  Software developers previously relying on this API are instructed to set the command queue 
     *  properties when creating the queue, instead. 
     */
    extern CL_API_ENTRY cl_int CL_API_CALL
    __sns_clSetCommandQueueProperty(cl_command_queue              /* command_queue */,
                              cl_command_queue_properties   /* properties */, 
                              cl_bool                        /* enable */,
                              cl_command_queue_properties * /* old_properties */) CL_EXT_SUFFIX__VERSION_1_0_DEPRECATED;
#endif /* CL_USE_DEPRECATED_OPENCL_1_0_APIS */
    
    
#ifdef CL_USE_DEPRECATED_OPENCL_1_1_APIS
    extern CL_API_ENTRY cl_mem CL_API_CALL
    __sns_clCreateImage2D(cl_context              /* context */,
                    cl_mem_flags            /* flags */,
                    const cl_image_format * /* image_format */,
                    size_t                  /* image_width */,
                    size_t                  /* image_height */,
                    size_t                  /* image_row_pitch */, 
                    void *                  /* host_ptr */,
                    cl_int *                /* errcode_ret */) CL_EXT_SUFFIX__VERSION_1_1_DEPRECATED;
    
    extern CL_API_ENTRY cl_mem CL_API_CALL
    __sns_clCreateImage3D(cl_context              /* context */,
                    cl_mem_flags            /* flags */,
                    const cl_image_format * /* image_format */,
                    size_t                  /* image_width */, 
                    size_t                  /* image_height */,
                    size_t                  /* image_depth */, 
                    size_t                  /* image_row_pitch */, 
                    size_t                  /* image_slice_pitch */, 
                    void *                  /* host_ptr */,
                    cl_int *                /* errcode_ret */) CL_EXT_SUFFIX__VERSION_1_1_DEPRECATED;
    
    extern CL_API_ENTRY cl_int CL_API_CALL
    __sns_clEnqueueMarker(cl_command_queue    /* command_queue */,
                    cl_event *          /* event */) CL_EXT_SUFFIX__VERSION_1_1_DEPRECATED;
    
    extern CL_API_ENTRY cl_int CL_API_CALL
    __sns_clEnqueueWaitForEvents(cl_command_queue /* command_queue */,
                           cl_uint          /* num_events */,
                           const cl_event * /* event_list */) CL_EXT_SUFFIX__VERSION_1_1_DEPRECATED;
    
    extern CL_API_ENTRY cl_int CL_API_CALL
    __sns_clEnqueueBarrier(cl_command_queue /* command_queue */) CL_EXT_SUFFIX__VERSION_1_1_DEPRECATED;

    extern CL_API_ENTRY cl_int CL_API_CALL
    __sns_clUnloadCompiler(void) CL_EXT_SUFFIX__VERSION_1_1_DEPRECATED;
    
#endif /* CL_USE_DEPRECATED_OPENCL_1_2_APIS */

#ifdef __cplusplus
}
#endif

#endif  /* __CLAPI_WRAP_H */

