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
extern void fb_write(const void *buf, off_t offset, size_t len);
extern void ramdisk_read(void *buf, off_t offset, size_t len);
extern void ramdisk_write(const void *buf, off_t offset, size_t len);
extern void dispinfo_read(void *buf, off_t offset, size_t len);
extern size_t events_read(void *buf, size_t len);

void init_fs() {
  // TODO: initialize the size of /dev/fb
	file_table[FD_FB].size = _screen.height * _screen.width * 4;
}

size_t fs_filesz(int fd) {
	return file_table[fd].size;
}

int fs_open(const char* filename, int flags, int mode) {
  for(int i=0; i<NR_FILES; i++) {
    if(strcmp(filename, file_table[i].name)==0) {
      Log("loading %s", filename);
      file_table[i].open_offset = 0;
      return i;
    }
  }
  panic("This filename not exist");
  return -1;
}

ssize_t fs_read(int fd, void *buf, size_t len) {
	ssize_t fs_size = fs_filesz(fd);
	if (file_table[fd].open_offset + len > fs_size) //偏移量不可以超过文件边界 超出部分舍弃
		len = fs_size - file_table[fd].open_offset;
	switch(fd) {
		case FD_STDOUT:
		case FD_STDERR:
		case FD_STDIN:
			return 0;
		case FD_EVENTS:
			len = events_read((void *)buf, len);
			break;
		case FD_DISPINFO:
			dispinfo_read(buf, file_table[fd].open_offset, len);
			file_table[fd].open_offset += len;	
			break;
		default:
			ramdisk_read(buf, file_table[fd].disk_offset + file_table[fd].open_offset, len);
			file_table[fd].open_offset += len;
			break;
	}
	return len;
}

ssize_t fs_write(int fd, const void *buf, size_t len) {
	ssize_t fs_size = fs_filesz(fd);
	switch(fd) {
		case FD_STDOUT:
		case FD_STDERR:
			for(int i = 0; i < len; i++) {
				_putc(((char*)buf)[i]);
			}
			break;
		case FD_FB:
			fb_write(buf, file_table[fd].open_offset, len);
			file_table[fd].open_offset += len;
			break;
		default:
			if(file_table[fd].open_offset + len > fs_size)
				len = fs_size - file_table[fd].open_offset;
			ramdisk_write(buf, file_table[fd].disk_offset + file_table[fd].open_offset, len);
			file_table[fd].open_offset += len;
			break;
	}
	return len;
}

off_t fs_lseek(int fd, off_t offset, int whence) {
	off_t result = -1;
	switch(whence) {
		case SEEK_SET:
			if (offset >= 0 && offset <= file_table[fd].size) {
				file_table[fd].open_offset = offset;
				result = file_table[fd].open_offset;
			}
			break;
		case SEEK_CUR:
			if ((offset + file_table[fd].open_offset >= 0) && (offset + file_table[fd].open_offset <= file_table[fd].size)) {
				file_table[fd].open_offset += offset;
				result = file_table[fd].open_offset;
			}
			break;
		case SEEK_END:
			file_table[fd].open_offset = file_table[fd].size + offset;
			result = file_table[fd].open_offset;
			break;
	}
	
	return result;
}
int fs_close(int fd) {
	return 0;
}