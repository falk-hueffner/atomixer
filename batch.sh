#! /bin/sh

for i in `cat stats | awk '{print "levels/"$1}'`; do
    if [ -f $i ]; then
	level=`basename $i`
	line=`grep $level stats`
	solved=`echo "$line" | cut -d'|' -f 11`
	if [ $solved ]; then
	    echo $level: already solved in $solved moves
	else
	    ./run.sh $i
	fi
    fi
done
