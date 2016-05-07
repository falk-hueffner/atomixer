#! /bin/sh -f

level=$1

if [ ! -r "$level" ]; then
    echo "Can't open $level"
    exit 1
fi

set `awk -f countatoms.awk < "$level"`

echo "$1 x $2 board, large: $3; $4 unique, $5 paired, and $6 multiples."

cat Size.hh.in | \
   sed s,@xsize@,$1, | \
   sed s,@ysize@,$2, | \
   sed s,@large_board@,$3, | \
   sed s,@unique@,$4, | \
   sed s,@paired@,$5, | \
   sed s,@multi@,$6, > Size.hh.tmp

if cmp -s Size.hh Size.hh.tmp; then
    rm Size.hh.tmp
else
    mv Size.hh.tmp Size.hh
fi

if which gmake 2>/dev/null; then
    gmake && ./atomixer "$level"
else
    make && ./atomixer "$level"
fi
