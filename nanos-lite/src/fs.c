#include "fs.h"

typedef struct {
  char *name;
  size_t size;
  off_t disk_offset;
  off_t open_offset;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB, FD_EVENTS, FD_DISPINFO, FD_NORMAL};

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  {"stdin (note that this is not the actual stdin)", 0, 0},
  {"stdout (note that this is not the actual stdout)", 0, 0},
  {"stderr (note that this is not the actual stderr)", 0, 0},
  [FD_FB] = {"/dev/fb", 0, 0},
  [FD_EVENTS] = {"/dev/events", 0, 0},
  [FD_DISPINFO] = {"/proc/dispinfo", 128, 0},
#include "files.h"
};

#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))

void init_fs() {
  // TODO: initialize the size of /dev/fb
}

extern void fb_write(const void *buf, off_t offset, size_t len);
extern void ramdisk_write(const void *buf, off_t offset, size_t len);
size_t fs_filesz(int fd) {
	return file_table[fd].size;
}
off_t disk_offset(int fd) {
  assert(fd>=0 && fd<NR_FILES);
  return file_table[fd].disk_offset;
}

off_t get_open_offset(int fd) {
  assert(fd>=0 && fd<NR_FILES);
  return file_table[fd].open_offset;
}

void set_open_offset(int fd, off_t n) {
  assert(fd>=0 && fd<NR_FILES);
  assert(n>=0);
  if(n > file_table[fd].size)
    n = file_table[fd].size;
  file_table[fd].open_offset = n;
}
ssize_t fs_write(int fd, void* buf, size_t len) {
  assert(fd>=0 && fd<NR_FILES);
  if (fd < 3 || fd==FD_DISPINFO) {
    Log("arg invaid:fd<3 || fd==FD_DISPINFO");
    return 0;
  }
  int n = fs_filesz(fd) - get_open_offset(fd);
  if (n > len)
    n = len;

  if (fd == FD_FB)
    fb_write(buf, get_open_offset(fd), n);
  else
    ramdisk_write(buf, disk_offset(fd)+get_open_offset(fd), n);
  set_open_offset(fd, get_open_offset(fd)+n);
  return n;
}