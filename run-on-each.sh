#!/usr/bin/env bash

# for ID in $(seq 4 7)
# for ID in cafe white
# for ID in imac
for SERVER in ww-imac.local
do
	# USER="pi"
	# SERVER="sniff$ID.local"

	USER="Administrator"
	# SERVER="ww-$ID.local"

	AT="$USER@$SERVER"
	echo $AT

	ssh $AT "$1"
done