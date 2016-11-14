#!/usr/bin/env bash

build/tinsSniffer | xargs -0 -n1 -I{} curl http://localhost:3000/add?url={}