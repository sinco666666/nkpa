#include "FLOAT.h"
#include <stdint.h>
#include <assert.h>

FLOAT F_mul_F(FLOAT a, FLOAT b) {
  //assert(0);
  // 使用64位整数避免溢出
  int64_t result = (int64_t)a * (int64_t)b;
  // 右移16位以获得正确的缩放
  return (FLOAT)(result >> 16);
}

FLOAT F_div_F(FLOAT a, FLOAT b) {
  //assert(0);
  // 使用64位整数避免溢出
  // 左移被除数16位以获得正确的缩放
  int64_t result = ((int64_t)a << 16) / (int64_t)b;
  return (FLOAT)result;
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

  //assert(0);
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
  
  // 调整指数以适应我们的定点表示
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
  //assert(0);
  return a >= 0 ? a : -a;
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
