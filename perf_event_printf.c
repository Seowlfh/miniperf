// Filename: perf_event_printf.c
#include <linux/perf_event.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

static long perf_event_open(struct perf_event_attr *hw_event, pid_t pid,
                            int cpu, int group_fd, unsigned long flags) {
  // We must use syscall, as stated in the manpage: "glibc provides no wrapper
  // for perf_event_open(), necessitating the use of syscall(2)."
  return syscall(SYS_perf_event_open, hw_event, pid, cpu, group_fd, flags);
}

int main(void) {
  int fd;
  long long count;
  struct perf_event_attr pe;

  // Preparing the perf_event_attr struct that's going to be given to
  // perf_event_open.
  memset(&pe, 0, sizeof(pe));
  pe.type = PERF_TYPE_HARDWARE; // Hardware event
  pe.size = sizeof(pe);
  pe.config = PERF_COUNT_HW_INSTRUCTIONS; // Count CPU instructions
  pe.disabled = 1; // Disable the counter (do not start counting just yet).
  pe.exclude_kernel = 1; // Do not count kernel code
  pe.exclude_hv = 1;     // Do not count hypervisor code

  // perf_event_open returns a file descriptor. Reading from this file
  // descriptor will give use the counter.
  fd = perf_event_open(&pe, 0, -1, -1, 0);
  if (fd == -1) {
    fprintf(stderr, "Error opening leader %llx\n", pe.config);
    exit(EXIT_FAILURE);
  }

  ioctl(fd, PERF_EVENT_IOC_RESET, 0); // Reset the counter
  ioctl(fd, PERF_EVENT_IOC_ENABLE,
        0); // Enable the counter right before the printf

  /* Begin measuring */
  printf("Measuring instruction count for this printf\n");
  /* Stop measuring */

  ioctl(fd, PERF_EVENT_IOC_DISABLE, 0); // Disable the counter
  read(fd, &count, sizeof(count)); // Read the number of instructions into count

  printf("Used %lld instructions\n", count);

  close(fd);
}
