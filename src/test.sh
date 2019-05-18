#!/usr/bin/bash

fail() {
    echo $1
    exit 1
}

test_emulator(){
    echo "$1" | ./encoder | ./emulator -q
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
# reg[0] <- 1; reg[1] <- 2; CMP(reg[0], reg[1]); BLT 1; reg[0] <- 0; HLT
test_emulator "c[0 0 1] c[0 1 2] a[1 0 5 0] d[1 1] c[0 0 0] a[0 0 15 0]" 1
# reg[0] <- 2; reg[1] <- 2; CMP(reg[0], reg[1]); BLE 1; reg[0] <- 0; HLT
test_emulator "c[0 0 2] c[0 1 2] a[1 0 5 0] d[2 1] c[0 0 0] a[0 0 15 0]" 2
# reg[0] <- 1; reg[1] <- 1; CMP(reg[0], reg[1]); BNE 1; reg[0] <- 0; HLT
test_emulator "c[0 0 1] c[0 1 1] a[1 0 5 0] d[3 1] c[0 0 0] a[0 0 15 0]" 0

## These tests assume that the target machine has von Neumann architecture.
## reg[1] <- 0; LD(reg[0], 0(reg[1])); HLT"
#test_emulator "c[0 1 0] b[0 0 1 0] a[0 0 15 0]" 0
## reg[1] <- 0; LD(reg[0], 1(reg[1])); HLT"
#test_emulator "c[0 1 0] b[0 0 1 1] a[0 0 15 0]" 1
# reg[1] <- 100; ST(reg[1], 0(reg[1])); LD(reg[0], 0(reg[1])); HLT"
test_emulator "c[0 1 100] b[1 1 1 0] b[0 0 1 0] a[0 0 15 0]" 100


### asm

test_assembler(){
    echo -n "$1" | ./assembler | ./emulator -q
    res=$?
    [ $res -eq $2 ] || fail "[ERROR] \"$1\": expect $2 but got $res"
}

test_assembler "
LI  R1, 3  // hoge piyo
MOV R0, R1 // hoge piyo
HLT // hoge piyo" 3

test_assembler "
LI  R0, 3  // hoge piyo
ADD R0, R0 // hoge piyo
HLT" 6

test_assembler "
LI  R0, 3 // hoge piyo
SUB R0, R0// hoge piyo
HLT" 0

test_assembler "
LI  R0, 5 // hoge piyo
LI  R1, 1 // hoge piyo
AND R0, R1
HLT" 1

test_assembler "
LI  R0, 5
LI  R1, 2 // hoge piyo
OR  R0, R1// hoge piyo
HLT" 7

test_assembler "
LI  R0, 5
LI  R1, 1
XOR R0, R1
HLT" 4

test_assembler "
LI  R0, 1
SLR R0, 1// hoge piyo
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
LI  R0, -30
SRA R0, 3
HLT" 252    # actually -4

test_assembler "
LI  R1, 0
LD  R0, 0(R1)
HLT" 0

# This test assumes that the target machine has von Neumann architecture.
#test_assembler "
#LI  R1, 0
#LD  R0, 1(R1)
#HLT" 1

test_assembler "
LI  R1, 100   // hoge piyo
ST  R1, 0(R1) // hoge piyo
LD  R0, 0(R1) // hoge piyo
HLT" 100

test_assembler "
LI  R0, 1
B   1// hoge piyo
LI  R0, 2
HLT" 1

test_assembler "
LI  R0, 1
LI  R1, 1
CMP R0, R1//hoge hoge piyo
BE 1//hoge hoge piyo
LI  R0, 0
HLT" 1

test_assembler "
LI  R0, 1
LI  R1, 2
CMP R0, R1
BLT 1//hoge hoge piyo
LI  R0, 0//hoge hoge piyo
HLT" 1

test_assembler "
LI  R0, 1
LI  R1, 1
CMP R0, R1//hoge hoge piyo
BLE 1//hoge hoge piyo
LI  R0, 0
HLT" 1

test_assembler "
LI  R0, -1
LI  R1, 2
CMP R0, R1
BLT 1
LI  R0, 0
HLT" 255

test_assembler "
LI  R0, -1
LI  R1, -1
CMP R0, R1
BLE 1
LI  R0, 0
HLT" 255

test_assembler "
LI  R0, 1
LI  R1, 1
CMP R0, R1
BNE 1
LI  R0, 0
HLT" 0

test_assembler "
LI   R0, 1//hoge hoge piyo
ADDI R0, 5//hoge hoge piyo
HLT" 6

test_assembler "
LI   R0, 1//hoge hoge piyo
CMPI R0, 2//hoge hoge piyo
BE   1
LI   R0, 0
HLT" 0

test_assembler_in_mif_format(){
    res=$(echo "$1" | ./assembler -mif)
    diff -bB <(echo "$res") <(echo "$2") || \
        fail "[ERROR] \"$1\": expect $2 but got $res"
}

test_assembler_in_mif_format "
LI R3, 8
LI R5, -2
ADD R3, R5
B -2" "
0000 : 8308;
0001 : 85FE;
0002 : EB00;
0003 : A0FE;"

### macro

test_macro(){
    echo "$1" | ./macro | ./assembler | ./emulator -q
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
MOV R1, +10
MOV R2, +20
ADD R1, R2
MOV [R1 - 1], R2
MOV [R1 + 1], R1
MOV R0, [R1 - 1]
HLT" 20

test_macro "
MOV R1, -0xA
MOV R2, 0x14
ADD R1, R2
MOV [R1], R2
MOV [R1 + 1], R1
MOV R0, [R1 + 1]
HLT" 10

test_macro "
MOV R1, 0x0A
MOV R2, 0x14
ADD R1, R2
MOV [R1 - 1], R2
MOV [R1 + 1], R1
MOV R0, [R1 - 1]
HLT" 20

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

test_macro "
MOV R0, 2
JMP exit
MOV R0,-1
exit:
HLT" 2

test_macro "
    MOV R0, 0
    MOV R1, 10
    MOV R2, 1
loop:
    ADD R0, R2
    CMP R0, R1
    JE loop
    HLT" 1

test_macro "
    MOV R0, 0
    MOV R1, 10
    MOV R2, 1
loop:
    ADD R0, R2
    CMP R0, R1
    JNE loop
    HLT" 10

test_macro "
    MOV R0, 0
    MOV R1, 10
    MOV R2, 1
loop:
    ADD R0, R2
    CMP R0, R1
    JL loop
    HLT" 10

test_macro "
    MOV R0, 0
    MOV R1, 10
    MOV R2, 1
loop:
    ADD R0, R2
    CMP R0, R1
    JLE loop
    HLT" 11

# This test assumes that SP will be read properly
#test_macro "
#    MOV R0, 0
#    MOV SP, 10
#    MOV R2, 1
#loop:
#    ADD R0, R2
#    CMP R0, SP
#    JLE loop
#    HLT" 11

# This test assumes that the target processor has CALL/RET insts.
#test_macro "
#    JMP main
#sub2:# hoge
#    MOV R0, 20
#    RET
#sub:
#    MOV R2, R6
#    CALL sub2
#    MOV R6, R2  # comment
#    # comcom
#    MOV R1, 10
#    ADD R0, R1
#    RET
#main:
#    CALL sub
#    HLT" 30

test_macro "
    MOV R0, 1   // comcom
    ADD R0, 2   # comcom
    CMP R0, 3
    JE  exit
    MOV R0, 0
exit:
    HLT
" 3

test_macro "
    define hoge 3
    MOV R0, hoge
    ADD R0, -2
    HLT
" 1

test_macro "
    define add_to_r0 ADD R0, 1
    MOV R0, 1
    add_to_r0
    HLT
" 2

test_macro "
    define hoge 3
    R0 = hoge
    HLT
" 3

test_macro "
    define sp R7
    define val 5
    sp = 10

    R0 = val
    [sp + 10] = R0
    R0 = [sp + 10]
    HLT
" 5

test_macro "
    define sp R7
    define val 5
    define val2 10
    sp = val2

    R0 = val
    [sp + val2] = R0
    R0 = [sp + val2]
    HLT
" 5

test_macro "
    define sp R7
    define val 5
    define val2 1
    sp = val2

    R0 = val
    R0 += val2
    R0 -= R0
    R0 = 1
    R0 <<= 1
    R0 >>= 1

    [sp + val2] = R0
    R0 = [sp + val2]
    HLT
" 1

test_macro "
    define sp R7
    define val 5
    define val2 1
    sp = val2

    R0 = val
    if R0 == 5 then goto exit
    R0 += val2
    R0 -= R0
    R0 = 1
    R0 <<= 1
    R0 >>= 1

    [sp + val2] = R0
    R0 = [sp + val2]
exit:
    HLT
" 5

test_macro "
    define MAX 55

    R0 = 2
    R1 = 1
    R4 = MAX
loop:
    R3 = R0
    R0 = R1
    R1 += R3
    if R0 <= R4 then goto loop
exit:
    HLT
" 76

test_macro "
    define MAX 76

    R0 = 2
    R1 = 1
    R4 = MAX
loop:
    R3 = R0
    R0 = R1
    R1 += R3
    if R0 != R4 then goto loop
exit:
    HLT
" 76

test_macro "
    define SP_BASE 0x10
    define A0 0
    define A1 1
    define LIMIT 55

    R7 = SP_BASE
    R0 = A0
    [R7 + 0] = R0
    R0 = A1
    [R7 + 1] = R0
    R0 = LIMIT
    [R7 + 2] = R0

loop:
    R0 = [R7 + 0]
    R1 = [R7 + 2]
    if R1 <= R0 then goto exit
    R1 = [R7 + 1]
    [R7 + 0] = R1
    R0 += R1
    [R7 + 1] = R0
    goto loop

exit:
    R0 = [R7 + 0]
    HLT
" 55

test_macro "
R0 = 5
R1 = 6
R0 &= R1
HLT" 4

test_macro "
R0 = 5
R1 = 6
R0 |= R1
HLT" 7

test_macro "
define hoge R1
hoge = 3
undef hoge
define hoge R0
hoge = 5
undef hoge
define hoge R1
hoge = 0
undef hoge
define hoge R0
hoge = 7
define foo R7
foo = 2
HLT" 7

test_macro "
R0\\
    = \\
    5
R\\
1 = 6
R0 &\\
= R\\
1
H\\
LT" 4

test_macro "
define hoge \\
    MOV R0, R2 \\
    ADD R0, R3

R0 = 2
R2 = 3
R3 = 7
hoge
HLT" 10

test_macro "
define hoge \\
    MOV R0, R2 \\
    ADD R0, R3

R0\\
\\
\\
\\
= 2
R2 = 3
R3 = 7
hoge
HLT" 10

test_macro "
begin(hoge)
    R0 = 0
foo:
    R0 += 1
    if R0 <= 1 then goto foo
end(hoge)

begin(piyo)
foo:
    R0 += 1
    if R0 <= 3 then goto foo
end(piyo)
HLT" 4

test_macro "
MOV R0, 10
MOV R1, 100
ST R0, -20(R1)
MOV R0, -1
LD R0, -20(R1)
HLT
" 10

test_macro "
MOV R0, 10
MOV R1, 100
ST R0, 20(R1)
MOV R0, -1
LD R0, 20(R1)
HLT
" 10

test_macro "
MOV R0, 10
MOV R1, 100
ST R0, (R1)
MOV R0, -1
LD R0, (R1)
HLT
" 10

test_macro "
LI R0, 100
HLT" 100

test_macro "
MOV R0, 2
B exit
MOV R0,-1
exit:
HLT" 2

test_macro "
    MOV R0, 0
    MOV R1, 10
    MOV R2, 1
loop:
    ADD R0, R2
    CMP R0, R1
    BLT loop
    HLT" 10

test_macro "
    MOV R0, 0
    MOV R1, 10
    MOV R2, 1
loop:
    ADD R0, R2
    CMP R0, R1
    BLE loop
    HLT" 11

test_macro "LI R0, 10 HLT  #\n" 10

test_macro "
inline hoge(a, b) {
    a += b
}
R0 = 5
R1 = 10
hoge(R0, R1)
HLT" 15

test_macro "
inline hoge(a, b) {
loop:
    a += b
	if a <= b then goto loop
}
R0 = 5
R1 = 10
hoge(R0, R1)
R1 = R0
hoge(R0, R1)
HLT" 30

test_macro "
inline fibstep() {
    R3 = R0
    R0 = R1
    R1 += R3
}

    define MAX 55
    define A0 0
    define A1 1

    R0 = A0
    R1 = A1
    R4 = MAX
loop:
    fibstep()
    if R0 < R4 then goto loop
exit:
    HLT" 55

test_macro "
inline main() {
    define SP_BASE 0x10
    define A0 0
    define A1 1
    define LIMIT 55

    R7 = SP_BASE
    R0 = A0
    [R7 + 0] = R0
    R0 = A1
    [R7 + 1] = R0
    R0 = LIMIT
    [R7 + 2] = R0

loop:
    R0 = [R7 + 0]
    R1 = [R7 + 2]
    if R1 <= R0 then goto exit
    R1 = [R7 + 1]
    [R7 + 0] = R1
    R0 += R1
    [R7 + 1] = R0
    goto loop

exit:
    HLT
}

    main()
" 55


test_macro_error() {
    res=$(echo -n "$1" | ./macro 2>&1)
    echo $res | egrep "$2" > /dev/null
    [ $? -eq 0 ] ||\
        fail "[ERROR] \"$1\": expect \"$2\" but got $res"
}

test_macro_error "
ADD R0, R2
%" "3:1:.+Unrecognized character.+%"

test_macro_error "
ADD R0, hoge
" "2:9:.+Unexpected token.+register"

test_macro_error "
ADD R0,
" "3:1:.+Unexpected EOF"

test_macro_error "
define hoge
define hoge" "3:8:.+macro with the same name"

test_macro_error "JMP hoge" "Undeclared label.+hoge"

test_macro_error "R1 += +=" "1:7:.+Unexpected token.+register"

test_macro_error "end(hoge)" "1:1:.+Invalid end of label namespace"

test_macro_error "begin(hoge) begin(foo)" "1:13:.+Nested label namespace is not allowed"

test_macro_error "begin(hoge) end(foo)" "1:13:.+Invalid end of label namespace"

test_macro_error "
define hoge (a, b) ADD a, b
R0 = 3
R1 = 5
hoge(R0, R1)
HLT" "5:1:.+Unexpected token.+'\('"

test_macro_error "
define hoge \\
    MOV R0, R2 \\
    ADD R0, R3

R0\\
\\
\\
\\
= ,2
R2 = 3
R3 = 7
hoge
HLT" "10:3:.+Unexpected token.+','"

test_macro_error "
MOV R0, [R0 + 128]" "Unexpected token.+'128'.+-128, 127"

test_macro_error "
MOV R0, [R0 - 129]" "Unexpected token.+'-129'.+-128, 127"

test_macro_error "
ADD R0, 8" "Unexpected token.+'8'.+-8, 7"

test_macro_error "
ADD R0, -9" "Unexpected token.+'-9'.+-8, 7"

test_macro_error "
CMP R0, 8" "Unexpected token.+'8'.+-8, 7"

test_macro_error "
CMP R0, -9" "Unexpected token.+'-9'.+-8, 7"

test_macro_error "
SLL R0, 16" "Unexpected token.+'16'.+0, 15"

test_macro_error "
SLL R0, -1" "Unexpected token"

echo "ok"
