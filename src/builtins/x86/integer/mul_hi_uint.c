/*****************************************************************************/
/* Copyright (C) 2010, 2011 Seoul National University                        */
/* and Samsung Electronics Co., Ltd.                                         */
/*                                                                           */
/* Contributed by Sangmin Seo <sangmin@aces.snu.ac.kr>, Jungwon Kim          */
/* <jungwon@aces.snu.ac.kr>, Jaejin Lee <jlee@cse.snu.ac.kr>, Seungkyun Kim  */
/* <seungkyun@aces.snu.ac.kr>, Jungho Park <jungho@aces.snu.ac.kr>,          */
/* Honggyu Kim <honggyu@aces.snu.ac.kr>, Jeongho Nah                         */
/* <jeongho@aces.snu.ac.kr>, Sung Jong Seo <sj1557.seo@samsung.com>,         */
/* Seung Hak Lee <s.hak.lee@samsung.com>, Seung Mo Cho                       */
/* <seungm.cho@samsung.com>, Hyo Jung Song <hjsong@samsung.com>,             */
/* Sang-Bum Suh <sbuk.suh@samsung.com>, and Jong-Deok Choi                   */
/* <jd11.choi@samsung.com>                                                   */
/*                                                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This file is part of the SNU-SAMSUNG OpenCL runtime.                      */
/*                                                                           */
/* The SNU-SAMSUNG OpenCL runtime is free software: you can redistribute it  */
/* and/or modify it under the terms of the GNU Lesser General Public License */
/* as published by the Free Software Foundation, either version 3 of the     */
/* License, or (at your option) any later version.                           */
/*                                                                           */
/* The SNU-SAMSUNG OpenCL runtime is distributed in the hope that it will be */
/* useful, but WITHOUT ANY WARRANTY; without even the implied warranty of    */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General  */
/* Public License for more details.                                          */
/*                                                                           */
/* You should have received a copy of the GNU Lesser General Public License  */
/* along with the SNU-SAMSUNG OpenCL runtime. If not, see                    */
/* <http://www.gnu.org/licenses/>.                                           */
/*****************************************************************************/

#include <cl_cpu_ops.h>

// uint
uint mul_hi(uint x, uint y){
	return (uint)(((ullong)x * (ullong)y) >> 32);
}
uint2 mul_hi(uint2 x, uint2 y){
	uint2 rst;
  for (int i = 0; i < 2; i++)
    rst[i] = (uint)(((ullong)x[i] * (ullong)y[i]) >> 32);
  return rst;
}
uint3 mul_hi(uint3 x, uint3 y){
	uint3 rst;
  for (int i = 0; i < 3; i++)
    rst[i] = (uint)(((ullong)x[i] * (ullong)y[i]) >> 32);
  return rst;
}
uint4 mul_hi(uint4 x, uint4 y){
	uint4 rst;
  for (int i = 0; i < 4; i++)
    rst[i] = (uint)(((ullong)x[i] * (ullong)y[i]) >> 32);
  return rst;
}
uint8 mul_hi(uint8 x, uint8 y){
	uint8 rst;
  for (int i = 0; i < 8; i++)
    rst[i] = (uint)(((ullong)x[i] * (ullong)y[i]) >> 32);
  return rst;
}
uint16 mul_hi(uint16 x, uint16 y){
	uint16 rst;
  for (int i = 0; i < 16; i++)
    rst[i] = (uint)(((ullong)x[i] * (ullong)y[i]) >> 32);
  return rst;
}

