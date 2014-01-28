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

#include <arch/cluster/ClusterHost.h>
#include <arch/cluster/ClusterDevice.h>

SNUCL_DEBUG_HEADER("ClusterHost");

extern "C" int __real_main(int argc, char** argv);

ClusterHost::ClusterHost(int nnode) {
  this->nnode = nnode;
  g_platform.cluster = true;
  g_platform.host = true;
  InitDevices();
}

ClusterHost::~ClusterHost() {
}

int ClusterHost::Execute(int argc, char** argv) {
  SNUCL_INFO("%s", "Execute the host program");

  SNUCL_TimerStart(10);
  int ret = __real_main(argc, argv);
  SNUCL_INFO("Exit the host program [%lf] secs", SNUCL_TimerStop(10));

  MessageCodec mc;
  mc.SetTag(CLUSTER_TAG_EXIT);
  FOREACH_NODE(i) {
    mpi_ret = MPI_Send(mc.msg, MESSAGE_COMMAND_SIZE, MPI_CHAR, i, CLUSTER_TAG_COMMAND, MPI_COMM_WORLD);
  }

  delete g_platform.issuer;

  return ret;
}

void ClusterHost::InitDevices() {
  for (size_t i = 0; i < g_platform.devices.size(); ++i) {
//    delete g_platform.devices[i];
  }
  g_platform.devices.clear();

  FOREACH_NODE(i) {
    MessageCodec mc;
    mc.SetTag(CLUSTER_TAG_NODE_INFO);
    mpi_ret = MPI_Sendrecv(mc.msg, MESSAGE_COMMAND_SIZE, MPI_CHAR, i, CLUSTER_TAG_COMMAND, mc.msg, MESSAGE_COMMAND_SIZE, MPI_CHAR, i, CLUSTER_TAG_NODE_INFO, MPI_COMM_WORLD, &mpi_status);
    mc.Init();
    int ndev = mc.GetInt();
    SNUCL_DEBUG("NODE[%d] DEVS[%d]", i, ndev);
    for (int j = 0; j < ndev; ++j) {
      cl_device_type dev_type = (cl_device_type) mc.GetULong();
      SNUCL_DEBUG("NODE[%d] DEV[%d] TYPE[%lu]", i, j, dev_type);
      ClusterCLDevice::CreateDevices(&g_platform.devices, i, j, dev_type);
    }
  }

}

