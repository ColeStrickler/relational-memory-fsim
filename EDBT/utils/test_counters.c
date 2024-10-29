#include <stdio.h>
#include <stdint.h>
#include "riscv.h"


void enable_perf()
{
    write_csr(mcounteren, -1);
    write_csr(scounteren, -1);
}

#define read_csr_safe(reg) ({ register long __tmp asm("a0"); \
        asm volatile ("csrr %0, " #reg : "=r"(__tmp)); \
        __tmp; })
#define write_csr(reg, val) ({ \
        asm volatile ("csrw " #reg ", %0" :: "rK"(val)); })


#define HPM_SETUP_EVENTS(name, eventset, events) ({ \
            write_csr(name, ((events << 8) | eventset)); \
        })


uint64_t read_cycle() {
    uint64_t cycle_count;
    asm volatile ("csrr %0, cycle" : "=r"(cycle_count));
    return cycle_count;
}


int main()
{
    //enable_perf();
    uint64_t inst_start = read_csr_safe(instret);
    int x = 0;
    for (int i = 0; i < 100; i++)
    {
        x += i;
    }
    uint64_t inst_end = read_csr_safe(instret);
    uint64_t cycle_end = read_csr_safe(cycle);


    printf("Cycle count: %llu\n", cycle_end-cycle_start);
    printf("Instructions retired: %llu\n", inst_end-inst_start);
}