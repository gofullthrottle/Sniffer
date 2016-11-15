#!/usr/bin/env bash

airport=/System/Library/PrivateFrameworks/Apple80211.framework/Versions/A/Resources/airport

sudo $airport -z

while true
do
	for channel in 1 6 11
	do
		sudo $airport --channel=$channel
		sudo $airport -I | grep channel
		sleep .5
	done
done