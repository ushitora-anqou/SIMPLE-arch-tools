    define INFO_ADDR 0x7F

    define src_index0 R0
    define src_index1 R1
    define dst_index0 R2
    define dst_index1 R3
    define src_limit0 R4
    define src_limit1 R5
    define digit_flag R6

init:
    R7 = INFO_ADDR
    R0 = 0x04
    R0 <<= 8    // R0 == 0x0400

    // save P0 = 0x0800
    R1 = R0
    R1 += R1
    [R7 + 0] = R1
    R2 = R1
    R4 = R1

    // save P1 = 0x0C00
    R1 += R0
    [R7 + 1] = R1
    R3 = R1

    // save Q0 = 0x1000
    R1 += R0
    [R7 + 2] = R1

    // save Q1 = 0x1400
    R1 += R0
    [R7 + 3] = R1

    // save scene = 0
    R1 = 0
    [R7 + 4] = R1

    R1 = 0
    R5 = 0
    R6 = 1

    // R0 == 0x0400 // src_index0
    // R1 == 0x0000 // src_index1
    // R2 == 0x0800 // dst_index0
    // R3 == 0x0C00 // dst_index1
    // R4 == 0x0800 // src_limit0
    // R5 == 0x0000 // src_limit1
    // R6 == 1      // digit_flag
    // R7           // n

main:
    if digit_flag == 0 then goto loop_for_result

// loop for 0
loop0:
    if src_limit0 <= src_index0 then goto loop1

    R7 = [src_index0]
    src_index0 += 1
    [dst_index0] = R7
    [dst_index1] = R7

    R7 &= digit_flag
    if R7 != 0 then goto loop0_else
        dst_index0 += 1
        goto loop0_end
    loop0_else:
        dst_index1 += 1
loop0_end:
    goto loop0

// loop for 1
loop1:
    if src_limit1 <= src_index1 then goto loops_end

    R7 = [src_index1]
    src_index1 += 1
    [dst_index0] = R7
    [dst_index1] = R7

    R7 &= digit_flag

    if R7 != 0 then goto loop1_else
        dst_index0 += 1
        goto loop1_end
    loop1_else:
        dst_index1 += 1
loop1_end:
    goto loop1

loops_end:
    R7 = INFO_ADDR
    R0 = [R7 + 4]	// prevent LD's stall

    src_limit0 = dst_index0
    src_limit1 = dst_index1

    if R0 != 0 then goto scene_else
        R0 = 1
        [R7 + 4] = R0   // set scene = 1

        src_index0 = [R7 + 0]
        src_index1 = [R7 + 1]
        dst_index0 = [R7 + 2]
        dst_index1 = [R7 + 3]
        goto scene_end
    scene_else:
        R0 = 0
        [R7 + 4] = R0   // set scene = 0

        src_index0 = [R7 + 2]
        src_index1 = [R7 + 3]
        dst_index0 = [R7 + 0]
        dst_index1 = [R7 + 1]
scene_end:
    digit_flag <<= 1
    goto main

loop_for_result:
    if src_limit0 <= src_index0 then goto exit

    R7 = [src_index0]
    src_index0 += 1
    [src_limit1] = R7
    src_limit1 += 1
    goto loop_for_result

exit:
    HLT
