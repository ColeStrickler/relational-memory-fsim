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

int main()
{
    //enable_perf();
    //uint64_t scount = read_csr_safe(scounteren);
    //uint64_t mcount = read_csr_safe(mcounteren);
    //printf("perf enabled %llu -- %llu\n", scount, mcount);
    uint64_t cycle_start = read_csr_safe(cycle);
    uint64_t inst_start = read_csr_safe(instret);
    int x = 0;
    for (int i = 0; i < 100; i++)
    {
        x += i;
    }
    uint64_t inst_end = read_csr_safe(instret);
    uint64_t cycle_end = read_csr_safe(cycle);

    printf("Cycle count: %llu\n", cycle_end-cycle_start);
    printf("Instructions retired: %llu\n", inst_start-inst_end);
}