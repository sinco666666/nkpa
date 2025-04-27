#include "common.h"

#define NAME(key) \
  [_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [_KEY_NONE] = "NONE",
  _KEYS(NAME)
};

size_t events_read(void *buf, size_t len) {
  int key = _read_key();
  bool down = false;

  // 打印读到的原始 key
  printf("[DEBUG] raw key = 0x%x\n", key);

  if (key == _KEY_NONE) {
    sprintf(buf, "t %d\n", _uptime());
    printf("[DEBUG] no key pressed, uptime = %d\n", _uptime());
  } else {
    if (key & 0x8000) {
      down = true;
      key ^= 0x8000;
    }
    printf("[DEBUG] processed key = 0x%x (%s), down = %d\n", key, keyname[key], down);

    if (keyname[key] != NULL) {
      sprintf(buf, "%s %s\n", down ? "kd" : "ku", keyname[key]);
    } else {
      // 如果 keyname[key] 是 NULL，说明 key 超出了范围，输出错误提示
      sprintf(buf, "Unknown key 0x%x\n", key);
      printf("[ERROR] Unknown key: 0x%x\n", key);
    }
  }

  printf("[DEBUG] final buf = %s\n", (char *)buf);
  return strlen(buf);
}



static char dispinfo[128] __attribute__((used));

void dispinfo_read(void *buf, off_t offset, size_t len) {
  strncpy(buf, dispinfo+offset, len);
}

void fb_write(const void *buf, off_t offset, size_t len) {
  int row = (offset/4)/_screen.width;
	int col = (offset/4)%_screen.width;
	_draw_rect(buf,col,row,len/4,1);
}

void init_device() {
  _ioe_init();

  // TODO: print the string to array `dispinfo` with the format
  // described in the Navy-apps convention
  sprintf(dispinfo,"WIDTH:%d\nHEIGHT:%d\n",_screen.width,_screen.height);
}
