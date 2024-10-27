CC=gcc

FILES = perf_event_printf \
	perf_event_command \
	perf_msr_cpuid \
	perf_msr_dev

all: $(FILES)

clean:
	$(RM) $(FILES)

.PHONY: clean
