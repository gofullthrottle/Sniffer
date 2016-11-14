#!/usr/bin/env bash

rsync -a build.sh dist/
rsync -a main.cpp dist/
rsync -a curl-loop.sh dist/

for ID in $(seq 4 7)
do
	{
		USER="pi"
		SERVER="sniff$ID.local"
		AT="$USER@$SERVER"

		echo $AT

		rsync -az dist/ pi@sniff$ID.local:~/sniffer/
	} &
done
wait