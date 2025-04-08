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
      
      // 确保移位不会导致溢出
      unsigned int new_ld = (ld | p);
      unsigned int new_rd = (rd | p);
      
      // 检查移位是否会导致问题
      if ((new_ld << 1) < new_ld || (new_rd >> 1) > new_rd) {
        // 处理潜在的溢出/下溢
        continue;
      }
      
      ans += dfs(row | p, new_ld << 1, new_rd >> 1);
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
