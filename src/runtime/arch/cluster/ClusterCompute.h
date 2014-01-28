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

#ifndef __CLUSTERCOMPUTE_H
#define __CLUSTERCOMPUTE_H

#define MAX_MSG_IN_CHUNK  128

#include <CLObject.h>
#include <Utils.h>
#include <mpi.h>
#include <arch/cluster/ClusterMessage.h>
#include <arch/cluster/ClusterSync.h>

class ClusterCompute {
public:
  ClusterCompute(int rank, char* name);
  ~ClusterCompute();
  int Execute(int argc, char** argv);

private:
  void HandleMsg(char* msg);
  void HandleMsgExit();
  void HandleMsgNodeInfo();
  void HandleMsgDevInfo(char* msg);
  void HandleMsgProgramBuild(char* msg);
  void HandleMsgKernelLaunch(char* msg);
  void HandleMsgMemAlloc(char* msg);
  void HandleMsgMemCopy(char* msg);
  void HandleMsgMemSend(char* msg);
  void HandleMsgMemRecv(char* msg);
  void HandleMsgMemCopyRect(char* msg);
  void HandleMsgMemSendRect(char* msg);
  void HandleMsgMemRecvRect(char* msg);
  void HandleMsgMemBcast(char* msg);
  void HandleMsgMemAlltoall(char* msg);
  void HandleMsgMemFree(char* msg);
  void HandleMsgDevStats(char* msg);
  void HandleMsgMarker(char* msg);
  void HandleMsgNativeKernel(char* msg);
  void HandleMsgImageInfo(char* msg);
  void HandleMsgImageCopy(char* msg);
  void HandleMsgImageRecv(char* msg);
  void HandleMsgImageSend(char* msg);
  void HandleMsgImageBuffer(char* msg);
  void HandleMsgBufferImage(char* msg);

  void AllocMem(int dev_id, unsigned long mem_id, cl_mem_flags flags, size_t size, bool image, cl_image_format* image_format, cl_image_desc* image_desc, bool sub, unsigned long p_mem_id, cl_buffer_create_type types, cl_buffer_region* buf_create_info);
  unsigned long GetMemID(int dev_id, MessageCodec* mc);
  unsigned long GetSamplerID(int dev_id, MessageCodec* mc);

  void InitContext();

  ClusterSync* MakeCS(unsigned long event_id);
  void CheckCompleteEvents();
  void CheckCompleteEventsCS();
  void EnqueueReadyQueue(CLDevice* device, CLCommand* command, cl_bool blocking);

private:

  int                             rank;
  char*                           name;
  bool                            running;
  int                             mpi_ret;
  MPI_Status                      mpi_status;
  MPI_Request                     mpi_request;

  CLContext*                      ctx;
  vector<CLDevice*>*              devs;
  map<unsigned long, CLMem*>      mems;
  map<unsigned long, CLSampler*>  samplers;
  map<unsigned long, CLProgram*>  programs;
  map<unsigned long, CLKernel*>   kernels;

  map<unsigned long, CLCommand*>  wait_events;
  vector<ClusterSync*>            wait_events_cs;

  char                            msg[MESSAGE_COMMAND_SIZE];

  MessageCodec* mc;
  unsigned long*                  cids;
  map<unsigned long, char*>       cid_msgs;
};

#endif

