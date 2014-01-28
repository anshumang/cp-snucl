#include <CL/cl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

const char* kernel_src = "__kernel void sample(__global int* dst, __global int* src, int offset) {\n"
                         "  int id = get_global_id(0);\n"
                         "  dst[id] = src[id] + offset;\n"
                         "}\n";

int main(int argc, char** argv)
{
  cl_device_type    DEV_TYPE = CL_DEVICE_TYPE_CPU;
  cl_platform_id    platform;
  cl_device_id      device;
  cl_context        context;
  cl_command_queue  command_queue;
  cl_program        program;
  cl_kernel         kernel;
  cl_mem            buffer_src;
  cl_mem            buffer_dst;
  cl_int            err;

  size_t local      = 4;
  size_t global     = local * 8;
  size_t SIZE       = global;

  err = clGetPlatformIDs(1, &platform, NULL);
  if (err != CL_SUCCESS) { printf("[%s:%d] ERR[%d]\n", __FILE__, __LINE__, err); exit(EXIT_FAILURE); }

  cl_uint num_dev = 1;
  err = clGetDeviceIDs(platform, DEV_TYPE, num_dev, &device, &num_dev);
  if (err != CL_SUCCESS) { printf("[%s:%d] ERR[%d]\n", __FILE__, __LINE__, err); exit(EXIT_FAILURE); }

  if (num_dev < 1) exit(EXIT_FAILURE);

  int* host_src = (int*) calloc(SIZE, sizeof(int));
  for (int i = 0; i < SIZE; i++) {
    host_src[i] = i * 10;
  }

  int* host_dst = (int*) calloc(SIZE, sizeof(int));

  context = clCreateContext(0, num_dev, &device, NULL, NULL, &err);
  if (err != CL_SUCCESS) { printf("[%s:%d] ERR[%d]", __FILE__, __LINE__, err); exit(EXIT_FAILURE); }

  command_queue = clCreateCommandQueue(context, device, 0, &err);
  if (err != CL_SUCCESS) { printf("[%s:%d] ERR[%d]", __FILE__, __LINE__, err); exit(EXIT_FAILURE); }

  buffer_src = clCreateBuffer(context, CL_MEM_READ_ONLY, SIZE * sizeof(int), NULL, &err);
  if (err != CL_SUCCESS) { printf("[%s:%d] ERR[%d]", __FILE__, __LINE__, err); exit(EXIT_FAILURE); }

  buffer_dst = clCreateBuffer(context, CL_MEM_WRITE_ONLY, SIZE * sizeof(int), NULL, &err);
  if (err != CL_SUCCESS) { printf("[%s:%d] ERR[%d]", __FILE__, __LINE__, err); exit(EXIT_FAILURE); }

  size_t kernel_src_len = strlen(kernel_src);
  program = clCreateProgramWithSource(context, 1, (const char**) &kernel_src, &kernel_src_len, &err);
  if (err != CL_SUCCESS) { printf("[%s:%d] ERR[%d]", __FILE__, __LINE__, err); exit(EXIT_FAILURE); }

  err = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
  if (err != CL_SUCCESS) { printf("[%s:%d] ERR[%d]", __FILE__, __LINE__, err); exit(EXIT_FAILURE); }

  err = clEnqueueWriteBuffer(command_queue, buffer_src, CL_TRUE, 0, SIZE * sizeof(int), host_src, 0, NULL, NULL);
  if (err != CL_SUCCESS) { printf("[%s:%d] ERR[%d]", __FILE__, __LINE__, err); exit(EXIT_FAILURE); }

  kernel = clCreateKernel(program, "sample", &err);
  if (err != CL_SUCCESS) { printf("[%s:%d] ERR[%d]", __FILE__, __LINE__, err); exit(EXIT_FAILURE); }

  err = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void*) &buffer_dst);
  if (err != CL_SUCCESS) { printf("[%s:%d] ERR[%d]", __FILE__, __LINE__, err); exit(EXIT_FAILURE); }

  err = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void*) &buffer_src);
  if (err != CL_SUCCESS) { printf("[%s:%d] ERR[%d]", __FILE__, __LINE__, err); exit(EXIT_FAILURE); }

  int offset = 100;
  err = clSetKernelArg(kernel, 2, sizeof(cl_int), (void*) &offset);
  if (err != CL_SUCCESS) { printf("[%s:%d] ERR[%d]", __FILE__, __LINE__, err); exit(EXIT_FAILURE); }

  err = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global, &local, 0, NULL, NULL);
  if (err != CL_SUCCESS) { printf("[%s:%d] ERR[%d]", __FILE__, __LINE__, err); exit(EXIT_FAILURE); }

  err = clEnqueueReadBuffer(command_queue, buffer_dst, CL_TRUE, 0, SIZE * sizeof(int), host_dst, 0, NULL, NULL);
  if (err != CL_SUCCESS) { printf("[%s:%d] ERR[%d]", __FILE__, __LINE__, err); exit(EXIT_FAILURE); }

  for (int i = 0; i < SIZE; i++) printf("[%2d] %d\n", i, host_dst[i]);

  free(host_src);
  free(host_dst);

  return 0;
}

