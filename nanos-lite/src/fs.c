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

// extern void fb_write(const void *buf, off_t offset, size_t len);
// extern void ramdisk_write(const void *buf, off_t offset, size_t len);
// size_t fs_filesz(int fd) {
// 	return file_table[fd].size;
// }
// ssize_t fs_write(int fd, const void *buf, size_t len) {
// 	ssize_t fs_size = fs_filesz(fd);
// 	switch(fd) {
// 		case FD_STDOUT:
// 		case FD_STDERR:
// 			for(int i = 0; i < len; i++) {
// 				_putc(((char*)buf)[i]);
// 			}
// 			break;
// 		case FD_FB:
// 			fb_write(buf, file_table[fd].open_offset, len);
// 			file_table[fd].open_offset += len;
// 			break;
// 		default:
// 			if(file_table[fd].open_offset + len > fs_size)
// 				len = fs_size - file_table[fd].open_offset;
// 			ramdisk_write(buf, file_table[fd].disk_offset + file_table[fd].open_offset, len);
// 			file_table[fd].open_offset += len;
// 			break;
// 	}
// 	return len;
// }