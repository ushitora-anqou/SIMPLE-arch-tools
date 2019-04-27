#!/usr/bin/bash

if [ ! -e debugger/debugger ]; then
    echo '`make` in debugger'
    exit 1
fi
if [ $# -ne 1 ]; then
    echo 'Usage: aqdb.sh filename.s';
    exit 1
fi

tmpfilename=$(mktemp)
cat $1 | ./macro > $tmpfilename
debugger/debugger $tmpfilename
