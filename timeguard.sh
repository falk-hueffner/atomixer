#! /bin/sh -f

prog=$1
limit=$2

while true; do
  runtime=`ps -C $prog -otime --no-headers 2>/dev/null`
  if [ $runtime ]; then
    minutes=$(echo "$runtime" | awk -F ':' '{ print $1 * 60 + $2 }')
    echo $prog running for $minutes/$limit minutes
    if [ $minutes -ge $limit ]; then
      echo killing $prog
      killall $prog
    fi
  else
    echo $prog not running
  fi
  sleep 60
done