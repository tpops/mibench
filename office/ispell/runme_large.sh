#!/bin/sh
sudo perf stat -r 10000 ./ispell -a -d tests/americanmed+ < tests/large.txt > tests/output_large.txt
