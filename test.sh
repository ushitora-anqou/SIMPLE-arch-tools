#!/bin/sh

fail() {
    echo $1
    exit 1
}

test_emulator(){
    echo "$1" | ./encoder > _test.bin
    ./emulator _test.bin
    res=$?
    [ $res -eq $2 ] || fail "[ERROR] \"$1\": expect $2 but got $res"
}

### emulator

# FE  DCB  A98  7654  3210
# 11  Rs   Rd   op3   d
#test_emulator "8001C0F0" 1
# reg[0] <- 1; HLT
test_emulator "c[0 0 1] a[0 0 15 0]" 1
# reg[1] <- 1; reg[0] <- reg[1]; HLT
test_emulator "c[0 1 1] a[1 0 6 0] a[0 0 15 0]" 1

# reg[0] <- 1; reg[1] <- 2; reg[0] <- reg[0] + reg[1]; HLT
test_emulator "c[0 0 1] c[0 1 2] a[1 0 0 0] a[0 0 15 0]" 3
# reg[0] <- 1; reg[1] <- -1; reg[0] <- reg[0] + reg[1]; HLT
test_emulator "c[0 0 1] c[0 1 -1] a[1 0 0 0] a[0 0 15 0]" 0
# reg[0] <- 2; reg[1] <- 1; reg[0] <- reg[0] - reg[1]; HLT
test_emulator "c[0 0 2] c[0 1 1] a[1 0 1 0] a[0 0 15 0]" 1
# reg[0] <- 2; reg[1] <- -1; reg[0] <- reg[0] - reg[1]; HLT
test_emulator "c[0 0 2] c[0 1 -1] a[1 0 1 0] a[0 0 15 0]" 3
# reg[0] <- 1; reg[1] <- 1; reg[0] <- reg[0] - reg[1]; HLT

test_emulator "c[0 0 1] c[0 1 1] a[1 0 1 0] a[0 0 15 0]" 0
# reg[0] <- 4; reg[1] <- 5; reg[0] <- reg[0] & reg[1]; HLT
test_emulator "c[0 0 4] c[0 1 5] a[1 0 2 0] a[0 0 15 0]" 4
# reg[0] <- 4; reg[1] <- 5; reg[0] <- reg[0] | reg[1]; HLT
test_emulator "c[0 0 4] c[0 1 5] a[1 0 3 0] a[0 0 15 0]" 5
# reg[0] <- 4; reg[1] <- 5; reg[0] <- reg[0] ^ reg[1]; HLT
test_emulator "c[0 0 4] c[0 1 5] a[1 0 4 0] a[0 0 15 0]" 1

# reg[0] <- 7; reg[0] <- SLL(reg[0], 2); HLT
test_emulator "c[0 0 7] a[1 0 8 2] a[0 0 15 0]" 28
# reg[0] <- 1; reg[0] <- SLL(reg[0], 15); reg[0] <- SLR(res[0], 1); HLT
test_emulator "c[0 0 1] a[1 0 8 15] a[1 0 9 1] a[0 0 15 0]" 1
# reg[0] <- 7; reg[0] <- SRL(reg[0], 2); HLT
test_emulator "c[0 0 7] a[1 0 10 2] a[0 0 15 0]" 1
# reg[0] <- -2; reg[0] <- SRA(reg[0], 1); HLT
test_emulator "c[0 0 -2] a[1 0 11 1] a[0 0 15 0]" 255
# reg[0] <- 2; reg[0] <- SRA(reg[0], 1); HLT
test_emulator "c[0 0 2] a[1 0 11 1] a[0 0 15 0]" 1

# reg[0] <- 2; B 1; reg[0] <- 1; HLT
test_emulator "c[0 0 3] c[4 0 1] c[0 0 1] a[0 0 15 0]" 3
# reg[0] <- 1; reg[1] <- 1; CMP(reg[0], reg[1]); BE 1; reg[0] <- 0; HLT
test_emulator "c[0 0 1] c[0 1 1] a[1 0 5 0] d[0 1] c[0 0 0] a[0 0 15 0]" 1
# reg[0] <- 1; reg[1] <- 1; CMP(reg[0], reg[1]); BNE 1; reg[0] <- 0; HLT
test_emulator "c[0 0 1] c[0 1 1] a[1 0 5 0] d[3 1] c[0 0 0] a[0 0 15 0]" 0

# reg[1] <- 0; LD(reg[0], 0(reg[1])); HLT"
test_emulator "c[0 1 0] b[0 0 1 0] a[0 0 15 0]" 0
# reg[1] <- 0; LD(reg[0], 1(reg[1])); HLT"
test_emulator "c[0 1 0] b[0 0 1 1] a[0 0 15 0]" 1
# reg[1] <- 100; ST(reg[1], 0(reg[1])); LD(reg[0], 0(reg[1])); HLT"
test_emulator "c[0 1 100] b[1 1 1 0] b[0 0 1 0] a[0 0 15 0]" 100


### asm

test_assembler(){
    echo "$1" | ./assembler > _test.bin
    ./emulator _test.bin
    res=$?
    [ $res -eq $2 ] || fail "[ERROR] \"$1\": expect $2 but got $res"
}

test_assembler "
LI  R1, 3
MOV R0, R1
HLT" 3

test_assembler "
LI  R0, 3
ADD R0, R0
HLT" 6

test_assembler "
LI  R0, 3
SUB R0, R0
HLT" 0

test_assembler "
LI  R0, 5
LI  R1, 1
AND R0, R1
HLT" 1

test_assembler "
LI  R0, 5
LI  R1, 2
OR  R0, R1
HLT" 7

test_assembler "
LI  R0, 5
LI  R1, 1
XOR R0, R1
HLT" 4

test_assembler "
LI  R0, 1
SLR R0, 1
HLT" 2

test_assembler "
LI  R0, 1
SLL R0, 15
SLR R0, 1
HLT" 1

test_assembler "
LI  R0, 3
SRL R0, 1
HLT" 1

test_assembler "
LI  R0, -2
SRA R0, 1
HLT" 255

test_assembler "
LI  R1, 0
LD  R0, 0(R1)
HLT" 0

test_assembler "
LI  R1, 0
LD  R0, 1(R1)
HLT" 1

test_assembler "
LI  R1, 100
ST  R1, 0(R1)
LD  R0, 0(R1)
HLT" 100

test_assembler "
LI  R0, 1
B   1
LI  R0, 2
HLT" 1

test_assembler "
LI  R0, 1
LI  R1, 1
CMP R0, R1
BE 1
LI  R0, 0
HLT" 1

test_assembler "
LI  R0, 1
LI  R1, 1
CMP R0, R1
BNE 1
LI  R0, 0
HLT" 0


### macro

test_macro(){
    echo "$1" | ./macro | ./assembler > _test.bin
    ./emulator _test.bin
    res=$?
    [ $res -eq $2 ] || fail "[ERROR] \"$1\": expect $2 but got $res"
}

test_macro "
MOV R1, 10
MOV R2, 20
ADD R1, R2
MOV [R1], R2
MOV [R1 + 1], R1
MOV R0, [R1 + 1]
HLT" 30

test_macro "
MOV R0, 5
MOV R1, 1
AND R0, R1
HLT" 1

test_macro "
MOV R0, 5
MOV R1, 2
OR R0, R1
HLT" 7

test_macro "
MOV R0, 5
MOV R1, 1
XOR R0, R1
HLT" 4

test_macro "
MOV R0, 1
SLR R0, 1
HLT" 2

test_macro "
MOV R0, 1
SLL R0, 15
SLR R0, 1
HLT" 1

test_macro "
MOV R0, 3
SRL R0, 1
HLT" 1

test_macro "
MOV R0, -2
SRA R0, 1
HLT" 255

echo "ok"
