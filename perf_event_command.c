// Filename: perf_event_command.c
#include <linux/perf_event.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static long perf_event_open(struct perf_event_attr *hw_event, pid_t pid,
                            int cpu, int group_fd, unsigned long flags) {
  // We must use syscall, as stated in the manpage: "glibc provides no wrapper
  // for perf_event_open(), necessitating the use of syscall(2)."
  return syscall(SYS_perf_event_open, hw_event, pid, cpu, group_fd, flags);
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Not enough arguments\n");
    return -1;
  }

  int fd;
  long long count;
  struct perf_event_attr pe;

  memset(&pe, 0, sizeof(pe));
  pe.type = PERF_TYPE_HARDWARE;
  pe.size = sizeof(pe);
  pe.config = PERF_COUNT_HW_INSTRUCTIONS;
  pe.disabled = 1;
  pe.exclude_kernel = 1;
  pe.exclude_hv = 1;
  pe.enable_on_exec =
      1;          // [NEW] Enable the counter automatically after a call to exec
  pe.inherit = 1; // [NEW] Also count for the children's instructions
  pe.exclude_guest = 1; // [NEW] Do not count guest code (VMs). Just userland.

  fd = perf_event_open(
      &pe, 0, -1, -1,
      PERF_FLAG_FD_CLOEXEC); // [NEW] Use the flag PERF_FLAG_FD_CLOEXEC
  if (fd == -1) {
    fprintf(stderr, "Error opening leader %llx\n", pe.config);
    exit(EXIT_FAILURE);
  }

  ioctl(fd, PERF_EVENT_IOC_RESET, 0);

  // [NEW] Execute the command given in arguments with fork and exec.
  pid_t pid = fork();
  if (pid == 0) {
    execvp(argv[1],
           argv + 1); // Measurement begins only after exevcp is called.
    exit(-1);         // Child should not reach this.
  }

  ioctl(fd, PERF_EVENT_IOC_DISABLE, 0); // Disable counting for the parent

  int wstatus;
  waitpid(pid, &wstatus, 0); // [NEW] Wait for the command to stop executing.

  if (!WIFEXITED(wstatus)) {
    fprintf(stderr, "Shit happened\n");
    return -2;
  }

  read(fd, &count, sizeof(count));

  printf("Used %lld instructions\n", count);

  close(fd);
}
