#!/bin/sh
echo ">>> Starting Collectl ..."
taskset 0x01 collectl   -c 3650  --export ./rsh.ph > $1 & 
taskset 0x01 sar 1 -o $2 > sartmpout &
