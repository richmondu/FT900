#ifndef SDCARD_H
#define SDCARD_H

#include "ff.h" // fatfs


void sdcard_setup(void);
int  sdcard_open(FIL* f, const char* filename, int bWrite, int bRead);
void sdcard_dir(char* path);
#define sdcard_read  f_read
#define sdcard_write f_write
#define sdcard_close f_close
#define sdcard_size  f_size
#define sdcard_lseek f_lseek


#endif // SDCARD_H
