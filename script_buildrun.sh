#!/bin/bash
	logfn="./log/AppLog"$(date +%s)".log"
	script -q -c "./buildrun.sh" $logfn
	echo " "
	echo '    file://'$(realpath $logfn)
	echo " "
