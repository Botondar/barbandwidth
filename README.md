# Test result

Hardware: i7-6700k and GTX 970.

After (and at) 256KiB writes the test results stabilized at ~7.5 GiB/s

4096 repetitions

Entries contain throughput in `avg (min)` format.

| Size | Temporal | Non-temporal |
| --- | --- | --- |
| 4 KiB | 9.115188 GiB/s ( 80.081882 GiB/s ) | 34.343986 GiB/s ( 63.878544 GiB/s ) |
| 8 KiB | 8.052138 GiB/s ( 64.253565 GiB/s ) | 37.496817 GiB/s ( 63.507876 GiB/s ) |
| 16 KiB | 8.034200 GiB/s ( 13.570395 GiB/s ) | 31.245723 GiB/s ( 61.371162 GiB/s ) |
| 32 KiB | 8.173733 GiB/s ( 10.082473 GiB/s ) | 34.582302 GiB/s ( 46.955412 GiB/s ) |
| 64 KiB | 7.520136 GiB/s ( 8.482209 GiB/s ) | 27.607888 GiB/s ( 41.548335 GiB/s ) |
| 128 KiB | 7.508719 GiB/s ( 7.893041 GiB/s ) | 8.082819 GiB/s ( 37.601972 GiB/s ) |
| 256 KiB | 7.549537 GiB/s ( 7.726569 GiB/s ) | 7.550842 GiB/s ( 7.811182 GiB/s ) |

I'm not sure how much the average case of the temporal stores is tainted by doing repetition testing: it might be the case that continuously hammering the same memory forces the processor to actually wait for the results to transfer on the PCIe bus (which would explain the average case dropping to the baseline ~7.5-8 GiB/s).

Looking at either the min or the avg cases of both tests (whichever's better), it seems like doing non-temporal stores for the common constant buffer sizes of <64 KiB is a clear winner. This is especially true if the buffer contents have just been prepared separately in a RAM buffer - doing temporal stores might knock out still needed cache lines in that case.