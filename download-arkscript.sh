#!/bin/bash

if [[ `whoami` == root ]]; then
    mkdir -p /root/ark
    cd /root/ark
else
    mkdir -p "/home/$(whoami)/ark"
    cd "/home/$(whoami)/ark"
fi

current=`curl -s https://github.com/ArkScript-lang/Ark/releases/latest | egrep -o "tag/(?[^\"]+)"`
if [ -f ark-version ]; then
    if [[ $current == `cat ark-version` ]]; then
        echo "You already have the latest ArkScript version"
        exit 0
    fi
else
    if [ -f linux64.zip ]; then
        echo "Deleting old linux.zip..."
        rm linux64.zip
    fi
    echo $current > ark-version
    echo "Found a new version: $current"
fi

current=`echo $current | cut -c 5- -`
url="https://github.com/ArkScript-lang/Ark/releases/download/$current/linux64.zip"
wget --quiet $url

if [ -f linux64.zip ]; then
    echo "New version downloaded in $(pwd)/linux64.zip"
else
    echo "Something went wrong, couldn't find linux64.zip"
fi
