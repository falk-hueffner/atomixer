#! /bin/sh -f

prog=$1
limit=$2

pc() {
  echo $2 | awk "{print \$$1}"
}

while sleep 60; do
  runtime=`ps -C $prog -otime --no-headers 2>/dev/null`
  if [ $runtime ]; then
    runtime=${runtime/:/ }
    runtime=${runtime/:/ }
    minutes=$((`pc 1 "$runtime"` * 60 + `pc 2 "$runtime"`))
    if [ $minutes -ge $limit ]; then
      killall $prog
    fi
  fi
done