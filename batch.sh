#! /bin/sh

for i in `cat stats | awk '{print levels/$1}'`; do
    if [ -f $i ]; then
        ./run.sh $i
    fi
done
