#!/usr/bin/bash

if [ ! -e debugger ]; then
    echo '`make`'
    exit 1
fi
if [ $# -ne 1 ]; then
    echo 'Usage: aqdb.sh filename.s';
    exit 1
fi

tmpfilename=$(mktemp)
cat $1 | ./macro > $tmpfilename
./debugger $tmpfilename
