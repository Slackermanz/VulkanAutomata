#!/bin/bash
	if [$1 == ""]
	then 
		echo "No Vulkan SDK folder specified!"
	else
		logfn="./vbk/VkAutoLog"$(date +%s)".log"
		script -q -c "./buildrun.sh $1" $logfn
		echo " "
		echo '    file://'$(realpath $logfn)
		echo " "
	fi
