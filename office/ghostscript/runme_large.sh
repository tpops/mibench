#!/bin/sh
sudo perf stat -r 100 ./src/gs -sDEVICE=ppm -dNOPAUSE -q -sOutputFile=data/output_large.ppm -- data/large.ps
