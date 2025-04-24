#include "common.h"
#include "syscall.h"

ssize_t fs_write(int fd, const void *buf, size_t len);
static inline _RegSet* sys_write(_RegSet *r) {
  int fd = (int)SYSCALL_ARG2(r);
  char* buf = (char*)SYSCALL_ARG3(r);
  size_t count = (size_t)SYSCALL_ARG4(r);
  if (fd == 1 || fd == 2) {
    for(int i = 0; i < count; i++){
      _putc(buf[i]);
    }
    SYSCALL_ARG1(r) = count;
  }
  if (fd >= 3) {
    SYSCALL_ARG1(r) = fs_write(fd, buf, count);
  }
  return NULL; 
}
_RegSet* do_syscall(_RegSet *r) {
  uintptr_t a[4];
  a[0] = SYSCALL_ARG1(r);

  switch (a[0]) {
    case SYS_none: SYSCALL_ARG1(r) =1; break;
    case SYS_exit: _halt(SYSCALL_ARG2(r)); break;
    case SYS_write:return sys_write(r);
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}
