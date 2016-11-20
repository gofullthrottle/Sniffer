#!/usr/bin/env bash

cd $(dirname $0)

#         0 1 2 3 4 5 6 7
CHANNELS=(0 0 0 0 4 4 4 4)
DOMAIN=wifiwhisperer.com
# DOMAIN=localhost:3000

IFACE=`./get-iface.sh`

if [ `uname -s` = "Darwin" ];
then
 	echo "Listening on OSX interface $IFACE"
else
	# get the raspberry pi's id from the hostname
	ID=`hostname | rev | cut -c1`
	# use the id to lookup the channel from the channels array
	CHANNEL=${CHANNELS[$ID]}
	sudo iwconfig $IFACE mode managed
	sudo iwconfig $IFACE channel $CHANNEL # seems to work but causes an error

	echo "Listening on Raspberry Pi $ID channel $CHANNEL with interface $IFACE"
	# sudo iwlist $IFACE power
	# sudo iwlist $IFACE channel | grep Current
	sudo ifconfig -v $IFACE
	sudo iwconfig $IFACE
fi

killall channel-hop
./channel-hop > /dev/null 2> /dev/null &

# needs to be sudo on linux
./sniffer $IFACE | xargs -0 -n1 -I{} curl -sL -w "%{http_code} %{url_effective}\\n" "http://$DOMAIN/add?{}"
# ./sniffer $IFACE | xargs -0 -n1 -I{} curl -sL -w "%{http_code} %{url_effective}\\n" -H "Content-Type: application/json" -d {} "http://$DOMAIN/probe-req"
# ./sniffer $IFACE | xargs -0 -n1 -I{} echo {}