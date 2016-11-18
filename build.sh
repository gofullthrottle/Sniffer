#!/usr/bin/env bash

BUILD=`uname -s`
LIB_DIR="lib"

if [ $BUILD = "Darwin" ];
then
  OS="osx"
 else
  OS="linux"
fi

echo "Building on $OS"

#TODO: Add check for libpcap
get_lib ()
{
  echo "Downloading libtins and creating $LIB_DIR"

  if [ $OS = "osx" ];
  then
    curl -s -o temp.zip "https://dl.dropboxusercontent.com/u/1108171/WifiWhisperer-Moogfest/libtins-osx/lib.zip"
  else
    wget -O temp.zip "https://dl.dropboxusercontent.com/u/1108171/WifiWhisperer-Moogfest/libtins-linux/lib.zip"
  fi

  unzip -q temp.zip -d  ./
  rm temp.zip

  if [ -d "__MACOSX" ]; then
    rm -rf __MACOSX
    echo 'Deleting garbage'
  fi
}

if [ -d "$LIB_DIR" ]; then
  echo "$LIB_DIR is present"
else
  get_lib
fi

if [ $OS = "osx" ]; then
  echo "Building channel-hop"
  clang -framework Foundation -framework CoreWLAN channel-hop.m -o channel-hop
fi

TINS_LIB_PATH="lib/libtins/"$OS"/lib"
TINS_INCLUDE_PATH="lib/libtins/"$OS"/include/"
APP_NAME="sniffer"
SRC=""

# NOTE: Make sure you have libpcap installed on your machine
# If using a pi: sudo apt-get install libpcap-dev
echo "Building sniffer"
g++ -std=c++11 $SRC"main.cpp" -o $APP_NAME -L$TINS_LIB_PATH  -I$TINS_INCLUDE_PATH -lpthread -ltins -lpcap

echo "Done"