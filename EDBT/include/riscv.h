#ifndef RISCV_H
#define RISCV_H
#include <stdint.h>
//#define mcounteren  0x0306
//#define scounteren  0x0106

#define mhpmevent3  0x0323
#define mhpmevent4  0x0324
#define mhpmevent5  0x0325
#define mhpmevent6  0x0326
#define mhpmevent7  0x0327
#define mhpmevent8  0x0328
#define mhpmevent9  0x0329

#define CYCLECOUNT  cycle
#define INSTRET     instret


// bits[0:7] select the event set, bits[8:?] select the even
#define BOOM_L1_ICACHEMISS 0x103 // this is either the 3rd or 2nd event set depending on indexing
#define BOOM_L1_DCACHEMISS 0x103 // this is either the 3rd or 2nd event set depending on indexing

#define read_csr(reg) ({ unsigned long __tmp; \
  asm volatile ("csrr %0, " #reg : "=r"(__tmp)); \
  __tmp; })

#define write_csr(reg, val) ({ \
  asm volatile ("csrw " #reg ", %0" :: "rK"(val)); })




#endif