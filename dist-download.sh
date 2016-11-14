#!/usr/bin/env bash

for ID in $(seq 4 7)
do
	USER="pi"
	SERVER="sniff$ID.local"
	AT="$USER@$SERVER"

	echo $AT

	rsync -avz pi@sniff$ID.local:~/wifiaggregator/ dist/sniff$ID/

done
