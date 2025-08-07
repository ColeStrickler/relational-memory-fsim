#define _GNU_SOURCE
#include "exp_header.h"
#include "performance_counters.h"
#include "dtl_api.hpp"
#include <fstream>
#include <string>

std::string FileToString(const std::string& file_)
{
  std::ifstream file(file_);
    if (!file) {
        std::cerr << "Failed to open file\n";
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();  // Read entire file into the buffer
    std::string contents = buffer.str();
    return contents;
}


/*
  Before this we were reading 4 bytes every time no matter what column size
*/
static inline uint64_t readData(struct _config_db *config_db,
                                unsigned char *addr) {
  switch (config_db->column_width) {
  case 1:
    return READ_UINT8(addr);
  case 2:
    return READ_UINT16(addr);
  case 4:
    return READ_UINT32(addr);
  case 8:
    return READ_UINT64(addr);
  default: {
    perror("readData() Error config_db->column_width is bad value\n");
    exit(-1);
  }
  }
}

void run_query1(struct _config_db config_db, struct _config_query params) {

  unsigned int cycleHi = 0, cycleLo = 0;
  struct perf_counters res, start, end;
  int fd = setup_pmcs();
  if (fd < 0)
    perror("Issue opening PMC FDs\n");

  bool mvcc_enabled = false;
  T *cold_array =
      (T*)malloc(config_db.row_count * params.enabled_column_number * sizeof(T));
  T *hot_array =
      (T*)malloc(config_db.row_count * params.enabled_column_number * sizeof(T));
  T *row_array =
      (T*)malloc(config_db.row_count * params.enabled_column_number * sizeof(T));
  T *col_array =
      (T*)malloc(config_db.row_count * params.enabled_column_number * sizeof(T));
  unsigned sum_col_width = 0;
  for (int i = 0; i < params.enabled_column_number; i++) {
    sum_col_width += config_db.column_widths[i];
  }
  //-- pasring arguments done --------------------------------------

  unsigned dram_size = config_db.row_count * config_db.row_size;
  int hpm_fd = open_fd();
  // int dram_fd         = open_fd();

  // mapping fpga:

  // we took out their extra flag
  printf("mapping plim\n");

  auto hwStat = new DTL::AGUHardwareStat(4, 4, 5, 6, 6, 4, 8);
  DTL::API api(hwStat);
  
  




  unsigned long *config =     (unsigned long*)mmap(NULL, RME_CONFIG_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, hpm_fd, RME_CONFIG);
  void* agu_config_base =  mmap(NULL, 0xfff, PROT_READ|PROT_WRITE, MAP_SHARED, hpm_fd, 0x4000000);  
  assert(agu_config_base != nullptr);                                                                                            
  unsigned long *cperf =      (unsigned long*)mmap(NULL, CACHE_PERF_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, hpm_fd, CACHE_PERF);
  unsigned char *plim = (unsigned char*)mmap((void *)0,
                             RELCACHE_SIZE,
                             PROT_READ | PROT_WRITE,
                             MAP_SHARED,
                             hpm_fd,
                             RELCACHE_ADDR);


  api.SetBaseAddr((uint64_t)agu_config_base);
  if (!api.Compile(FileToString("./aguconfig")))
  {
    printf("Failed to compile dtl program or map onto agu\n");
    return;
  }
  api.ProgramHardware();

  printf("Successfully compiled dtl\n");
  printf("Successfully programmed agu\n");
  



  printf("mapped plim\n");
  // mapping dram
  printf("mapping DRAM\n");
  unsigned char *dram =
      plim; // mmap((void*)0, RELCACHE_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED,
            // dram_fd, RELCACHE_ADDR);
  printf("mapped DRAM\n");

  //T data;
  unsigned long data_count = 0; // changed data type from T to unsigned long



  uint64_t sum_rme = 0;
  if (config_db.store_type == 'r') {
    unsigned width = config_db.column_widths[0];
    printf("Max RME access 0x%x\n", (config_db.row_count-1) * sum_col_width + width * (params.enabled_column_number-1));
    printf("Max Row store access 0x%x\n", config_db.row_size*(config_db.row_count-1));

       // move multiplication outside
    EnableRelCache(hpm_fd);
    
    get_rme_pmcs(&start, config, cperf);
    pmcs_get_value(&start);
    //  magic_timing_begin(&cycleLo, &cycleHi);



    for (int i = 0; i < config_db.row_count; i++) {
      for (int j = 0; j < params.enabled_column_number; j++) {\
         cold_array[data_count++] = // change back?
            *(T *)(plim + i * sum_col_width + width * j);
        // cold_array[data_count] = readData(&config_db, (plim + i*sum_col_width
        // + width*j));
      }
    }    
    //  magic_timing_end(&cycleLo, &cycleHi);
    pmcs_get_value(&end);
    get_rme_pmcs(&end, config, cperf);
    res = pmcs_diff(&end, &start);
    fprintf(params.output_file,
            "q1, r, c, %d, %d, %d, %d, %lu, %lu, %lu, %lu, %lu, %lu, %lu, %lu, %lu, %lu, %lu, %lu\n",
            params.enabled_column_number,
            config_db.row_size,
            config_db.row_count,
            config_db.column_widths[0],
            res.cycles,
            res.l1_references,
            res.l1_refills,
            res.l2_references,
            res.l2_refills,
            res.inst_retired,
            res.time.tv_sec * 1000000000L + res.time.tv_nsec,
            res.stall_ctrl_trapper,
            res.stall_fetch_ctrl,
            res.stall_fetch_full,
            res.stall_fetch_memory,
            res.stall_req_fetch);
    data_count = 0;

    FlushAndDisable(hpm_fd);
  
    // pmcs_get_value(&start);
    // magic_timing_begin(&cycleLo, &cycleHi);

    // for(int i = 0; i < config_db.row_count; i++){
    //   unsigned offset = 0;
    //   for(int j=0; j<params.enabled_column_number; j++){
    //     hot_array[data_count] = *(T*)(plim + i*sum_col_width + width*j);
    //     data_count++;
    //   }
    // }

    uint64_t sum_row = 0;
    // magic_timing_end(&cycleLo, &cycleHi);
    // pmcs_get_value(&end);
    // res = pmcs_diff(&end, &start);
    // fprintf(params.output_file,"q1, c, -, %d, %d, %d, %d, %d, %lu, %lu, %lu,
    // %lu, %lu,%llu\n", params.enabled_column_number, config_db.row_size,
    // config_db.row_count, config_db.column_widths[0], res.cycles,
    // res.l1_references, res.l1_refills, res.l2_references, res.l2_refills,
    // res.inst_retired, res.time.tv_sec*1000000000L+res.time.tv_nsec);
    data_count = 0;
    get_rme_pmcs(&start, config, cperf);
    pmcs_get_value(&start);
    // magic_timing_begin(&cycleLo, &cycleHi);
    for (int i = 0; i < config_db.row_count; i++) {
      for (int j = 0; j < params.enabled_column_number; j++) {
        row_array[data_count++] = *(T *)(dram + i * config_db.row_size + params.col_offsets[j]);
        // cold_array[data_count] = readData(&config_db, dram +
        // i*config_db.row_size + params.col_offsets[j]);
      }
    }

    // magic_timing_end(&cycleLo, &cycleHi);
    pmcs_get_value(&end);
    get_rme_pmcs(&end, config, cperf);
    res = pmcs_diff(&end, &start);
    fprintf(params.output_file,
            "q1, d, -, %d, %d, %d, %d, %lu, %lu, %lu, %lu, %lu, %lu,%lu, %lu, %lu, %lu, %lu, %lu\n",
            params.enabled_column_number,
            config_db.row_size,
            config_db.row_count,
            config_db.column_widths[0],
            res.cycles,
            res.l1_references,
            res.l1_refills,
            res.l2_references,
            res.l2_refills,
            res.inst_retired,
            res.time.tv_sec * 1000000000L + res.time.tv_nsec,
            res.stall_ctrl_trapper,
            res.stall_fetch_ctrl,
            res.stall_fetch_full,
            res.stall_fetch_memory,
            res.stall_req_fetch);


    if (config_db.print == true) {
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
  } else if (config_db.store_type == 'c') {
    unsigned width = config_db.column_widths[0];
    FlushAndDisable(hpm_fd);
    uint64_t sum_col = 0;
    
    data_count = 0;
    get_rme_pmcs(&start, config, cperf);
    pmcs_get_value(&start);
    // magic_timing_begin(&cycleLo, &cycleHi);
    for (int i = 0; i < config_db.row_count; i++) {
      for (int j = 0; j < params.enabled_column_number; j++) {
        // for now only for projectivity experiment
        col_array[data_count++] =
            *(T *)(dram + (i + j * config_db.row_count) * width);
        // col_array[data_count++] = readData(&config_db, dram + (i + j *
        // config_db.row_count) * config_db.column_width);

      
      }
    }
    //  magic_timing_end(&cycleLo, &cycleHi);
    pmcs_get_value(&end);
    get_rme_pmcs(&end, config, cperf);
    res = pmcs_diff(&end, &start);
    fprintf(params.output_file,
            "q1, c, -, %d, %d, %d, %d, %lu, %lu, %lu, %lu, %lu, %lu,%lu, %lu, %lu, %lu, %lu, %lu\n",
            params.enabled_column_number,
            config_db.row_size,
            config_db.row_count,
            config_db.column_widths[0],
            res.cycles,
            res.l1_references,
            res.l1_refills,
            res.l2_references,
            res.l2_refills,
            res.inst_retired,
            res.time.tv_sec * 1000000000L + res.time.tv_nsec,
            res.stall_ctrl_trapper,
            res.stall_fetch_ctrl,
            res.stall_fetch_full,
            res.stall_fetch_memory,
            res.stall_req_fetch);
    if (config_db.print == true) {
      printf("\nColumn store query results:\n");
      for (unsigned int i = 0; i < data_count; i++) {
        printf("%d \n", col_array[i]);
      }
    }

    free(col_array);
  }

  int ret = teardown_pmcs();
  if (ret < 0)
    perror("Issue detected while tearing down the PMCs\n");

  fflush(params.output_file);

  munmap(plim, RELCACHE_SIZE);
  munmap(cperf, CACHE_PERF_SIZE);
  munmap(config, RME_CONFIG_SIZE);
  // munmap(dram, dram_size);

  close(hpm_fd);
  // close(dram_fd);
}
