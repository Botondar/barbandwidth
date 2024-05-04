# Test result

Hardware: i7-6700k and GTX 970.

After (and at) 256KiB writes the test results stabilized at 8 GiB/s

4096 repetitions

Entries contain throughput in `avg (min)` format.

| Size | Temporal | Non-temporal |
| --- | --- | --- |
| 4 KiB | 8.006196 GiB/s ( 66.734899 GiB/s ) | 35.490000 GiB/s ( 74.962490 GiB/s ) |
| 8 KiB | 8.036980 GiB/s ( 56.512170 GiB/s ) | 37.387903 GiB/s ( 62.302790 GiB/s ) |
| 16 KiB | 7.985278 GiB/s ( 16.820477 GiB/s ) | 21.905990 GiB/s ( 54.677053 GiB/s ) |
| 32 KiB | 7.984161 GiB/s ( 10.524424 GiB/s ) | 25.459408 GiB/s ( 37.815802 GiB/s ) |
| 64 KiB | 7.901010 GiB/s ( 8.764091 GiB/s ) | 29.489534 GiB/s ( 38.062392 GiB/s ) |
| 128 KiB | 7.948674 GiB/s ( 8.551801 GiB/s ) | 8.019220 GiB/s ( 37.149928 GiB/s ) |
| 256 KiB | 7.998072 GiB/s ( 8.309995 GiB/s ) | 8.002274 GiB/s ( 8.311310 GiB/s ) |

I'm not sure how much the average case of the temporal stores is tainted by doing repetition testing: it might be the case that continuously hammering the same memory forces the processor to actually wait for the results to transfer on the PCIe bus (which would explain the average case dropping to the baseline ~8 GiB/s).

Looking at either the min or the avg cases of both tests (depending on which one is better), it seems like doing non-temporal stores for the common constant buffer sizes of 16-64 KiB is a clear winner. This is especially true if the buffer contents have just been prepared separately in a RAM buffer - doing temporal stores might knock out still needed cache lines in that case.