#!/usr/bin/env bash

rsync -a build.sh dist/
rsync -a main.cpp dist/
rsync -a curl-loop.sh dist/
rsync -a get-iface.sh dist/
rsync -a channel-hop.sh dist/
rsync -a channel-hop.m dist/

# for ID in $(seq 4 7)
# for ID in cafe white
for SERVER in 10.10.201.184
do
	{
		# USER="pi"
		# SERVER="sniff$ID.local"

		USER="Administrator"
		# SERVER="ww-$ID.local"

		AT="$USER@$SERVER"
		echo $AT

		rsync -az dist/ $AT:~/sniffer/
	} &
done
wait