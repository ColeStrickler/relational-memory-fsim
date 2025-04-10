#ifndef CONFIG_H
#define CONFIG_H

#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#define WRITE_BOOL(addr, value)(*(bool*)(addr) = value)
#define WRITE_UINT8(addr, value)(*(uint8_t*)(addr) = value)
#define WRITE_UINT16(addr, value)(*(uint16_t*)(addr) = value)
#define WRITE_UINT32(addr, value)(*(uint32_t*)(addr) = value)
#define WRITE_UINT64(addr, value)(*(uint64_t*)(addr) = value)

#define READ_BOOL(addr)(*(bool*)(addr))
#define READ_UINT8(addr)(*(uint8_t*)(addr))
#define READ_UINT16(addr)(*(uint16_t*)(addr))
#define READ_UINT32(addr)(*(uint32_t*)(addr))
#define READ_UINT64(addr)(*(uint64_t*)(addr))


#define CL_SIZE        64

#define K              1000
#define KB             1024
#define MB             1024*KB

#define RELCACHE_ADDR  0x110000000UL
#define RELCACHE_SIZE  0x00fffffffUL
#define DRAM_ADDR      0x800000000UL
#define OCM_ADDR       0x00FFFC0000
#define OCM_SIZE       256*KB

#define    LPD0_SIZE  4*KB
#define    LPD0_ADDR  0x80000000


struct col16
{
    unsigned long a;
    unsigned long b;
};

#define T unsigned char
#define MAX_GROUPS 11
#define SIZE (1024 * 1024) // 1MB
#define COLUMN_WIDTH 4
struct _config_db {
    unsigned int   row_size;
    unsigned int   row_count;
    unsigned int   num_columns;
    unsigned int*  column_widths;
    char*          column_types;
    char           store_type;
    char           col_type;
    bool           print;
    unsigned int   min;
    unsigned int   max;
    bool           mvcc_enabled;
    bool           load_file;
    unsigned int column_width;
};

struct _config_query {
    unsigned int enabled_column_number;
    unsigned short col_offsets[MAX_GROUPS];
    unsigned short col_width[MAX_GROUPS];
    int k_value;
    FILE* output_file;
    char* offsets; //just for user input
};


struct experiment_config {
    unsigned int num_samples;             // Number of times to run experiment
    unsigned int r_col;          // fixed column width for the varying row size experiment
    unsigned int c_row;         // fixed row size for the varying column width experiment
};

struct _config {
  unsigned int   row_size; //4B
  unsigned int   row_count; //4B
  unsigned int   reset;//4B
  unsigned int   enabled_col_num;//4B
  unsigned short col_widths[MAX_GROUPS];//We support maximum of 11 columns //22B
  unsigned short col_offsets[MAX_GROUPS];//22B
  unsigned int   frame_offset;//4B
}; 

int open_fd();
void flush_cache();


#endif // CONFIG_H