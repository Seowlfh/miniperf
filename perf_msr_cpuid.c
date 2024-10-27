// Filename: perf_msr_cpuid.c
#include <stdio.h>

static void cpuid(unsigned int op, unsigned int *a, unsigned int *b,
                  unsigned int *c, unsigned int *d) {
  asm("cpuid\n" : "=a"(*a), "=D"(*b), "=c"(*c), "=d"(*d) : "a"(op));
}

int main(void) {
  unsigned int a, b, c, d;
  // Depending on the value inside the EAX register when calling cpuid, the
  // function outputs different things.
  // 10 represents the CPUID.0AH leaf, which exposes information about the
  // AMC. See Intel Software Developer's Manual, Volume 3, Section 20.1.
  cpuid(10, &a, &b, &c, &d);

  printf("Architectural Performance Monitoring Version: %d\n", a % 256);

  return 0;
}
