# miniperf

Code example for the article "How Perf(1) works" on blog.gistre.epita.fr. The
examples follow do not follow good practices, like verifying the return code of
functions, for the sake of being shorter.

You can build the examples using make:
```sh
42sh$ make
```

Each exemple will create a binary.

The binaries can be launched like this:
```
42sh$ ./perf_event_printf         
Measuring instruction count for this printf
Used 4565 instructions

42sh$ ./perf_event_command sleep 5
Used 1168694 instructions

42sh$ ./perf_msr_cpuid            
Architectural Performance Monitoring Version: 5

42sh$ sudo ./perf_msr_dev sleep 5
Instructions Retired: 89350255
```
