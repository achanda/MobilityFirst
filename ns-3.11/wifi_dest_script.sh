#!/bin/bash

ns_file=./src/bypass/examples/wired-new.cc
ns_file_exec=wired-new


for i in 1 51 101 502 753 1028 2056 3048 4112
#for i in 25
do
	for j in 1 2 3 4 5 6 7 8 9 10
	do

		filename="wired-bypass+$i+$j.txt"
		echo $filename

		`./waf --run  "$ns_file_exec $j $i" > $filename`
	done
done
