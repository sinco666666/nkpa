#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];

    memset(wp_pool[i].expression, 0, sizeof(wp_pool[i].expression));
    wp_pool[i].val = 0;
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP* new_wp() {
  // 如果没有空闲的监视点，则返回NULL
  if (free_ == NULL) {
    printf("No free watchpoint available!\n");
    return NULL;
  }
  // 从free_链表头部取出一个监视点
  WP *wp = free_;
  free_ = free_->next;
  
  // 初始化监视点的其他成员
  memset(wp->expression, 0, sizeof(wp->expression));
  wp->val = 0;

  // 将该监视点插入到head链表中，head链表表示正在使用的监视点
  wp->next = head;
  head = wp;
  
  return wp;
}

void free_wp(WP *wp) {
  if (wp == NULL) return;

  // 从head链表中删除该监视点
  if (head == wp) {
    head = head->next;
  } else {
    WP *cur = head;
    while (cur != NULL && cur->next != wp) {
      cur = cur->next;
    }
    if (cur != NULL) {
      cur->next = wp->next;
    }
  }
  
  memset(wp->expression, 0, sizeof(wp->expression));
  wp->val = 0;
  
  // 将监视点重新归还到free_链表头部
  wp->next = free_;
  free_ = wp;
}


void printWP() {
  printf("Num        Expr        Value\n");
  for (WP *wp = head; wp != NULL; wp = wp->next) {
    printf("%-8d %-16s 0x%08x\n", wp->NO, wp->expression, wp->val);
  }
}

bool watch_wp() {
  bool changed = false;
  for (WP *wp = head; wp != NULL; wp = wp->next) {
    bool success = false;
    uint32_t new_val = expr(wp->expression, &success);
    if (!success) {
      printf("Error evaluating expression: %s\n", wp->expression);
    } else if (new_val != wp->val) {
      printf("Watchpoint %d: %s changed from 0x%08x to 0x%08x\n", wp->NO, wp->expression, wp->val, new_val);
      wp->val = new_val;
      changed = true;
    }
  }
  return changed;
}

WP* get_head(){
  return head;
}

WP* get_free_(){
  return free_;
}