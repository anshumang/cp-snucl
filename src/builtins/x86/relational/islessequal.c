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

inline static int snu_islessequal(double x, double y) {
  return x<=y;
} 
inline static int snu_islessequalv(double x, double y) {
  return x<=y? -1 : 0; 
} 

int islessequal(float x, float y) {
	return snu_islessequal(x, y);
}

int2 islessequal(float2 x, float2 y) {
	int2 ret;
	ret[0] = snu_islessequalv(x[0], y[0]);
	ret[1] = snu_islessequalv(x[1], y[1]);
	return ret;
}

int3 islessequal(float3 x, float3 y) {
	int3 ret;
	for(int i=0;i<3;i++) {
		ret[i] = snu_islessequalv(x[i], y[i]);
  }
	return ret;
}

int4 islessequal(float4 x, float4 y) {
	int4 ret;
	for(int i=0;i<4;i++) {
		ret[i] = snu_islessequalv(x[i], y[i]);
  }
	return ret;
}

int8 islessequal(float8 x, float8 y) {
	int8 ret;
	for(int i=0;i<8;i++) {
		ret[i] = snu_islessequalv(x[i], y[i]);
  }
	return ret;
}

int16 islessequal(float16 x, float16 y) {
	int16 ret;
	for(int i=0;i<16;i++) {
    ret[i] = snu_islessequalv(x[i], y[i]);
  }
	return ret;
}

int islessequal(double x, double y) {
	return snu_islessequal(x, y);
}

long2 islessequal(double2 x, double2 y) {
	long2 ret;
	ret[0] = snu_islessequalv(x[0], y[0]);
	ret[1] = snu_islessequalv(x[1], y[1]);
	return ret;
}

long3 islessequal(double3 x, double3 y) {
	long3 ret;
	for(int i=0;i<3;i++) {
		ret[i] = snu_islessequalv(x[i], y[i]);
  }
	return ret;
}

long4 islessequal(double4 x, double4 y) {
	long4 ret;
	for(int i=0;i<4;i++) {
		ret[i] = snu_islessequalv(x[i], y[i]);
  }
	return ret;
}

long8 islessequal(double8 x, double8 y) {
	long8 ret;
	for(int i=0;i<8;i++) {
		ret[i] = snu_islessequalv(x[i], y[i]);
  }
	return ret;
}

long16 islessequal(double16 x, double16 y) {
	long16 ret;
	for(int i=0;i<16;i++) {
		ret[i] = snu_islessequalv(x[i], y[i]);
  }
	return ret;
}
