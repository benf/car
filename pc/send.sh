#!/bin/bash
tmp=$*
for ((i=0; i<${#tmp}; ++i)); do
	echo -n "${tmp:i:1}" >> /dev/ttyUSB0
	sleep 0.01
done
echo >> /dev/ttyUSB0
