#! /bin/sh

exec 6>batch.$$.log

while read level; do
    printf "$level: started on `date`" >&6
    ./run.sh "$level"
    printf " finished on `date`\n" >&6
done
