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
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

void printWP() {
    WP *wp = head;

    // 打印表头
    printf("NUM           Expr           Value\n");

    // 遍历链表并打印每个 watchpoint 的信息
    while (wp != NULL) {
        printf("%-12d  %-16s  %u\n", wp->NO, wp->expression, wp->val);
        wp = wp->next;
    }
}
