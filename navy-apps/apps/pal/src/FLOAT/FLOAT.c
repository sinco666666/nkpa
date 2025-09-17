#include "FLOAT.h"
#include <stdint.h>
#include <assert.h>

FLOAT F_mul_F(FLOAT a, FLOAT b) {
  // assert(0);
  // return 0;
  // 保存符号
    int sign = 1;
    if (a < 0) {
        sign = -sign;
        a = -a;
    }
    if (b < 0) {
        sign = -sign;
        b = -b;
    }
    
    // 分解a和b为高16位和低16位
    int a_hi = a >> 16;
    int a_lo = a & 0xFFFF;
    int b_hi = b >> 16;
    int b_lo = b & 0xFFFF;
    
    // 计算四个部分
    int hi_hi = a_hi * b_hi;
    int hi_lo = a_hi * b_lo;
    int lo_hi = a_lo * b_hi;
    int lo_lo = a_lo * b_lo;
    
    // 组合结果，注意右移16位
    int result = (hi_hi << 16) + hi_lo + lo_hi + (lo_lo >> 16);
    
    // 应用符号
    return sign > 0 ? result : -result;
}

FLOAT F_div_F(FLOAT a, FLOAT b) {
  // assert(0);
  // return 0;
  // 处理特殊情况
    if (b == 0) {
        assert(0);
        return 0;
    }
    
    // 保存符号
    int sign = 1;
    if (a < 0) {
        sign = -sign;
        a = -a;
    }
    if (b < 0) {
        sign = -sign;
        b = -b;
    }
    
    // 直接计算整数部分
    FLOAT q = a / b;
    
    // 计算余数
    FLOAT r = a % b;
    
    // 计算小数部分
    FLOAT f = 0;
    int mask = 0x8000; // 最高位的小数位
    
    for (int i = 0; i < 16; i++) {
        r <<= 1;
        if (r >= b) {
            f |= mask;
            r -= b;
        }
        mask >>= 1;
    }
    
    // 最终结果
    FLOAT result = (q << 16) | f;
    
    // 应用符号
    return sign > 0 ? result : -result;
}

FLOAT f2F(float a) {
  /* You should figure out how to convert `a' into FLOAT without
   * introducing x87 floating point instructions. Else you can
   * not run this code in NEMU before implementing x87 floating
   * point instructions, which is contrary to our expectation.
   *
   * Hint: The bit representation of `a' is already on the
   * stack. How do you retrieve it to another variable without
   * performing arithmetic operations on it directly?
   */

  // assert(0);
  // return 0;
  // 使用指针获取float的位表示，避免使用浮点操作
    int *p = (int *)&a;
    int i = *p;
    
    int sign = (i >> 31) & 1;
    int exp = ((i >> 23) & 0xFF);
    int frac = i & 0x7FFFFF;
    
    // 处理零的情况
    if (exp == 0 && frac == 0) {
        return 0;
    }
    
    // 调整指数（去除偏移）
    exp = exp - 127;
    
    // 调整指数以适应定点表示
    exp = exp - 23 + 16;
    
    FLOAT result;
    
    if (exp >= 0) {
        result = (frac | 0x800000) << exp;
    } else if (exp >= -24) {
        result = (frac | 0x800000) >> (-exp);
    } else {
        result = 0;
    }
    
    // 应用符号
    if (sign) {
        result = -result;
    }
    
    return result;
}

FLOAT Fabs(FLOAT a) {
  // assert(0);
  // return 0;
  return a < 0 ? -a : a;
}

/* Functions below are already implemented */

FLOAT Fsqrt(FLOAT x) {
  FLOAT dt, t = int2F(2);

  do {
    dt = F_div_int((F_div_F(x, t) - t), 2);
    t += dt;
  } while(Fabs(dt) > f2F(1e-4));

  return t;
}

FLOAT Fpow(FLOAT x, FLOAT y) {
  /* we only compute x^0.333 */
  FLOAT t2, dt, t = int2F(2);

  do {
    t2 = F_mul_F(t, t);
    dt = (F_div_F(x, t2) - t) / 3;
    t += dt;
  } while(Fabs(dt) > f2F(1e-4));

  return t;
}
