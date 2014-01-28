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

#include <arch/x86/TempKernelParam.h>
#include <Utils.h>

SNUCL_DEBUG_HEADER("TempKernelParam");

/* sampler: normalized coords */
enum _sampler_normalized_coords {
  CLK_NORMALIZED_COORDS_TRUE = 0x0100,
  CLK_NORMALIZED_COORDS_FALSE = 0x0080
};

/* sampler: addressing mode */
enum _sampler_addressing_modes {
  CLK_ADDRESS_CLAMP_TO_EDGE = 0x0010, 
  CLK_ADDRESS_CLAMP = 0x0008, 
  CLK_ADDRESS_NONE = 0x0004, 
  CLK_ADDRESS_REPEAT = 0x0002, 
  CLK_ADDRESS_MIRRORED_REPEAT = 0x0001
};

/* sampler: filter mode */
enum _sampler_filter_modes {
  CLK_FILTER_NEAREST = 0x0040, 
  CLK_FILTER_LINEAR = 0x0020
};

x86_kernel_param* TEMP_GET_x86KP_FROM_WGA(CLWorkGroupAssignment* wga) {
  x86_kernel_param* kp = (x86_kernel_param*) malloc(sizeof(x86_kernel_param));

  CLCommand* command = wga->command;
  kp->launch_id = (unsigned int) command->id;
  kp->work_dim = command->work_dim;
  kp->lws[0] = command->lws[0];
  kp->lws[1] = command->lws[1];
  kp->lws[2] = command->lws[2];
  kp->gwo[0] = command->gwo[0];
  kp->gwo[1] = command->gwo[1];
  kp->gwo[2] = command->gwo[2];
  kp->orig_grid[0] = command->gws[0] / command->lws[0];
  kp->orig_grid[1] = command->gws[1] / command->lws[1];
  kp->orig_grid[2] = command->gws[2] / command->lws[2];
  kp->wg_id_start = wga->wg_id_start;
  kp->wg_id_end = wga->wg_id_end;
	kp->kernel_idx = command->kernel->kernel_idx;

  int args_offset = 0;
  map<cl_uint, CLKernelArg*>* args = command->kernel_args;
  for (map<cl_uint, CLKernelArg*>::const_iterator it = args->begin(); it != args->end(); ++it) {
    cl_uint index = it->first;
    CLKernelArg* arg = it->second;
    if (arg->mem) {
      CLMem* mem = arg->mem;
      if (mem->is_image) {
        pthread_mutex_lock(&mem->mutex_dev_specific);
        void* m = (cl_mem) mem->dev_specific[command->device];
        pthread_mutex_unlock(&mem->mutex_dev_specific);

        CLImage * image_obj = &mem->image_obj;
        if(mem->image_desc.image_type == CL_MEM_OBJECT_IMAGE1D_BUFFER) {
          CLMem * imagebuf = mem->image_desc.buffer->c_obj;
          command->device->CheckBuffer(imagebuf);
          image_obj->ptr = 
            mem->image_desc.buffer->c_obj->dev_specific[command->device];
        }
        else {
          image_obj->ptr = m;
        }
        image_obj->size = mem->size;
        image_obj->image_format = mem->image_format;
        image_obj->image_desc = mem->image_desc;
        image_obj->image_row_pitch = mem->image_row_pitch;
        image_obj->image_slice_pitch = mem->image_slice_pitch;
        image_obj->image_elem_size = mem->image_elem_size;
        image_obj->image_channels = mem->image_channels;

        memcpy(kp->args + args_offset, (void*)&image_obj, sizeof(void*));
        args_offset += sizeof(void*);
      }
      else {
        pthread_mutex_lock(&mem->mutex_dev_specific);
        void* m = (cl_mem) mem->dev_specific[command->device];
        pthread_mutex_unlock(&mem->mutex_dev_specific);
        memcpy(kp->args + args_offset, &m, sizeof(void*));
        args_offset += sizeof(void*);
      }
    } else if (arg->sampler) {
      CLSampler * sampler = arg->sampler;
      int value = 0;

      // normalized coords
      if (sampler->normalized_coords) {
        value |= CLK_NORMALIZED_COORDS_TRUE;
      }
      else {
        value |= CLK_NORMALIZED_COORDS_FALSE;
      }

      // addressing mode
      switch (sampler->addressing_mode) {
        case CL_ADDRESS_CLAMP_TO_EDGE:
          value |= CLK_ADDRESS_CLAMP_TO_EDGE;
          break;
        case CL_ADDRESS_CLAMP:
          value |= CLK_ADDRESS_CLAMP;
          break;
        case CL_ADDRESS_NONE:
          value |= CLK_ADDRESS_NONE;
          break;
        case CL_ADDRESS_REPEAT:
          value |= CLK_ADDRESS_REPEAT;
          break;
        case CL_ADDRESS_MIRRORED_REPEAT:
          value |= CLK_ADDRESS_MIRRORED_REPEAT;
          break;
        default:
          break;
      }

      // filter mode
      switch (sampler->filter_mode) {
        case CL_FILTER_NEAREST:
          value |= CLK_FILTER_NEAREST;
          break;
        case CL_FILTER_LINEAR:
          value |= CLK_FILTER_LINEAR;
          break;
        default:
          break;
      }
      memcpy(kp->args + args_offset, &value, sizeof(value));
      args_offset += sizeof(value);
    } else if (arg->local) {
      uint32_t local_size = (uint32_t) arg->size;
      memcpy(kp->args + args_offset, &local_size, sizeof(uint32_t));
      args_offset += sizeof(uint32_t);
    } else {
      memcpy(kp->args + args_offset, arg->value, arg->size);
      args_offset += arg->size;
    }
  }

  return kp;
}

