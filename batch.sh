#! /bin/zsh

for i in levels/*; do
    level=`basename $i`
    echo -n "$level: "
    line=`grep $level stats`
    unique=`echo "$line" | cut -d'|' -f 4 | tr ' ' '0' | sed 's,^0*\([^0]\),\1,'`
    paired=`echo "$line" | cut -d'|' -f 5 | tr ' ' '0' | sed 's,^0*\([^0]\),\1,'`
    echo $unique $paired
    cat Size.hh.in | sed s,@unique@,$unique, | sed s,@paired@,$paired, > Size.hh
    make
    ./atomixer $i
done
