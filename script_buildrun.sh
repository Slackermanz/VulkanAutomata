#!/bin/bash
	logfn="./vbk/VkAutoLog"$(date +%s)".log"
	script -q -c './buildrun.sh' $logfn
	echo '    file://'$(realpath $logfn)
	echo " "
