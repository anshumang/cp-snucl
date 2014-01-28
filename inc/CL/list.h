/* Platform API */
extern cl_int clGetPlatformIDs
extern cl_int clGetPlatformInfo
/* Device APIs */
extern cl_int clGetDeviceIDs
extern cl_int clGetDeviceInfo
extern cl_int clCreateSubDevices //X
extern cl_int clRetainDevice //X
extern cl_int clReleaseDevice //X
/* Context APIs */
extern cl_context clCreateContext
extern cl_context clCreateContextFromType
extern cl_int clRetainContext
extern cl_int clReleaseContext
extern cl_int clGetContextInfo //X
/* Command Queue APIs */
extern cl_command_queue clCreateCommandQueue
extern cl_int clRetainCommandQueue
extern cl_int clReleaseCommandQueue
extern cl_int clGetCommandQueueInfo //X
/* Memory Object APIs */
extern cl_mem clCreateBuffer
extern cl_mem clCreateSubBuffer //X
extern cl_mem clCreateImage //X
extern cl_int clRetainMemObject
extern cl_int clReleaseMemObject
extern cl_int clGetSupportedImageFormats //X
extern cl_int clGetMemObjectInfo //X
extern cl_int clGetImageInfo //X
extern cl_int clSetMemObjectDestructorCallback //X
/* Sampler APIs */
extern cl_sampler clCreateSampler //X
extern cl_int clRetainSampler //X
extern cl_int clReleaseSampler //X
extern cl_int clGetSamplerInfo //X
/* Program Object APIs */
extern cl_program clCreateProgramWithSource
extern cl_program clCreateProgramWithBinary //X
extern cl_program clCreateProgramWithBuiltInKernels //X
extern cl_int clRetainProgram
extern cl_int clReleaseProgram
extern cl_int clBuildProgram
extern cl_int clCompileProgram //X
extern cl_program clLinkProgram //X
extern cl_int clUnloadPlatformCompiler //X
extern cl_int clGetProgramInfo //X
extern cl_int clGetProgramBuildInfo //X
/* Kernel Object APIs */
extern cl_kernel clCreateKernel
extern cl_int clCreateKernelsInProgram //X
extern cl_int clRetainKernel
extern cl_int clReleaseKernel
extern cl_int clSetKernelArg
extern cl_int clGetKernelInfo //X
extern cl_int clGetKernelArgInfo //X
extern cl_int clGetKernelWorkGroupInfo //X
/* Event Object APIs */
extern cl_int clWaitForEvents
extern cl_int clGetEventInfo //X
extern cl_event clCreateUserEvent //X
extern cl_int clRetainEvent
extern cl_int clReleaseEvent
extern cl_int clSetUserEventStatus //X
extern cl_int clSetEventCallback //X
/* Profiling APIs */
extern cl_int clGetEventProfilingInfo //X
/* Flush and Finish APIs */
extern cl_int clFlush
extern cl_int clFinish
/* Enqueued Commands APIs */
extern cl_int clEnqueueReadBuffer
extern cl_int clEnqueueReadBufferRect //X
extern cl_int clEnqueueWriteBuffer
extern cl_int clEnqueueWriteBufferRect //X
extern cl_int clEnqueueFillBuffer //X
extern cl_int clEnqueueCopyBuffer
extern cl_int clEnqueueCopyBufferRect //X
extern cl_int clEnqueueReadImage //X
extern cl_int clEnqueueWriteImage //X
extern cl_int clEnqueueFillImage //X
extern cl_int clEnqueueCopyImage //X
extern cl_int clEnqueueCopyImageToBuffer //X
extern cl_int clEnqueueCopyBufferToImage //X
extern void * clEnqueueMapBuffer //X
extern void * clEnqueueMapImage //X
extern cl_int clEnqueueUnmapMemObject //X
extern cl_int clEnqueueMigrateMemObjects //X
extern cl_int clEnqueueNDRangeKernel
extern cl_int clEnqueueTask
extern cl_int clEnqueueNativeKernel //X
extern cl_int clEnqueueMarkerWithWaitList //X
extern cl_int clEnqueueBarrierWithWaitList //X
