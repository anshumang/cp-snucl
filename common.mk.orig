LINKTAIL  := -Wl,--wrap,clGetPlatformIDs,--wrap,clGetPlatformInfo,--wrap,clGetDeviceIDs,--wrap,clGetDeviceInfo,--wrap,clCreateSubDevices
LINKTAIL  += -Wl,--wrap,clRetainDevice,--wrap,clReleaseDevice,--wrap,clCreateContext,--wrap,clCreateContextFromType,--wrap,clRetainContext
LINKTAIL  += -Wl,--wrap,clReleaseContext,--wrap,clGetContextInfo,--wrap,clCreateCommandQueue,--wrap,clRetainCommandQueue,--wrap,clReleaseCommandQueue
LINKTAIL  += -Wl,--wrap,clGetCommandQueueInfo,--wrap,clCreateBuffer,--wrap,clCreateSubBuffer,--wrap,clCreateImage,--wrap,clRetainMemObject
LINKTAIL  += -Wl,--wrap,clReleaseMemObject,--wrap,clGetSupportedImageFormats,--wrap,clGetMemObjectInfo,--wrap,clGetImageInfo,--wrap,clSetMemObjectDestructorCallback
LINKTAIL  += -Wl,--wrap,clCreateSampler,--wrap,clRetainSampler,--wrap,clReleaseSampler,--wrap,clGetSamplerInfo,--wrap,clCreateProgramWithSource,--wrap,clCreateProgramWithBinary,--wrap,clCreateProgramWithBuiltInKernels,--wrap,clRetainProgram,--wrap,clReleaseProgram
LINKTAIL  += -Wl,--wrap,clBuildProgram,--wrap,clCompileProgram,--wrap,clLinkProgram,--wrap,clUnloadPlatformCompiler,--wrap,clGetProgramInfo
LINKTAIL  += -Wl,--wrap,clGetProgramBuildInfo,--wrap,clCreateKernel,--wrap,clCreateKernelsInProgram,--wrap,clRetainKernel,--wrap,clReleaseKernel
LINKTAIL  += -Wl,--wrap,clSetKernelArg,--wrap,clGetKernelInfo,--wrap,clGetKernelArgInfo,--wrap,clGetKernelWorkGroupInfo,--wrap,clWaitForEvents
LINKTAIL  += -Wl,--wrap,clGetEventInfo,--wrap,clCreateUserEvent,--wrap,clRetainEvent,--wrap,clReleaseEvent,--wrap,clSetUserEventStatus,--wrap,clSetEventCallback,--wrap,clGetEventProfilingInfo,--wrap,clFlush,--wrap,clFinish
LINKTAIL  += -Wl,--wrap,clEnqueueReadBuffer,--wrap,clEnqueueReadBufferRect,--wrap,clEnqueueWriteBuffer,--wrap,clEnqueueWriteBufferRect,--wrap,clEnqueueFillBuffer
LINKTAIL  += -Wl,--wrap,clEnqueueCopyBuffer,--wrap,clEnqueueCopyBufferRect,--wrap,clEnqueueReadImage,--wrap,clEnqueueWriteImage,--wrap,clEnqueueFillImage
LINKTAIL  += -Wl,--wrap,clEnqueueCopyImage,--wrap,clEnqueueCopyImageToBuffer,--wrap,clEnqueueCopyBufferToImage,--wrap,clEnqueueMapBuffer,--wrap,clEnqueueMapImage
LINKTAIL  += -Wl,--wrap,clEnqueueUnmapMemObject,--wrap,clEnqueueMigrateMemObjects,--wrap,clEnqueueNDRangeKernel,--wrap,clEnqueueTask,--wrap,clEnqueueNativeKernel
LINKTAIL  += -Wl,--wrap,clEnqueueMarkerWithWaitList,--wrap,clEnqueueBarrierWithWaitList,--wrap,clSetCommandQueueProperty
LINKTAIL  += -Wl,--wrap,clCreateImage2D,--wrap,clCreateImage3D,--wrap,clEnqueueMarker,--wrap,clEnqueueWaitForEvents,--wrap,clEnqueueBarrier
LINKTAIL  += -Wl,--wrap,clUnloadCompiler

TARGETDIR	:= bin
TARGET		:= $(TARGETDIR)/$(EXECUTABLE)
SRCDIR		:=
OBJDIR		:= obj
OBJS			:= $(patsubst %.cpp,$(OBJDIR)/%.cpp.o,$(notdir $(CCFILES)))

INCLUDES	+= -I$(SNUCLROOT)/inc -I.
WARNINGS	+= -Wno-write-strings
CXXFLAGS	+= $(INCLUDES) $(WARNINGS) -DUNIX
LDFLAGS		+= -L$(SNUCLROOT)/lib
LDFLAGS		+= -ldl -lpthread -lOpenCL

ifeq ($(verbose), 1)
	VERBOSE :=
else
	VERBOSE := @
endif

ifeq ($(debug), 1)
	CXXFLAGS	+= -g
endif

ifeq ($(cluster), 1)
	CXX				:= mpic++
	CXXFLAGS	+= -DCLUSTER
	LDFLAGS		+= -lsnucl
	#LINKTAIL	+= -Wl,--wrap,main
else
	CXX 			:= g++
	LDFLAGS		+= -lsnucl 
endif

ifeq ($(USEGLLIB),1)
	LDFLAGS += -lGL -lGLU -lX11 -lXmu
endif

ifeq ($(USEGLUT),1)
	LDFLAGS += -lglut
endif

$(OBJDIR)/%.cpp.o: $(SRCDIR)%.cpp
	$(VERBOSE)$(CXX) $(CXXFLAGS) -o $@ -c $<

$(TARGET): makedirectories $(OBJS) Makefile
	$(VERBOSE)$(CXX) -o $(TARGET) $(OBJS) $(LDFLAGS) $(LINKTAIL)

makedirectories:
	$(VERBOSE)mkdir -p $(TARGETDIR)
	$(VERBOSE)mkdir -p $(OBJDIR)

clean:
	$(VERBOSE)rm -rf $(TARGETDIR)
	$(VERBOSE)rm -rf $(OBJDIR)
	$(VERBOSE)rm -rf snucl_kernels
