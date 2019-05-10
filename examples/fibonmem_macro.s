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
    OUT R0
    R1 = [R7 + 2]
    if R1 <= R0 then goto exit
    R1 = [R7 + 1]
    [R7 + 0] = R1
    R0 += R1
    [R7 + 1] = R0
    goto loop

exit:
    HLT
