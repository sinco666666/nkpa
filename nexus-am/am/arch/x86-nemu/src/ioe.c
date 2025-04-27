#include <am.h>
#include <x86.h>

#define RTC_PORT 0x48   // Note that this is not standard
static unsigned long boot_time;

void _ioe_init() {
  boot_time = inl(RTC_PORT);
}

unsigned long _uptime() {
  return inl(RTC_PORT)-boot_time;
}

uint32_t* const fb = (uint32_t *)0x40000;

_Screen _screen = {
  .width  = 400,
  .height = 300,
};

extern void* memcpy(void *, const void *, int);

void _draw_rect(const uint32_t *pixels, int x, int y, int w, int h) {
  // int i;
  // for (i = 0; i < _screen.width * _screen.height; i++) {
  //   fb[i] = i;
  // }
  int i;
  for(i=0;i<h;i++)
      memcpy(fb+(y+i)*_screen.width+x,pixels+i*w,w*4);
}

void _draw_sync() {
}

// 简单 putch 输出一个字符
void putch(char ch) {
  outb(0x3f8, ch); // 向串口 COM1 发送一个字节
}

// 简单打印 16 进制
void print_hex(uint32_t val) {
  const char *hex = "0123456789ABCDEF";
  for (int i = 7; i >= 0; i--) {
    putch(hex[(val >> (i * 4)) & 0xF]);
  }
}

// 修改后的 _read_key 带调试输出
int _read_key() {
  if (inb(0x64)) {
    uint32_t keycode = inl(0x60);
    putch('['); putch('D'); putch('E'); putch('B'); putch('U'); putch('G'); putch(']'); putch(' ');
    putch('k'); putch('e'); putch('y'); putch('c'); putch('o'); putch('d'); putch('e'); putch('=');
    print_hex(keycode);
    putch('\n');
    return keycode;
  } else {
    return _KEY_NONE;
  }
}
