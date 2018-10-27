#!/bin/sh

fail() {
    echo $1
    exit 1
}

runtest(){
    ./encoder "$1" > _test.bin
    ./aqemu _test.bin
    res=$?
    [ $res -eq $2 ] || fail "[ERROR] \"$1\": expect $2 but got $res"
}

runtest "0000" 0

echo "ok"
