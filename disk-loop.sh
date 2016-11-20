#!/usr/bin/env bash

cd $(dirname $0)

sudo chmod o+r /dev/bpf*

killall channel-hop
./channel-hop > /dev/null 2> /dev/null &

IFACE=`./get-iface.sh`
./sniffer $IFACE >> sniffing.log
