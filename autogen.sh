#! /bin/sh -f

mkdir aux
aclocal
automake -a
autoconf
