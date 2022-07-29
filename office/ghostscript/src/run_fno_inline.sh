#!/bin/bash
# usage <bash> <policy_name>
policy_name=$1
for i in 0
 do 
make clean && make gs PARAMS="-fno-inline -O3 -flto=thin -mllvm -stats -Wl,-mllvm,-stats" 2> debug_${policy_name}_$i 1> out_${policy_name}_$i && objdump -d ./gs | checkstack.pl x86_64 \
| grep "Function:" -A 50 > cgdump_${policy_name}_$i && echo -n "$i" && cat cgdump_${policy_name}_$i\
 | head -n 5 | grep "astack:" && echo -n "Code Size: " && stat -c %s ./gs\
 && cat debug_${policy_name}_$i | grep -- "- Number of functions inlined"\
| awk -F ' ' '{sum+=$1} END {print "inlines: " +sum}' && ./runme_small.sh &> perf.log &&\
 grep "seconds time elapsed" perf.log 
done
