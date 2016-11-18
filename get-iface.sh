#!/usr/bin/env bash

if [ `uname -s` = "Darwin" ];
then
	if ( `system_profiler SPHardwareDataType | grep -q "MacBook Pro"` );
	then
		echo "en0"
	else
		echo "en1"
	fi
else
	RPI_PREFIX="b8:27:eb"
	# get the wlan interface that doesn't have the RPI MAC prefix
	echo `sudo ifconfig | grep wlan | grep -v $RPI_PREFIX | cut -d' ' -f1`
fi