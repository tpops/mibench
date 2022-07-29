#!/bin/bash
# usage <bash> <policy_name>
policy_name=$1
for i in 0 120 140
 do 
make clean && make gs PARAMS="-O3  -fno-optimize-sibling-calls -flto=thin -mllvm -inline-stack-policy=${policy_name} -mllvm -inline-stack-weight=$i\
 -mllvm -debug-only=inline -mllvm -stats -save-temps -Wl,-mllvm,-stats,-mllvm,-inline-stack-policy=${policy_name},-mllvm,-inline-stack-weight=$i,-mllvm,\
-debug-only=inline" 2> debug_${policy_name}_$i 1> out_${policy_name}_$i && objdump -d ./gs | checkstack.pl x86_64 \
| grep "Function:" -A 50 > cgdump_${policy_name}_$i && echo -n "$i" && cat cgdump_${policy_name}_$i\
 | head -n 5 | grep "astack:" && echo -n "Code Size: " && stat -c %s ./gs\
 && cat debug_${policy_name}_$i | grep -- "- Number of functions inlined"\
| awk -F ' ' '{sum+=$1} END {print "inlines: " + sum}'
cd ..   
 ./runme_large.sh  &&\
 sudo perf stat -r 100 ls > /dev/null &&\
 ./runme_large.sh  &&\
 sudo perf stat -r 100 ls > /dev/null
cd src/
done
