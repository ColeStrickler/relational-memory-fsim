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
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include "config.h"
#include "print_utils.h"

// #define KB      1024
// #define MB      KB*KB
// #define WORD_SIZE 128


#define HIGH_DDR_ADDR 0x800000000
unsigned char* db_mapping = NULL;
#define BUS_WIDTH      16

unsigned int get_uniform(unsigned int rangeLow, unsigned int rangeHigh) {
    double myRand = rand() / (1.0 + RAND_MAX);
    unsigned int range = rangeHigh - rangeLow + 1;
    unsigned int myRand_scaled = (myRand * range) + rangeLow;
    return myRand_scaled;
}

void generate_db(struct _config_db config) {
    srand(1);
    unsigned check_row_size = 0;

    // Use 'config.num_columns' and 'config.column_widths'
    for (int i = 0; i < config.num_columns; i++) {
        check_row_size += config.column_widths[i];
    }

    unsigned row_size = check_row_size;

    // Use 'config.row_count' instead of 'row_count'
    unsigned db_size = config.row_count * row_size;
        
    // #ifdef linux
    //     unsigned char* db = (unsigned char *) malloc ( db_size * sizeof(unsigned char) );
    // #else
    //     int hpm_fd = open_fd();
    //     unsigned char* db = mmap((void*)0, db_size, PROT_EXEC|PROT_READ|PROT_WRITE, MAP_SHARED, hpm_fd, HIGH_DDR_ADDR); //Uncached mapping
    // #endif

    int hpm_fd = open_fd();
    //unsigned char* db = mmap((void*)0, db_size, PROT_EXEC|PROT_READ|PROT_WRITE, MAP_SHARED, hpm_fd, HIGH_DDR_ADDR); //Uncached mapping
    //unsigned char* db = malloc(db_size); // NEED TO MMAP THIS, MAKE CONTIGUOUS
    
    #ifdef USE_MALLOC
    // Code for non-x86 architectures
     printf("using malloc\n");
    unsigned char* db = malloc(db_size);
                               
    #elif !defined(__x86_64__) && !defined(__i386__)
    
    unsigned char* db = mmap(0, 
				       db_size,
				       PROT_READ | PROT_WRITE | PROT_EXEC, 
				       MAP_SHARED, 
				       hpm_fd, 0xf0000000);

    #else
    // Code for x86 architectures
    printf("using malloc\n");
    unsigned char* db = malloc(db_size);
    
    #endif
    
    /*
        reserved-memory {
            #address-cells = <2>;
            #size-cells = <2>;
            ranges;

            my_reserved_memory: memory@f0000000 {
                reg = <0x0 0xf0000000 0x0 0x01000000>;  // Example: 16MB region at address 0x80000000
                no-map;  // Optional: Prevent the memory region from being mapped by the kernel
            };
        }; 

        cat /proc/iomem
        10015000-10015fff : 10015000.blkdev-controller control
        54000000-54000fff : 54000000.serial control
        80000000-8003ffff : Reserved
        80200000-17fffffff : System RAM
        80202000-813546a7 : Kernel image
        80202000-808e7acd : Kernel code
        80e00000-80ffffff : Kernel rodata
        81200000-812e35ff : Kernel data
        812e4000-813546a7 : Kernel bss
    */


    db_mapping = db;
    if (db == NULL)
    {
        printf("generate_db() mmmap failed\n");
        return;
    }
    for (int i = 0; i < db_size; i++) {
        db[i] = 0;            
    }

    __uint128_t value = 0;
    int offset = 0;


    // Use 'config.num_columns'
    for (int j = 0; j < config.num_columns; j++) {

        // Use 'config.row_count'
        for (int i = 0; i < config.row_count; i++) {
            if (config.column_types[j] == 's') {
                value = (__uint128_t)i;
            }
            else if (config.column_types[j] == 'r') {
                value = get_uniform(config.min, config.max);
            }
            else if (config.column_types[j] == 'z') {
                value = 0;
            }

            // Row Store
            if (config.store_type == 'r') {
                // Use 'config.column_widths'
                if (config.column_widths[j] == 1) {
                    *(unsigned char*)(db + (i * row_size) + offset) = (unsigned char)value;
                    
                }
                else if (config.column_widths[j] == 2) {
                  
                    *(unsigned short*)(db + (i * row_size) + offset) = (unsigned short)value;
                   
                }
                else if (config.column_widths[j] == 4) {
           
                    *(unsigned int*)(db + (i * row_size) + offset) = (unsigned int)value;
                    
                }
                else if (config.column_widths[j] == 8) {
    		        *(unsigned long*)( db + (i*row_size) + offset) = (unsigned long)value;
                }
                else {
                    for (int k = 0; k < config.column_widths[j]; k++) {
                       
                        *(unsigned char*)(db + (i * row_size) + offset + k) = (unsigned char)value;
                        if (config.column_types[j] == 's') {
                            value = value / 64;
                        }
                        
                    }
                }
            }
            // Column Store
            else if (config.store_type == 'c') {
                if (config.column_widths[j] == 1) {
                    *(unsigned char*)(db + config.row_count * offset + i * config.column_widths[j]) = (unsigned char)value;
                }
                else if (config.column_widths[j] == 2) {
                    *(unsigned short*)(db + config.row_count * offset + i * config.column_widths[j]) = (unsigned short)value;
                }
                else if (config.column_widths[j] == 4) {
                    *(unsigned int*)(db + config.row_count * offset + i * config.column_widths[j]) = (unsigned int)value;
                }
                else if (config.column_widths[j] == 8) {
                    *(unsigned long*)(db + config.row_count * offset + i * config.column_widths[j]) = (unsigned long)value;
                }
                else {
                    for (int k = 0; k < config.column_widths[j]; k++) {
                        *(unsigned char*)(db + config.row_count * offset + i * config.column_widths[j] + k) = (unsigned char)value;
                        if (config.column_types[j] == 's') {
                            value = value / 64;
                        }
                    }
                }
            }
        }
        offset += config.column_widths[j];


    }

    // print DB
    if (config.print == true) {
        print_db(config, db, row_size);
    }
    //if (munmap(db, db_size) == -1) {
    //perror("Error unmapping the memory");
    // Handle the error as appropriate
    //}
    //free()
}









