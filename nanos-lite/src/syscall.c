#include "common.h"
#include "syscall.h"

int fs_open(const char *pathname, int flags, int mode);
ssize_t fs_read(int fd, void *buf, size_t len);
ssize_t fs_write(int fd, const void *buf, size_t len);
off_t fs_lseek(int fd, off_t offset, int whence);
int fs_close(int fd);
static inline _RegSet* sys_write(_RegSet *r){
  int fd = (int)SYSCALL_ARG2(r);
  char *buf = (char *)SYSCALL_ARG3(r);
  int len = (int)SYSCALL_ARG4(r);
  Log("");
  if(fd == 1 || fd == 2){
      for(int i = 0; i < len; i++) {
          _putc(buf[i]);
      }
      SYSCALL_ARG1(r) = SYSCALL_ARG4(r);
  }
  return NULL;
}
_RegSet* do_syscall(_RegSet *r) {
  uintptr_t a[4];
  a[0] = SYSCALL_ARG1(r);

  switch (a[0]) {
    case SYS_none: SYSCALL_ARG1(r) =1; break;
    case SYS_exit: _halt(SYSCALL_ARG2(r)); break;
    case SYS_write: return sys_write(r);
    case SYS_brk:  SYSCALL_ARG1(r) = 0; break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}
