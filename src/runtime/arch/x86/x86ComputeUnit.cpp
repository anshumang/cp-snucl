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

#include <arch/x86/x86ComputeUnit.h>
#include <dlfcn.h>
#include <arch/x86/TempKernelParam.h>

SNUCL_DEBUG_HEADER("x86ComputeUnit");

x86ComputeUnit::x86ComputeUnit(CLDevice* device, int id) {
  this->device = device;
  this->id = id;

  work_to_do = false;
	exit=false;

	program = NULL;
	program_id = -1;
	program_buildVersion = -1;

  pthread_mutex_init(&mutex, NULL);
  pthread_cond_init(&cond_work_to_do, NULL);

  pthread_create(&thread, NULL, &x86ComputeUnit::ThreadFunc, this);
}

x86ComputeUnit::~x86ComputeUnit() {
  pthread_mutex_lock(&mutex);
  work_to_do = true;
  exit = true;
  pthread_cond_signal(&cond_work_to_do);
  pthread_mutex_unlock(&mutex);

  pthread_join(thread, NULL);
}

void x86ComputeUnit::Launch(CLWorkGroupAssignment *wga) {
  pthread_mutex_lock(&mutex);
  work_queue.push_back(wga);
  work_to_do = true;
  pthread_mutex_unlock(&mutex);

  pthread_cond_signal(&cond_work_to_do);
}

void* x86ComputeUnit::ThreadFunc(void* argp) {
  ((x86ComputeUnit*) argp)->Run();
  return NULL;
}

void x86ComputeUnit::Run() {
  bool running = true;

  deque<CLWorkGroupAssignment*> pong_queue;

  while (running) {
    pthread_mutex_lock(&mutex);
    while (!work_to_do) pthread_cond_wait(&cond_work_to_do, &mutex);
    if (exit) running = false;
    pong_queue.swap(work_queue);
    pthread_mutex_unlock(&mutex);

    while (!pong_queue.empty()) {
      CLWorkGroupAssignment *wga = pong_queue.front();
      if (program == NULL || program_id != wga->command->program->id || program_buildVersion != wga->command->program->buildVersion) {
				program = wga->command->program;
				program_id = program->id;
				program_buildVersion = program->buildVersion;

				pthread_mutex_lock(&program->mutex_dev_specific);
				program_handle = program->dev_specific[device];
				pthread_mutex_unlock(&program->mutex_dev_specific);
      }

      int (*entry)(int id, void* argp);
      *(void**) (&entry) = dlsym(program_handle, "dev_entry");
      if (dlerror() != NULL) SNUCL_ERROR("SYMBOL [%s] DOES NOT EXIST", "dev_entry");
      x86_kernel_param* kp = TEMP_GET_x86KP_FROM_WGA(wga);
      (*entry)(id, kp);
      pong_queue.pop_front();
      free(kp);
    }

    pthread_mutex_lock(&mutex);
    if (work_queue.empty()) {
      work_to_do = false;
      pthread_cond_broadcast(&cond_work_to_do);
    }
    pthread_mutex_unlock(&mutex);
  }
}

void x86ComputeUnit::Sync() {
  pthread_mutex_lock(&mutex);
  while (work_to_do) pthread_cond_wait(&cond_work_to_do, &mutex);
  pthread_mutex_unlock(&mutex);
}

