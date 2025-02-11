#!/bin/bash

forbiddenports="$*"
port=${port:-1024}
quit=0

while [ "$quit" -ne 1 ]; do

  if ! ( netstat -tulpn 2>&1 |grep  :$port[^0-9] > /dev/null ) && ! (echo $forbiddenports | grep -w -q $port ); then
    quit=1
  else
    port=`expr $port + 1`
  fi
done

echo PORT $port
