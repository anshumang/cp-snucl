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

#include <arch/fusion/FusionDevice.h>

SNUCL_DEBUG_HEADER("FusionDevice");

int FusionCLDevice::CreateDevices(vector<CLDevice*>* devices, bool force) {
  char filepath[256];
  char* snucl_root = getenv("SNUCLROOT");
  sprintf(filepath, "%s/bin/snucl_fusion", snucl_root);
  SNUCL_DEBUG("FUSION FILE [%s]", filepath);

  char* line = NULL;
  size_t len = 0;
  ssize_t read;
  FILE* fp = fopen(filepath, "r");
  if (fp == NULL) return 0;
  if (read = getline(&line, &len, fp) != -1) {
    int mode = atoi(line);
    if (!force && mode != 1) {
      fclose(fp);
      return 0;
    }
  } else {
    fclose(fp);
    return 0;
  }

  vector<FusionCLDevice*> devices_fusion;
  vector<CLDevice*> devices_copy;
  devices_copy.resize(devices->size());
  copy(devices->begin(), devices->end(), devices_copy.begin());

  while ((read = getline(&line, &len, fp)) != -1) {
    FusionCLDevice* device = ParseConfigure(line, devices);
    if (device) devices_fusion.push_back(device);
  }
  fclose(fp);

  devices->clear();

  for (vector<FusionCLDevice*>::iterator it = devices_fusion.begin(); it != devices_fusion.end(); ++it) {
    devices->push_back(*it);
  }

  for (vector<CLDevice*>::iterator it = devices_copy.begin(); it != devices_copy.end(); ++it) {
    bool infusion = false;
    for (vector<FusionCLDevice*>::iterator it2 = devices_fusion.begin(); it2 != devices_fusion.end(); ++it2) {
      FusionCLDevice* fd = *it2;
      vector<CLDevice*>* devices_in_fusion = &fd->devices;
      for (vector<CLDevice*>::iterator it3 = devices_in_fusion->begin(); it3 != devices_in_fusion->end(); ++it3) {
        if (*it == *it3) {
          infusion = true;
          break;
        }
      }
      if (infusion) break;
    }
    CLDevice* dev = *it;
    SNUCL_DEBUG("DEV[%lu] TYPE[%lx] INFUSION[%d]", dev->id, dev->type, infusion);
    if (!infusion) devices->push_back(*it);
  }

  return 0;
}

FusionCLDevice* FusionCLDevice::ParseConfigure(char* line, vector<CLDevice*>* devices) {
  char* pch;
  FusionCLDevice* device;
  pch = strtok(line, " ");
  if (pch[0] == '#') return NULL;
  if (strcmp(pch, "CPU") == 0) {
    device = new FusionCLDevice(CL_DEVICE_TYPE_CPU);
  } else if (strcmp(pch, "GPU") == 0) {
    device = new FusionCLDevice(CL_DEVICE_TYPE_GPU);
  } else {
    SNUCL_ERROR("UNSUPPORT DEVICE TYPE [%s]", pch);
    return NULL;
  }

  vector<int> cpus;
  vector<int> gpus;
  while (1) {
    pch = strtok(NULL, " \r\n");
    if (pch == NULL || pch[0] == '#') break;
    if (pch[0] == 'C') cpus.push_back(atoi(pch+1));
    else if (pch[0] == 'G') gpus.push_back(atoi(pch+1));
    else SNUCL_ERROR("UNSUPPORT DEVICE TYPE [%s]", pch);
  }

  printf("CPU : [");
  for (vector<int>::iterator it = cpus.begin(); it != cpus.end(); ++it) {
    int i = *it;
    printf(" %d ", i); 
  }
  printf("]\n");

  printf("GPU : [");
  for (vector<int>::iterator it = gpus.begin(); it != gpus.end(); ++it) {
    int i = *it;
    printf(" %d ", i); 
  }
  printf("]\n");

  for (vector<int>::iterator it = cpus.begin(); it != cpus.end(); ++it) {
    int id = *it;
    int dev_idx = 0;
    for (vector<CLDevice*>::iterator it2 = devices->begin(); it2 != devices->end(); ++it2) {
      CLDevice* dev = *it2;
      if (dev->type == CL_DEVICE_TYPE_CPU) {
        if (dev_idx++ == id) {
          device->devices.push_back(dev);
          break;
        }
      }
    }
  }

  for (vector<int>::iterator it = gpus.begin(); it != gpus.end(); ++it) {
    int id = *it;
    int dev_idx = 0;
    for (vector<CLDevice*>::iterator it2 = devices->begin(); it2 != devices->end(); ++it2) {
      CLDevice* dev = *it2;
      if (dev->type == CL_DEVICE_TYPE_GPU) {
        if (dev_idx++ == id) {
          device->devices.push_back(dev);
          break;
        }
      }
    }
  }

  device->NDEV = device->devices.size();

  return device;
}

FusionCLDevice::FusionCLDevice(cl_device_type type) : CLDevice(type) {
  worker = new CLDeviceWorker(this);
}

FusionCLDevice::~FusionCLDevice() {
  delete worker;
}

void FusionCLDevice::Init() {
}

void FusionCLDevice::InitInfo() {
}

void FusionCLDevice::LaunchKernel(CLCommand* command) {
  map<cl_uint, CLKernelArg*>* args = &command->kernel_args;
  for (map<cl_uint, CLKernelArg*>::const_iterator it = args->begin(); it != args->end(); ++it) {
    CLKernelArg* arg = it->second;
    CLMem* mem = arg->mem;
    if (mem) {
      CheckBuffer(mem);
    }
  }
}

void FusionCLDevice::ReadBuffer(CLCommand* command) {

}

void FusionCLDevice::WriteBuffer(CLCommand* command) {
  SNUCL_CHECK();
  CLMem* mem_dst = command->mem_dst;
  size_t off_dst = command->off_dst;
  size_t size = command->cb;
  void* ptr = command->ptr;

  PageRepository::instance()->RecordPage(command);

  command->event->Complete();
}

void FusionCLDevice::CopyBuffer(CLCommand* command) {

}

void FusionCLDevice::BuildProgram(CLCommand* command) {
  CLProgram* program = command->program;

  FOREACH_DEV(i) {
    devices[i]->EnqueueReadyQueue(command);
  }

  program->build_status = CL_BUILD_SUCCESS;
}

void FusionCLDevice::AllocBuffer(CLMem* mem) {

}

void FusionCLDevice::FreeBuffer(CLCommand* command) {

}

