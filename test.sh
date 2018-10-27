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

# FE  DCB  A98  7654  3210
# 11  Rs   Rd   op3   d
runtest "C0F0" 0

echo "ok"
