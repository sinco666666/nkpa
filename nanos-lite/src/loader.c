#include "common.h"

#define DEFAULT_ENTRY ((void *)0x8048000)
extern void ramdisk_read(void *buf, off_t offset, size_t len);
extern size_t get_ramdisk_size();

int fs_open(const char *pathname, int flags, int mode);
size_t fs_filesz(int fd);
ssize_t fs_read(int fd, void *buf, size_t len);
int fs_close(int fd);

void* new_page(void);

uintptr_t loader(_Protect *as, const char *filename) {
  // size_t size = get_ramdisk_size();
  // void * buff = DEFAULT_ENTRY;
  // ramdisk_read(buff,0,size);
  // return (uintptr_t)buff;
  int fd = fs_open(filename, 0, 0);        //打开文件
  //fs_read(fd, DEFAULT_ENTRY, fs_filesz(fd)); //读文件
  int size = fs_filesz(fd);
  void *pa, *va = DEFAULT_ENTRY;
  Log("filename=%s, fd=%d", filename, fd);
  while(size > 0) {
    pa = new_page();
    _map(as, va, pa);
    size_t len = size >= PGSIZE ? PGSIZE : size;
    fs_read(fd, pa, len);
    va += PGSIZE;
    size -= PGSIZE;
  }
  fs_close(fd);                             //关闭文件
  return (uintptr_t)DEFAULT_ENTRY;
}
