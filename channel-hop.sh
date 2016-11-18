#!/usr/bin/env bash

DELAY=${1:-0.2}
UNAME=`uname -s`
IFACE=`./get-iface.sh`

# setup
if [ $UNAME = "Darwin" ];
then
	airport=/System/Library/PrivateFrameworks/Apple80211.framework/Versions/A/Resources/airport
	sudo $airport -z
	OS="OSX"
else
	OS="Raspberry Pi"
fi

echo "Channel hopping on $OS with interface $IFACE and delay $DELAY seconds"

setchannel() {
	CHANNEL=$1
	if [ $UNAME = "Darwin" ];
	then
		sudo $airport $IFACE --channel=$CHANNEL
		sudo $airport $IFACE --channel
	else
		sudo iwconfig $IFACE channel $CHANNEL
		iwlist $IFACE channel | grep Current
	fi
}

while true
do
	for CHANNEL in 104 108 112 # 1 6 11 36 40 44 48
	do
		setchannel $CHANNEL
		sleep $DELAY
	done
done
