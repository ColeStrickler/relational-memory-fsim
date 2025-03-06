#define _GNU_SOURCE
#include "exp_header.h"
#include "performance_counters.h"

/*
  Before this we were reading 4 bytes every time no matter what column size
*/
static inline uint64_t readData(struct _config_db* config_db, unsigned char* addr)
{
  switch(config_db->column_width)
  {
    case 1: return READ_UINT8(addr);
    case 2: return READ_UINT16(addr);
    case 4: return READ_UINT32(addr);
    case 8: return READ_UINT64(addr);
    default:
    {
      perror("readData() Error config_db->column_width is bad value\n");
      exit(-1);
    }
  }
}





void run_query1(struct _config_db config_db, struct _config_query params) {

    unsigned int cycleHi    = 0, cycleLo=0;
    struct perf_counters res, start, end;
    int fd = setup_pmcs();
    if (fd < 0)
        perror("Issue opening PMC FDs\n");

    bool mvcc_enabled = false;
    T *cold_array = malloc(config_db.row_count * params.enabled_column_number * sizeof(T));
    T *hot_array = malloc(config_db.row_count * params.enabled_column_number * sizeof(T));
    T *row_array = malloc(config_db.row_count * params.enabled_column_number * sizeof(T));
    unsigned sum_col_width = 0;
    for ( int i=0 ; i<params.enabled_column_number ; i++ ) {
        sum_col_width += config_db.column_widths[i];
    }
    //-- pasring arguments done --------------------------------------

    unsigned dram_size  = config_db.row_count*config_db.row_size;
    int hpm_fd          = open_fd();
   // int dram_fd         = open_fd();

    //mapping fpga:

    // we took out their extra flag
    printf("mapping plim\n");
    unsigned char* plim = mmap((void*)0, RELCACHE_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, hpm_fd, RELCACHE_ADDR);
    printf("mapped plim\n");
    //mapping dram
    printf("mapping DRAM\n");
    unsigned char* dram = plim;//mmap((void*)0, RELCACHE_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, dram_fd, RELCACHE_ADDR);
     printf("mapped DRAM\n");


    T data;
    T data_count = 0;

    uint64_t row_size = config_db.row_size;
    uint64_t bytes_per_row = params.enabled_column_number * config_db.column_width;
    uint64_t stride_access = (64 / (row_size/bytes_per_row)) * 64;

    uint64_t sum_rme = 0;
    if ( config_db.store_type == 'r' ){
        // move multiplication outside
        EnableRelCache(hpm_fd);
        unsigned width = config_db.column_widths[0];
        pmcs_get_value(&start);
      //  magic_timing_begin(&cycleLo, &cycleHi);
        for(int i = 0; i < config_db.row_count; i++){
          unsigned offset = 0;
          for(int j=0; j<params.enabled_column_number; j++){
            /* cold_array[data_count] */ sum_rme += *(T*)(plim + i*sum_col_width + width*j);
            //cold_array[data_count] = readData(&config_db, (plim + i*sum_col_width + width*j));
            data_count++;
          }    
        }
      //  magic_timing_end(&cycleLo, &cycleHi);
        pmcs_get_value(&end);
        res = pmcs_diff(&end, &start);
        fprintf(params.output_file,"q1, r, c, %d, %d, %d, %d, %d, %lu, %lu, %lu, %lu, %lu,%llu\n", params.enabled_column_number, config_db.row_size, config_db.row_count, config_db.column_widths[0], res.cycles, res.l1_references, res.l1_refills, res.l2_references, res.l2_refills, res.inst_retired, res.time.tv_sec*1000000000L+res.time.tv_nsec);
        data_count = 0;

        FlushAndDisable(hpm_fd);

        

        //pmcs_get_value(&start);
        //magic_timing_begin(&cycleLo, &cycleHi);
        
        
        //for(int i = 0; i < config_db.row_count; i++){
        //  unsigned offset = 0;
        //  for(int j=0; j<params.enabled_column_number; j++){
        //    hot_array[data_count] = *(T*)(plim + i*sum_col_width + width*j);
        //    data_count++;
        //  }   
        //}
    


        


      uint64_t sum_row = 0;
       // magic_timing_end(&cycleLo, &cycleHi);
       // pmcs_get_value(&end);
        //res = pmcs_diff(&end, &start);
        //fprintf(params.output_file,"q1, c, -, %d, %d, %d, %d, %d, %lu, %lu, %lu, %lu, %lu,%llu\n", params.enabled_column_number, config_db.row_size, config_db.row_count, config_db.column_widths[0], res.cycles, res.l1_references, res.l1_refills, res.l2_references, res.l2_refills, res.inst_retired, res.time.tv_sec*1000000000L+res.time.tv_nsec);
        data_count = 0;
        pmcs_get_value(&start);
       // magic_timing_begin(&cycleLo, &cycleHi);
        for(int i = 0; i < config_db.row_count; i++){
          for(int j=0; j<params.enabled_column_number; j++){
               /*row_array[data_count]*/sum_row += *(T*)(dram + i*config_db.row_size + params.col_offsets[j]);
               //cold_array[data_count] = readData(&config_db, dram + i*config_db.row_size + params.col_offsets[j]);
               data_count++;
          }
        }

        
       // magic_timing_end(&cycleLo, &cycleHi);
        pmcs_get_value(&end);
        res = pmcs_diff(&end, &start);
       fprintf(params.output_file,"q1, d, -, %d, %d, %d, %d, %d, %lu, %lu, %lu, %lu, %lu,%llu\n", params.enabled_column_number, config_db.row_size, config_db.row_count, config_db.column_widths[0], res.cycles, res.l1_references, res.l1_refills, res.l2_references, res.l2_refills, res.inst_retired, res.time.tv_sec*1000000000L+res.time.tv_nsec);
        if (config_db.print == true){
            printf("\nRow store query results:\n");
            printf("cold, hot, ROW\n");
            for (unsigned int i = 0; i < data_count; i++) {
                printf("%d, %d, %d\n", cold_array[i], hot_array[i], row_array[i]);
            }
        }

        printf("rme %lld\n", sum_rme);
        printf("row %lld\n", sum_row);
        free(cold_array);
        free(hot_array);
        free(row_array);
    }
    else if ( config_db.store_type == 'c' ){

        FlushAndDisable(hpm_fd);
        uint64_t sum_col = 0;
        T *col_array = malloc(config_db.row_count * params.enabled_column_number * sizeof(T));
        data_count = 0;
        pmcs_get_value(&start);
        //magic_timing_begin(&cycleLo, &cycleHi);
        for (int i = 0; i < config_db.row_count; i++) {
            for (int j = 0; j < params.enabled_column_number; j++) {
                // for now only for projectivity experiment
                /*col_array[data_count++]*/sum_col += *(T*) (dram + (i + j * config_db.row_count) * sizeof(T));
                //col_array[data_count++] = readData(&config_db, dram + (i + j * config_db.row_count) * config_db.column_width);
            }
        }
      //  magic_timing_end(&cycleLo, &cycleHi);
        pmcs_get_value(&end);
        res = pmcs_diff(&end, &start);
        fprintf(params.output_file,"q1, c, -, %d, %d, %d, %d, %d, %lu, %lu, %lu, %lu, %lu,%llu\n", params.enabled_column_number, config_db.row_size, config_db.row_count, config_db.column_widths[0], res.cycles, res.l1_references, res.l1_refills, res.l2_references, res.l2_refills, res.inst_retired, res.time.tv_sec*1000000000L+res.time.tv_nsec);
        if (config_db.print == true){
            printf("\nColumn store query results:\n");
            for (unsigned int i = 0; i < data_count; i++) {
                printf("%d \n", col_array[i]);
            }
        }

        printf("sum col %lld\n", sum_col);
        free(col_array);
    }

    int ret = teardown_pmcs();
    if (ret < 0)
        perror("Issue detected while tearing down the PMCs\n");

    fflush(params.output_file);

    munmap(plim, RELCACHE_SIZE);
    //munmap(dram, dram_size);

    close(hpm_fd);
    //close(dram_fd);
}




























