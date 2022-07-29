#!/bin/bash
# usage <bash> <policy_name>
policy_name=$1
for i in 0
 do 
make clean && make ispell PARAMS="-fno-inline -O3 -flto=thin -mllvm -stats -Wl,-mllvm,-stats" 2> debug_${policy_name}_$i 1> out_${policy_name}_$i && objdump -d ./ispell | checkstack.pl x86_64 \
| grep "Function:" -A 50 > cgdump_${policy_name}_$i && echo -n "$i" && cat cgdump_${policy_name}_$i\
 | head -n 5 | grep "astack:" && echo -n "Code Size: " && stat -c %s ./ispell\
 && cat debug_${policy_name}_$i | grep -- "- Number of functions inlined"\
| awk -F ' ' '{sum+=$1} END {print "inlines: " +sum}' &&\
 ./runme_large.sh  &&\
 sudo perf stat -r 100 ls > /dev/null &&\
 ./runme_large.sh  &&\
 sudo perf stat -r 100 ls > /dev/null 
done
