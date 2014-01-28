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

#include <CLObject.h>
#include <Utils.h>
#include <mpi.h>
#include <arch/cluster/ClusterHost.h>
#include <arch/cluster/ClusterCompute.h>

SNUCL_DEBUG_HEADER("ClusterEntry");

static int rank, size;
static char processor_name[MPI_MAX_PROCESSOR_NAME];
MPI_Comm MPI_COMM_NODE;

int RunClusterHost(int argc, char** argv) {
  SNUCL_CHECK();
  SNUCL_INFO("[%s] Host node (with %d compute nodes)", processor_name, size - 1);
  SNUCL_CHECK();
  ClusterHost* host = new ClusterHost(size - 1);
  SNUCL_CHECK();
  int ret = host->Execute(argc, argv);
  SNUCL_CHECK();
  return ret;
}

int RunClusterCompute(int argc, char** argv) {
  SNUCL_INFO("[%s] Compute node (rank %d)", processor_name, rank);
  ClusterCompute* compute = new ClusterCompute(rank, processor_name);
  return compute->Execute(argc, argv);
}

int ClusterMain(int argc, char** argv) {
  int ret;
  MPI_Group MPI_GROUP_WORLD, MPI_GROUP_NODE;

  MPI_Init(&argc, &argv);
  MPI_Comm_group(MPI_COMM_WORLD, &MPI_GROUP_WORLD);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Get_processor_name(processor_name, &ret);

  int* ranks = (int*) malloc(size * sizeof(int));
  for (int i = 0; i < size; i++) ranks[i] = i;

  MPI_Group_incl(MPI_GROUP_WORLD, size - 1, ranks + 1, &MPI_GROUP_NODE);
  MPI_Comm_create(MPI_COMM_WORLD, MPI_GROUP_NODE, &MPI_COMM_NODE);

  ret = (rank == 0) ? RunClusterHost(argc, argv) : RunClusterCompute(argc, argv);

  MPI_Finalize();

  return ret;
}

extern "C" int __wrap_main(int argc, char** argv) {
  return ClusterMain(argc, argv);
}

