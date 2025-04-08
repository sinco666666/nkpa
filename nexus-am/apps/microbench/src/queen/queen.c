#include <benchmark.h>

static unsigned int FULL;

static unsigned int dfs(unsigned int row, unsigned int ld, unsigned int rd) {
  if (row == FULL) {
    return 1;
  } else {
    unsigned int pos = FULL & (~(row | ld | rd)), ans = 0;
    while (pos) {
      unsigned int p = (pos & (~pos + 1));
      pos -= p;
      
      // 修复：安全地处理移位操作
      unsigned int new_ld = (ld | p) << 1;
      unsigned int new_rd = (rd | p) >> 1;
      
      ans += dfs(row | p, new_ld, new_rd);
    }
    return ans;
  }
}

static unsigned int ans;

void bench_queen_prepare() {
  ans = 0;
  FULL = (1 << setting->size) - 1;
}

void bench_queen_run() {
  ans = dfs(0, 0, 0);
}

int bench_queen_validate() {
  return ans == setting->checksum;
}
