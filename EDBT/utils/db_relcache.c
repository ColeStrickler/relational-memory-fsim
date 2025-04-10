#define _GNU_SOURCE
#include "config.h" 
#include "print_utils.h"


#define RME_CONFIG                  0x3000000
#define RME_CONFIG_SIZE             0xfff
#define RME_EN(base)				        ((uint64_t)base)
#define RME_ROWSIZE(base)			      ((uint64_t)base + 0x10)
#define RME_EN_COL(base)			      ((uint64_t)base + 0x30)
#define RME_COL_WIDTH(base)		      ((uint64_t)base + 0x40)
#define RME_COL_OFFSET(base, i)	    ((uint64_t)base + i * 0x10 + 0x48)
#define RME_RESET(base)             ((uint64_t)base + 16 * 0x10 + 0x48)

int configure_relcache(struct _config_db config_db, struct _config_query *params) {

  flush_cache();
  // added this for testing


  printf("configure_relcache()\n");
    unsigned int   frame_offset = 0;
    int lpd_fd  = open_fd();
    printf("got lpd_fd %d\n", lpd_fd);
    struct _config* config = (struct _config *)mmap(NULL, RME_CONFIG_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, lpd_fd, RME_CONFIG);
    if (!config)
    {
        printf("could not map relcache config\n");
        return -1;
    }
    printf("Attempting to configure RME\n");
    printf("row size %d\n", config_db.row_size);
    WRITE_UINT32(RME_ROWSIZE(config), config_db.row_size);
    printf("Wrote row size\n");
    WRITE_UINT16(RME_EN_COL(config), params->enabled_column_number);
    WRITE_UINT16(RME_COL_WIDTH(config), config_db.column_width);
    printf("Wrote enabled col number %d\n", params->enabled_column_number);

    unsigned short sum_col_offsets = 0;
    for(int i=0; i<params->enabled_column_number; i+=1){

      printf("Offset %d\n", params->col_offsets[i]);
      WRITE_UINT16(RME_COL_OFFSET(config, i), params->col_offsets[i] - sum_col_offsets); // i think this should work.
      printf("Offset %d\n", params->col_offsets[i]);
      sum_col_offsets = params->col_offsets[i];  
    }

    /*
    config->row_size = config_db.row_size;
    config->row_count = config_db.row_count;
    config->enabled_col_num = params->enabled_column_number;
    
    for(int i=0; i<params->enabled_column_number; i+=1){
      config->col_widths[i] = config_db.column_widths[i];
    }
    unsigned short sum_col_offsets = 0;
    for(int i=0; i<params->enabled_column_number; i+=1){
      config->col_offsets[i] = params->col_offsets[i] - sum_col_offsets;
      sum_col_offsets = params->col_offsets[i];   
    }

    config->frame_offset = frame_offset;
    */
    //print_config_info(config);
    
    WRITE_BOOL(RME_EN(config), 1);
    close(lpd_fd);
    int unmap_result = munmap(config, RME_CONFIG_SIZE);
    return unmap_result;
}

#define __dsb(){\
  do{\
    asm volatile("dsb 15");\
  }while(0);\
}

int reset_relcache(unsigned int frame_offset) {  
    printf("Resetting RME\n");
    int lpd_fd  = open_fd();
    printf("got lpd_fd %d\n", lpd_fd);
    struct _config* config = (struct _config *)mmap(NULL, RME_CONFIG_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, lpd_fd, RME_CONFIG);
    //__dsb();
    // reset
    
    WRITE_BOOL(RME_RESET(config), 1);

    while (READ_BOOL(RME_RESET(config)) != 1)
    {

    }
    WRITE_BOOL(RME_RESET(config), 0);
    /*
        config->frame_offset = frame_offset; 
        unsigned int reset_data = config->reset;
        config->reset = (reset_data + 1) & 0x1; 
    */
    //__dsb();
    //unmap
    int unmap_result = munmap(config, RME_CONFIG_SIZE);
    close(lpd_fd);
    return unmap_result;
}


int EnableRelCache(int fd)
{
    return 0;
    printf("EnableRelCache()\n");
    int lpd_fd  = open_fd();

    struct _config* config = (struct _config *)mmap(NULL, RME_CONFIG_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, lpd_fd, RME_CONFIG);
    printf("EnableRelCache() Got config 0x%x\n", (uint64_t)config);
    WRITE_BOOL(RME_EN(config), 1);
    printf("Wrote config!\n");
    while(READ_BOOL(RME_EN(config)) != 1)
    {
      printf("Wrote config 0x%x\n", READ_BOOL(RME_EN(config)));
    }



  int unmap_result = munmap(config, RME_CONFIG_SIZE);
  printf("EnableRelCache() done\n");
  return unmap_result;
}


volatile void FlushAndDisable(int fd)
{
   printf("FlushAndDisable()\n");
    //int lpd_fd  = open_fd();
    struct _config* config = (struct _config *)mmap(NULL, RME_CONFIG_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, RME_CONFIG);
    WRITE_BOOL(RME_EN(config), 0);
    while(READ_BOOL(RME_EN(config)) != 0)
    {
      
    }



  int unmap_result = munmap(config, RME_CONFIG_SIZE);
  
  flush_cache();
  printf("FlushAndDisable() done\n");
}