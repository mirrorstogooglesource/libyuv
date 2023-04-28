/*
 *  Copyright 2023 The LibYuv Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS. All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

/*
 * Copyright (c) 2023 SiFive, Inc. All rights reserved.
 *
 * Contributed by Darren Hsieh <darren.hsieh@sifive.com>
 * Contributed by Bruce Lai <bruce.lai@sifive.com>
 */

#include <assert.h>

#include "libyuv/row.h"

#if !defined(LIBYUV_DISABLE_RVV) && defined(__riscv_vector)
#include <riscv_vector.h>

#ifdef __cplusplus
namespace libyuv {
extern "C" {
#endif

void ARGBToAR64Row_RVV(const uint8_t* src_argb, uint16_t* dst_ar64, int width) {
  size_t avl = (size_t)4 * width;
  do {
    vuint16m8_t v_ar64;
    vuint8m4_t v_argb;
    size_t vl = __riscv_vsetvl_e8m4(avl);
    v_argb = __riscv_vle8_v_u8m4(src_argb, vl);
    v_ar64 = __riscv_vwaddu_vx_u16m8(v_argb, 0, vl);
    v_ar64 = __riscv_vmul_vx_u16m8(v_ar64, 0x0101, vl);
    __riscv_vse16_v_u16m8(dst_ar64, v_ar64, vl);
    avl -= vl;
    src_argb += vl;
    dst_ar64 += vl;
  } while (avl > 0);
}

void ARGBToAB64Row_RVV(const uint8_t* src_argb, uint16_t* dst_ab64, int width) {
  size_t avl = (size_t)width;
  do {
    vuint16m2_t v_b_16, v_g_16, v_r_16, v_a_16;
    vuint8m1_t v_b, v_g, v_r, v_a;
    size_t vl = __riscv_vsetvl_e8m1(avl);
    __riscv_vlseg4e8_v_u8m1(&v_b, &v_g, &v_r, &v_a, src_argb, vl);
    v_b_16 = __riscv_vwaddu_vx_u16m2(v_b, 0, vl);
    v_g_16 = __riscv_vwaddu_vx_u16m2(v_g, 0, vl);
    v_r_16 = __riscv_vwaddu_vx_u16m2(v_r, 0, vl);
    v_a_16 = __riscv_vwaddu_vx_u16m2(v_a, 0, vl);
    v_b_16 = __riscv_vmul_vx_u16m2(v_b_16, 0x0101, vl);
    v_g_16 = __riscv_vmul_vx_u16m2(v_g_16, 0x0101, vl);
    v_r_16 = __riscv_vmul_vx_u16m2(v_r_16, 0x0101, vl);
    v_a_16 = __riscv_vmul_vx_u16m2(v_a_16, 0x0101, vl);
    __riscv_vsseg4e16_v_u16m2(dst_ab64, v_r_16, v_g_16, v_b_16, v_a_16, vl);
    avl -= vl;
    src_argb += 4 * vl;
    dst_ab64 += 4 * vl;
  } while (avl > 0);
}

void AR64ToARGBRow_RVV(const uint16_t* src_ar64, uint8_t* dst_argb, int width) {
  size_t avl = (size_t)4 * width;
  do {
    vuint16m8_t v_ar64;
    vuint8m4_t v_argb;
    size_t vl = __riscv_vsetvl_e16m8(avl);
    v_ar64 = __riscv_vle16_v_u16m8(src_ar64, vl);
    v_argb = __riscv_vnsrl_wx_u8m4(v_ar64, 8, vl);
    __riscv_vse8_v_u8m4(dst_argb, v_argb, vl);
    avl -= vl;
    src_ar64 += vl;
    dst_argb += vl;
  } while (avl > 0);
}

void AB64ToARGBRow_RVV(const uint16_t* src_ab64, uint8_t* dst_argb, int width) {
  size_t avl = (size_t)width;
  do {
    vuint16m2_t v_b_16, v_g_16, v_r_16, v_a_16;
    vuint8m1_t v_b, v_g, v_r, v_a;
    size_t vl = __riscv_vsetvl_e16m2(avl);
    __riscv_vlseg4e16_v_u16m2(&v_r_16, &v_g_16, &v_b_16, &v_a_16, src_ab64, vl);
    v_b = __riscv_vnsrl_wx_u8m1(v_b_16, 8, vl);
    v_g = __riscv_vnsrl_wx_u8m1(v_g_16, 8, vl);
    v_r = __riscv_vnsrl_wx_u8m1(v_r_16, 8, vl);
    v_a = __riscv_vnsrl_wx_u8m1(v_a_16, 8, vl);
    __riscv_vsseg4e8_v_u8m1(dst_argb, v_b, v_g, v_r, v_a, vl);
    avl -= vl;
    src_ab64 += 4 * vl;
    dst_argb += 4 * vl;
  } while (avl > 0);
}

void RAWToARGBRow_RVV(const uint8_t* src_raw, uint8_t* dst_argb, int width) {
  size_t w = (size_t)width;
  size_t vl = __riscv_vsetvl_e8m2(w);
  vuint8m2_t v_a = __riscv_vmv_v_x_u8m2(255u, vl);
  do {
    vuint8m2_t v_b, v_g, v_r;
    __riscv_vlseg3e8_v_u8m2(&v_r, &v_g, &v_b, src_raw, vl);
    __riscv_vsseg4e8_v_u8m2(dst_argb, v_b, v_g, v_r, v_a, vl);
    w -= vl;
    src_raw += vl * 3;
    dst_argb += vl * 4;
    vl = __riscv_vsetvl_e8m2(w);
  } while (w > 0);
}

void RAWToRGBARow_RVV(const uint8_t* src_raw, uint8_t* dst_rgba, int width) {
  size_t w = (size_t)width;
  size_t vl = __riscv_vsetvl_e8m2(w);
  vuint8m2_t v_a = __riscv_vmv_v_x_u8m2(255u, vl);
  do {
    vuint8m2_t v_b, v_g, v_r;
    __riscv_vlseg3e8_v_u8m2(&v_r, &v_g, &v_b, src_raw, vl);
    __riscv_vsseg4e8_v_u8m2(dst_rgba, v_a, v_b, v_g, v_r, vl);
    w -= vl;
    src_raw += vl * 3;
    dst_rgba += vl * 4;
    vl = __riscv_vsetvl_e8m2(w);
  } while (w > 0);
}

void RAWToRGB24Row_RVV(const uint8_t* src_raw, uint8_t* dst_rgb24, int width) {
  size_t w = (size_t)width;
  do {
    vuint8m2_t v_b, v_g, v_r;
    size_t vl = __riscv_vsetvl_e8m2(w);
    __riscv_vlseg3e8_v_u8m2(&v_b, &v_g, &v_r, src_raw, vl);
    __riscv_vsseg3e8_v_u8m2(dst_rgb24, v_r, v_g, v_b, vl);
    w -= vl;
    src_raw += vl * 3;
    dst_rgb24 += vl * 3;
  } while (w > 0);
}

void ARGBToRAWRow_RVV(const uint8_t* src_argb, uint8_t* dst_raw, int width) {
  size_t w = (size_t)width;
  do {
    vuint8m2_t v_b, v_g, v_r, v_a;
    size_t vl = __riscv_vsetvl_e8m2(w);
    __riscv_vlseg4e8_v_u8m2(&v_b, &v_g, &v_r, &v_a, src_argb, vl);
    __riscv_vsseg3e8_v_u8m2(dst_raw, v_r, v_g, v_b, vl);
    w -= vl;
    src_argb += vl * 4;
    dst_raw += vl * 3;
  } while (w > 0);
}

void ARGBToRGB24Row_RVV(const uint8_t* src_argb,
                        uint8_t* dst_rgb24,
                        int width) {
  size_t w = (size_t)width;
  do {
    vuint8m2_t v_b, v_g, v_r, v_a;
    size_t vl = __riscv_vsetvl_e8m2(w);
    __riscv_vlseg4e8_v_u8m2(&v_b, &v_g, &v_r, &v_a, src_argb, vl);
    __riscv_vsseg3e8_v_u8m2(dst_rgb24, v_b, v_g, v_r, vl);
    w -= vl;
    src_argb += vl * 4;
    dst_rgb24 += vl * 3;
  } while (w > 0);
}

void RGB24ToARGBRow_RVV(const uint8_t* src_rgb24,
                        uint8_t* dst_argb,
                        int width) {
  size_t w = (size_t)width;
  size_t vl = __riscv_vsetvl_e8m2(w);
  vuint8m2_t v_a = __riscv_vmv_v_x_u8m2(255u, vl);
  do {
    vuint8m2_t v_b, v_g, v_r;
    __riscv_vlseg3e8_v_u8m2(&v_b, &v_g, &v_r, src_rgb24, vl);
    __riscv_vsseg4e8_v_u8m2(dst_argb, v_b, v_g, v_r, v_a, vl);
    w -= vl;
    src_rgb24 += vl * 3;
    dst_argb += vl * 4;
    vl = __riscv_vsetvl_e8m2(w);
  } while (w > 0);
}

void SplitRGBRow_RVV(const uint8_t* src_rgb,
                     uint8_t* dst_r,
                     uint8_t* dst_g,
                     uint8_t* dst_b,
                     int width) {
  size_t w = (size_t)width;
  do {
    vuint8m2_t v_b, v_g, v_r;
    size_t vl = __riscv_vsetvl_e8m2(w);
    __riscv_vlseg3e8_v_u8m2(&v_r, &v_g, &v_b, src_rgb, vl);
    __riscv_vse8_v_u8m2(dst_r, v_r, vl);
    __riscv_vse8_v_u8m2(dst_g, v_g, vl);
    __riscv_vse8_v_u8m2(dst_b, v_b, vl);
    w -= vl;
    dst_r += vl;
    dst_g += vl;
    dst_b += vl;
    src_rgb += vl * 3;
  } while (w > 0);
}

void MergeRGBRow_RVV(const uint8_t* src_r,
                     const uint8_t* src_g,
                     const uint8_t* src_b,
                     uint8_t* dst_rgb,
                     int width) {
  size_t w = (size_t)width;
  do {
    size_t vl = __riscv_vsetvl_e8m2(w);
    vuint8m2_t v_r = __riscv_vle8_v_u8m2(src_r, vl);
    vuint8m2_t v_g = __riscv_vle8_v_u8m2(src_g, vl);
    vuint8m2_t v_b = __riscv_vle8_v_u8m2(src_b, vl);
    __riscv_vsseg3e8_v_u8m2(dst_rgb, v_r, v_g, v_b, vl);
    w -= vl;
    src_r += vl;
    src_g += vl;
    src_b += vl;
    dst_rgb += vl * 3;
  } while (w > 0);
}

void SplitARGBRow_RVV(const uint8_t* src_argb,
                      uint8_t* dst_r,
                      uint8_t* dst_g,
                      uint8_t* dst_b,
                      uint8_t* dst_a,
                      int width) {
  size_t w = (size_t)width;
  do {
    vuint8m2_t v_b, v_g, v_r, v_a;
    size_t vl = __riscv_vsetvl_e8m2(w);
    __riscv_vlseg4e8_v_u8m2(&v_b, &v_g, &v_r, &v_a, src_argb, vl);
    __riscv_vse8_v_u8m2(dst_a, v_a, vl);
    __riscv_vse8_v_u8m2(dst_r, v_r, vl);
    __riscv_vse8_v_u8m2(dst_g, v_g, vl);
    __riscv_vse8_v_u8m2(dst_b, v_b, vl);
    w -= vl;
    dst_a += vl;
    dst_r += vl;
    dst_g += vl;
    dst_b += vl;
    src_argb += vl * 4;
  } while (w > 0);
}

void MergeARGBRow_RVV(const uint8_t* src_r,
                      const uint8_t* src_g,
                      const uint8_t* src_b,
                      const uint8_t* src_a,
                      uint8_t* dst_argb,
                      int width) {
  size_t w = (size_t)width;
  do {
    size_t vl = __riscv_vsetvl_e8m2(w);
    vuint8m2_t v_r = __riscv_vle8_v_u8m2(src_r, vl);
    vuint8m2_t v_g = __riscv_vle8_v_u8m2(src_g, vl);
    vuint8m2_t v_b = __riscv_vle8_v_u8m2(src_b, vl);
    vuint8m2_t v_a = __riscv_vle8_v_u8m2(src_a, vl);
    __riscv_vsseg4e8_v_u8m2(dst_argb, v_b, v_g, v_r, v_a, vl);
    w -= vl;
    src_r += vl;
    src_g += vl;
    src_b += vl;
    src_a += vl;
    dst_argb += vl * 4;
  } while (w > 0);
}

void SplitXRGBRow_RVV(const uint8_t* src_argb,
                      uint8_t* dst_r,
                      uint8_t* dst_g,
                      uint8_t* dst_b,
                      int width) {
  size_t w = (size_t)width;
  do {
    vuint8m2_t v_b, v_g, v_r, v_a;
    size_t vl = __riscv_vsetvl_e8m2(w);
    __riscv_vlseg4e8_v_u8m2(&v_b, &v_g, &v_r, &v_a, src_argb, vl);
    __riscv_vse8_v_u8m2(dst_r, v_r, vl);
    __riscv_vse8_v_u8m2(dst_g, v_g, vl);
    __riscv_vse8_v_u8m2(dst_b, v_b, vl);
    w -= vl;
    dst_r += vl;
    dst_g += vl;
    dst_b += vl;
    src_argb += vl * 4;
  } while (w > 0);
}

void MergeXRGBRow_RVV(const uint8_t* src_r,
                      const uint8_t* src_g,
                      const uint8_t* src_b,
                      uint8_t* dst_argb,
                      int width) {
  size_t w = (size_t)width;
  size_t vl = __riscv_vsetvl_e8m2(w);
  vuint8m2_t v_a = __riscv_vmv_v_x_u8m2(255u, vl);
  do {
    vuint8m2_t v_r, v_g, v_b;
    v_r = __riscv_vle8_v_u8m2(src_r, vl);
    v_g = __riscv_vle8_v_u8m2(src_g, vl);
    v_b = __riscv_vle8_v_u8m2(src_b, vl);
    __riscv_vsseg4e8_v_u8m2(dst_argb, v_b, v_g, v_r, v_a, vl);
    w -= vl;
    src_r += vl;
    src_g += vl;
    src_b += vl;
    dst_argb += vl * 4;
    vl = __riscv_vsetvl_e8m2(w);
  } while (w > 0);
}

#ifdef __cplusplus
}  // extern "C"
}  // namespace libyuv
#endif

#endif  // !defined(LIBYUV_DISABLE_RVV) && defined(__riscv_vector)
