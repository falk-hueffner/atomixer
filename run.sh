#! /bin/sh -f

if [ ! -f Makefile ]; then
    ./configure
fi

level=$1

if [ ! -r "$level" ]; then
    echo "Can't open $level"
    exit 1
fi

set `awk -f countatoms.awk < "$level"`

echo $1 unique, $2 paired, and $3 multiples.

cat Size.hh.in | \
   sed s,@unique@,$1, | \
   sed s,@paired@,$2, | \
   sed s,@multi@,$3, > Size.hh.tmp

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
