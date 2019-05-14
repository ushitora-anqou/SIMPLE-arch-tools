    define MAX 55
    define A0 0
    define A1 1

    R0 = A0
    R1 = A1
    R4 = MAX
loop:
    OUT R0
    R3 = R0
    R0 = R1
    R1 += R3
    if R0 <= R4 then goto loop
exit:
    HLT
