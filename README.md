# lagscope

## Summary

A Linux tool to measure the network transport layer latency.

## Features

* Support network transport layer latency measurement (round-trip latency).

* Support specifying ping message size, and ping interval.

* Support two test modes: test-duration mode and ping-iteration mode.

* Support CPU affinity.

* Support running in background (daemon).


## Getting Started

### Building lagscope

```
make; make install
```

### Usage

```
lagscope -h
```

### Known issues

* UDP is not supported.

### Example run Histogram

To measure the network TCP latency between two multi-core serves running Ubuntu 1604, NODE1 (192.168.4.1) and NODE2 (192.168.4.2).

On NODE1 (the receiver), run:
```
./lagscope -r
```
(Translation: Run lagscope as a receiver with default settings. See the output from `./lagscope -h` for more details about the default settings.)

And on NODE2 (the sender), run:
```
./lagscope -s192.168.4.1 -H -a 10 -l 1 -c 98
```
(Translation: Run lagscope as a sender, with default test settings; report histogram value with customized factors.)


Example sender-side output from a given run (which showcases 0.515ms latency in average):

```
simonxiao@NODE2:~/lagscope/src# ./lagscope -s192.168.4.1 -H -a 10 -l 1 -c 98
lagscope 0.1.1
---------------------------------------------------------
13:19:44 INFO: New connection: local:13948 [socket:3] --> 192.168.4.1:6001
13:36:24 INFO: TEST COMPLETED.
13:36:24 INFO: Ping statistics for 192.168.4.1:
13:36:24 INFO:  Number of successful Pings: 1000000
13:36:24 INFO:  Minimum = 30.994us, Maximum = 8699.894us, Average = 54.063us
Interval(usec)   Frequency
      0          0
     10          0
     11          0
     12          0
     ...
     50          75086
     51          116062
     52          36401
     53          27288
     54          19781
     55          16578
     56          29390
     57          8965
     58          8980
     59          8812
     60          8418
     ...	 
    100          519
    101          472
    102          849
    103          324
    104          324
    105          327
    106          291
    107          288
    108          6107
```

### Example run Percentile

To measure the network TCP latency between two multi-core serves running Ubuntu 1604, NODE1 (192.168.4.1) and NODE2 (192.168.4.2).

On NODE1 (the receiver), run:
```
./lagscope -r
```
(Translation: Run lagscope as a receiver with default settings. See the output from `./lagscope -h` for more details about the default settings.)

And on NODE2 (the sender), run:
```
./lagscope -s192.168.4.1 -P
```
(Translation: Run lagscope as a sender. Prints these percentiles of the latencies: 50%, 75%, 90%, 99%, 99.9%, 99.99%, 99.999%.)


Example sender-side output from a given run:

```
paulkim@NODE2:~/lagscope/src# ./lagscope -s192.168.4.1 -P
lagscope 0.1.2
---------------------------------------------------------
17:49:03 INFO: New connection: local:13948 [socket:3] --> 192.168.4.1:6001
17:50:37 INFO: TEST COMPLETED.
17:50:37 INFO: Ping statistics for 192.168.4.1:
17:50:37 INFO:  Number of successful Pings: 1000000
17:50:37 INFO:  Minimum = 72.002us, Maximum = 4552.126us, Average = 92.055us

Percentile       Latency(us)
     50%         80
     75%         102
     90%         113
   99.9%         410
  99.99%         2566
 99.999%         3921
```

### Example run to dump latencies into a csv file

On NODE2 (the sender), run:
```
./lagscope -s192.168.4.1 -Rlatencies_log.csv
```
(Translation: Run lagscope as a sender and dumps latencies into a csv file)


Example sender-side output from a given run:

```
paulkim@NODE2:~/lagscope/src# ./lagscope -s192.168.4.1 -Rlatencies_log.csv
lagscope 0.1.2
---------------------------------------------------------
19:38:31 INFO: New connection: local:13948 [socket:3] --> 192.168.4.1:6001
19:38:32 INFO: TEST COMPLETED.
19:38:32 INFO: Ping statistics for 192.168.4.1:
19:38:32 INFO:  Number of successful Pings: 1000000
19:38:32 INFO:  Minimum = 96.083us, Maximum = 2828.121us, Average = 147.913us
19:38:32 INFO: Dumping all latencies into latencies_log.csv
```

## Related topics

1. [NTTTCP-for-Linux](https://github.com/Microsoft/ntttcp-for-linux)

2. [Microsoft Server & Cloud Blog](http://blogs.technet.com/b/server-cloud/)

3. [HyperV Linux Integrated Services Test](https://github.com/LIS/lis-test)


## Terms of Use

By downloading and running this project, you agree to the license terms of the third-party application software, Microsoft products, and components to be installed.

The third-party software and products are provided to you by third parties. You are responsible for reading and accepting the relevant license terms for all software that will be installed. Microsoft grants you no rights to third party software.


## License

```
The MIT License (MIT)

Copyright (c) 2016 Microsoft Corporation

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```
