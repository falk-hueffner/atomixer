#! /bin/sh

timelimit=10

exec 6>batch.$$.log

./timeguard.sh atomixer $timelimit &

for i in `cat stats | awk '{print "levels/"$1}'`; do
    if [ -f $i ]; then
	level=`basename $i`
	line=`grep $level stats`
	solved=`echo "$line" | cut -d'|' -f 11`
	if [ $solved ]; then
	    echo $level: already solved in $solved moves
	else
	    printf "$level: started on `date`" >&6
	    ./run.sh $i
	    printf " finished on `date`\n" >&6
	fi
    fi
done

killall timeguard.sh
