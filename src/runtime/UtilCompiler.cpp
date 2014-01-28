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

#include <UtilCompiler.h>
#include <UtilDebug.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <string.h>

SNUCL_DEBUG_HEADER("UtilCompiler");

void SNUCL_GetProgramInfo(CLProgram* program, const char* options) {
  char cmd[256];
  char kernel_dir[128];
  char kernel_file[128];
  char kernel_info_file[128];

  mkdir("snucl_kernels", 0755);
  mkdir("snucl_kernels/host", 0755);
  sprintf(kernel_dir, "snucl_kernels/host");
  char file_idx[11];
  while (1) {
    file_idx[0] = rand()%10 + '0';
    file_idx[1] = rand()%10 + '0';
    file_idx[2] = rand()%10 + '0';
    file_idx[3] = rand()%26 + 'a';
    file_idx[4] = rand()%26 + 'A';
    file_idx[5] = rand()%26 + 'a';
    file_idx[6] = rand()%10 + '0';
    file_idx[7] = rand()%10 + '0';
    file_idx[8] = rand()%26 + 'A';
    file_idx[9] = rand()%10 + '0';
    file_idx[10] = '\0';
    sprintf(kernel_file, "%s/__cl_kernel_%s.cl", kernel_dir, file_idx);
    if (access(kernel_file, F_OK) != 0) {
      FILE *file = fopen(kernel_file, "a");
      fprintf(file, "%s", program->src);
      fclose(file);
      break;
    }
  }

  if (options && strlen(options)) {
    SNUCL_DEBUG("OPTIONS [%s]", options);
    sprintf(cmd, "t-host.sh %s %s", file_idx, options);
  } else {
    SNUCL_DEBUG("NO OPTIONS [%s]", "");
    sprintf(cmd, "t-host.sh %s", file_idx);
  }

  if (system(cmd) == -1) {
    SNUCL_ERROR("BUILD ERROR PROGRAM[%lu]", program->id);
  }

	int temp_idx;

  sprintf(kernel_info_file, "%s/__cl_kernel_info_%s.so", kernel_dir, file_idx);
  void* kernel_info_handle = dlopen(kernel_info_file, RTLD_NOW);
  if (kernel_info_handle == NULL) SNUCL_ERROR("KERNEL INFO SO [%s] DOES NOT EXIST", kernel_info_file);

  char *err;
  unsigned int* _cl_kernel_num = (unsigned int*)dlsym(kernel_info_handle, "_cl_kernel_num");
  if ((err = dlerror()) != NULL) SNUCL_ERROR("SYMBOL [%s] DOES NOT EXIST", "_cl_kernel_num");
  program->num_kernels = (size_t)*_cl_kernel_num;

	//_cl_kernel_names
	char** _cl_kernel_names = (char**)dlsym(kernel_info_handle, "_cl_kernel_names");
  if ((err = dlerror()) != NULL) SNUCL_ERROR("SYMBOL [%s] DOES NOT EXIST", "_cl_kernel_names");
	program->kernel_names = (char**)malloc(sizeof(char*)*program->num_kernels);
  for (uint i = 0; i < program->num_kernels; ++i) {
		program->kernel_names[i] = (char*)calloc((strlen(_cl_kernel_names[i])+1),sizeof(char));
    strcpy(program->kernel_names[i], _cl_kernel_names[i]);
    SNUCL_DEBUG("PROGRAM[%d] KERNEL[%u] [%s]", program->id, i, program->kernel_names[i]);
  }

	//_cl_kernel_num_args
	unsigned int* _cl_kernel_num_args = (unsigned int*)dlsym(kernel_info_handle, "_cl_kernel_num_args");
	program->kernel_num_args = (cl_uint*)malloc(sizeof(cl_uint)*program->num_kernels);
	for (uint i=0; i<program->num_kernels; i++) 
		program->kernel_num_args[i] = (cl_uint)_cl_kernel_num_args[i];

	//_cl_kernel_attributes
	char** _cl_kernel_attributes = (char**)dlsym(kernel_info_handle, "_cl_kernel_attributes");
	program->kernel_attributes = (char**)malloc(sizeof(char*)*program->num_kernels);
	for (uint i=0; i<program->num_kernels; i++) {
		program->kernel_attributes[i] = (char*)calloc((strlen(_cl_kernel_attributes[i])+1),sizeof(char));
		strcpy(program->kernel_attributes[i], _cl_kernel_attributes[i]);
	}

	//_cl_kernel_work_group_size_hint
	unsigned int (*_cl_kernel_work_group_size_hint)[3] = (unsigned int (*)[3])dlsym(kernel_info_handle, "_cl_kernel_work_group_size_hint");
	program->kernel_work_group_size_hint = (size_t**)malloc(sizeof(size_t*)*program->num_kernels);
	for (uint i=0; i<program->num_kernels; i++) {
		program->kernel_work_group_size_hint[i] = (size_t*)malloc(sizeof(size_t)*3);
		program->kernel_work_group_size_hint[i][0] = (size_t)_cl_kernel_work_group_size_hint[i][0];
		program->kernel_work_group_size_hint[i][1] = (size_t)_cl_kernel_work_group_size_hint[i][1];
		program->kernel_work_group_size_hint[i][2] = (size_t)_cl_kernel_work_group_size_hint[i][2];
	}
	//_cl_kernel_reqd_work_group_size
	unsigned int (*_cl_kernel_reqd_work_group_size)[3] = (unsigned int (*)[3])dlsym(kernel_info_handle, "_cl_kernel_reqd_work_group_size");
	program->kernel_reqd_work_group_size = (size_t**)malloc(sizeof(size_t*)*program->num_kernels);
	for (uint i=0; i<program->num_kernels; i++) {
		program->kernel_reqd_work_group_size[i] = (size_t*)malloc(sizeof(size_t)*3);
		program->kernel_reqd_work_group_size[i][0] = (size_t)_cl_kernel_reqd_work_group_size[i][0];
		program->kernel_reqd_work_group_size[i][1] = (size_t)_cl_kernel_reqd_work_group_size[i][1];
		program->kernel_reqd_work_group_size[i][2] = (size_t)_cl_kernel_reqd_work_group_size[i][2];
	}
	//_cl_kernel_local_mem_size
	unsigned long long* _cl_kernel_local_mem_size = (unsigned long long*)dlsym(kernel_info_handle, "_cl_kernel_local_mem_size");
	program->kernel_local_mem_size = (cl_ulong*)malloc(sizeof(cl_ulong)*program->num_kernels);
	for (uint i=0; i<program->num_kernels; i++) {
		program->kernel_local_mem_size[i] = (cl_ulong)_cl_kernel_local_mem_size[i];
	}
	//_cl_kernel_private_mem_size
	unsigned long long* _cl_kernel_private_mem_size = (unsigned long long*)dlsym(kernel_info_handle, "_cl_kernel_private_mem_size");
	program->kernel_private_mem_size = (cl_ulong*)malloc(sizeof(cl_ulong)*program->num_kernels);
	for (uint i=0; i<program->num_kernels; i++) {
		program->kernel_private_mem_size[i] = (cl_ulong)_cl_kernel_private_mem_size[i];
	}
	//_cl_kernel_arg_address_qualifier
	char** _cl_kernel_arg_address_qualifier = (char**)dlsym(kernel_info_handle, "_cl_kernel_arg_address_qualifier");
	program->kernel_arg_address_qualifier = (cl_kernel_arg_address_qualifier**)malloc(sizeof(cl_kernel_arg_address_qualifier*)*program->num_kernels);
	for (uint i=0; i<program->num_kernels; i++) {
		program->kernel_arg_address_qualifier[i] = (cl_kernel_arg_address_qualifier*)malloc(sizeof(cl_kernel_arg_address_qualifier)*program->kernel_num_args[i]);
		for (uint j=0; j<program->kernel_num_args[i]; j++) {
			char item = _cl_kernel_arg_address_qualifier[i][j];
			switch (item) {
				case '0': program->kernel_arg_address_qualifier[i][j] = CL_KERNEL_ARG_ADDRESS_PRIVATE; break;
				case '1': program->kernel_arg_address_qualifier[i][j] = CL_KERNEL_ARG_ADDRESS_LOCAL; break;
				case '2': program->kernel_arg_address_qualifier[i][j] = CL_KERNEL_ARG_ADDRESS_CONSTANT; break;
				case '3': program->kernel_arg_address_qualifier[i][j] = CL_KERNEL_ARG_ADDRESS_GLOBAL; break;
				default: SNUCL_ERROR("%s", "Wrong kernel arg address quailifier"); break;
			}
		}
	}
	//_cl_kernel_arg_access_qualifier
	char** _cl_kernel_arg_access_qualifier = (char**)dlsym(kernel_info_handle, "_cl_kernel_arg_access_qualifier");
	program->kernel_arg_access_qualifier = (cl_kernel_arg_access_qualifier**)malloc(sizeof(cl_kernel_arg_access_qualifier*)*program->num_kernels);
	for (uint i=0; i<program->num_kernels; i++) {
		program->kernel_arg_access_qualifier[i] = (cl_kernel_arg_access_qualifier*)malloc(sizeof(cl_kernel_arg_access_qualifier)*program->kernel_num_args[i]);
		for (uint j=0; j<program->kernel_num_args[i]; j++) {
			char item = _cl_kernel_arg_access_qualifier[i][j];
			switch (item) {
				case '0': program->kernel_arg_access_qualifier[i][j] = CL_KERNEL_ARG_ACCESS_NONE; break;
				case '1': program->kernel_arg_access_qualifier[i][j] = CL_KERNEL_ARG_ACCESS_READ_WRITE; break;
				case '2': program->kernel_arg_access_qualifier[i][j] = CL_KERNEL_ARG_ACCESS_WRITE_ONLY; break;
				case '3': program->kernel_arg_access_qualifier[i][j] = CL_KERNEL_ARG_ACCESS_READ_ONLY; break;
				default: SNUCL_ERROR("%s", "Wrong kernel arg access quailifier"); break;
			}
		}
	}
	//_cl_kernel_arg_type_name
	char** _cl_kernel_arg_type_name = (char**)dlsym(kernel_info_handle, "_cl_kernel_arg_type_name");
	temp_idx = 0;
	program->kernel_arg_type_name = (char***)malloc(sizeof(char*)*program->num_kernels);
	for (uint i=0; i<program->num_kernels; i++) {
		program->kernel_arg_type_name[i] = (char**)malloc(sizeof(char*)*program->kernel_num_args[i]);
		for (uint j=0; j<program->kernel_num_args[i]; j++) {
			program->kernel_arg_type_name[i][j] = (char*)calloc((strlen(_cl_kernel_arg_type_name[temp_idx])+1),sizeof(char));
			strcpy(program->kernel_arg_type_name[i][j], _cl_kernel_arg_type_name[temp_idx++]);
		}
	}
	//_cl_kernel_arg_type_qualifier
	unsigned int* _cl_kernel_arg_type_qualifier = (unsigned int*)dlsym(kernel_info_handle, "_cl_kernel_arg_type_qualifier");
	temp_idx = 0;
	program->kernel_arg_type_qualifier = (cl_kernel_arg_type_qualifier**)malloc(sizeof(cl_kernel_arg_type_qualifier*)*program->num_kernels);
	for (uint i=0; i<program->num_kernels; i++) {
		program->kernel_arg_type_qualifier[i] = (cl_kernel_arg_type_qualifier*)malloc(sizeof(cl_kernel_arg_type_qualifier)*program->kernel_num_args[i]);
		for (uint j=0; j<program->kernel_num_args[i]; j++) {
			program->kernel_arg_type_qualifier[i][j] = (cl_kernel_arg_type_qualifier)_cl_kernel_arg_type_qualifier[temp_idx++];
		}
	}
	//_cl_kernel_arg_name
	char** _cl_kernel_arg_name = (char**)dlsym(kernel_info_handle, "_cl_kernel_arg_name");
	temp_idx = 0;
	program->kernel_arg_name = (char***)malloc(sizeof(char**)*program->num_kernels);
	for (uint i=0; i<program->num_kernels; i++) {
		program->kernel_arg_name[i] = (char**)malloc(sizeof(char*)*program->kernel_num_args[i]);
		for (uint j=0; j<program->kernel_num_args[i]; j++) {
			program->kernel_arg_name[i][j] = (char*)calloc((strlen(_cl_kernel_arg_name[temp_idx])+1),sizeof(char));
			strcpy(program->kernel_arg_name[i][j], _cl_kernel_arg_name[temp_idx++]);
		}
	}

	//preferred_work_group_size_multiple
	program->preferred_work_group_size_multiple = 64;

	//isBuiltInKernel
	program->isBuiltInKernel = (bool*)malloc(sizeof(bool)*program->num_kernels);
	memset(program->isBuiltInKernel, 0, sizeof(bool)*program->num_kernels);

  if (dlclose(kernel_info_handle) != 0) SNUCL_ERROR("CANNOT CLOSE KERNEL INFO SO [%s]", kernel_info_file);

}

