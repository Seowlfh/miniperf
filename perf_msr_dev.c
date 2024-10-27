// Filename: perf_msr_dev.c
#define _GNU_SOURCE

#include "sys/wait.h"
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <sched.h>
#include <errno.h>
#include <err.h>

#define MSR_IA32_PERF_GLOBAL_CTRL     0x38F
#define MSR_IA32_PERFEVTSEL0          0x186
#define MSR_IA32_PMC0                 0xC1

// Function to read an MSR
uint64_t read_msr(int cpu, uint32_t msr) {
    char msr_path[32];
    snprintf(msr_path, sizeof(msr_path), "/dev/cpu/%d/msr", cpu);

    int fd = open(msr_path, O_RDONLY);
    uint64_t data;
    pread(fd, &data, sizeof(data), msr) != sizeof(data);

    close(fd);
    return data;
}

// Function to write to an MSR
void write_msr(int cpu, uint32_t msr, uint64_t value) {
    char msr_path[32];
    snprintf(msr_path, sizeof(msr_path), "/dev/cpu/%d/msr", cpu);

    int fd = open(msr_path, O_WRONLY);
    pwrite(fd, &value, sizeof(value), msr) != sizeof(value);
    close(fd);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Not enough arguments\n");
        return -1;
    }

    int cpu = 0;

    // Configure MSR_IA32_PERFEVTSEL0 to count retired instructions where:
    // 		  event type |    user code    |   enabled
    uint64_t evtsel0 = (0xC0 | (0x01ULL << 16) | (1ULL << 22));
    write_msr(cpu, MSR_IA32_PERFEVTSEL0, evtsel0);

    pid_t pid = fork();
    if (pid == 0) {
       execvp(argv[1], argv + 1); 
       exit(-1);
    } else {
        write_msr(cpu, MSR_IA32_PMC0, 0x0);  // Clear the counter register
        write_msr(cpu, MSR_IA32_PERF_GLOBAL_CTRL, 0x1); // Enable the counters
    }

    int wstatus;
    waitpid(pid, &wstatus, 0);

    write_msr(cpu, MSR_IA32_PERF_GLOBAL_CTRL, 0x0); // Disable the counters

    uint64_t instructions_retired = read_msr(cpu, MSR_IA32_PMC0);

    printf("Instructions Retired: %ld\n", instructions_retired);

    return 0;
}
