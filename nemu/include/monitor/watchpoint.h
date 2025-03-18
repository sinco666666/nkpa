#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  char expression[64];         //表达式
  uint32_t val;                //表达式计算值


} WP;

WP* new_wp();
void free_wp(WP *wp);
void printWP();
WP* get_head();
WP* get_free_();

#endif
