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
#include "convert_util.h"

ullong convert_ulong_sat_rtz(int x) {
	return (ullong)(x < 0 ? 0 : x);
}

ulong2 convert_ulong2_sat_rtz(int2 x) {
	ulong2 ret;                                              
	for (int i = 0; i < 2; i++)                            
		ret[i] = (ullong)(x[i] < 0 ? 0 : x[i]);
	return ret;                                            
}

ulong3 convert_ulong3_sat_rtz(int3 x) {
	ulong3 ret;                                              
	for (int i = 0; i < 3; i++)                            
		ret[i] = (ullong)(x[i] < 0 ? 0 : x[i]);
	return ret;                                            
}

ulong4 convert_ulong4_sat_rtz(int4 x) {
	ulong4 ret;                                              
	for (int i = 0; i < 4; i++)                            
		ret[i] = (ullong)(x[i] < 0 ? 0 : x[i]);
	return ret;                                            
}

ulong8 convert_ulong8_sat_rtz(int8 x) {
	ulong8 ret;                                              
	for (int i = 0; i < 8; i++)                            
		ret[i] = (ullong)(x[i] < 0 ? 0 : x[i]);
	return ret;                                            
}

ulong16 convert_ulong16_sat_rtz(int16 x) {
	ulong16 ret;                                              
	for (int i = 0; i < 16; i++)                           
		ret[i] = (ullong)(x[i] < 0 ? 0 : x[i]);
	return ret;                                            
}

