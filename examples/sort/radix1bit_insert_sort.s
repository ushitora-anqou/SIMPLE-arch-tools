    define INFO_ADDR 0x7F

// 1BIT RADIX SORT ---------- FROM HERE
    define src_index0 R0
    define src_index1 R1
    define dst_index0 R2
    define dst_index1 R3
    define src_limit0 R4
    define src_limit1 R5
    define digit_flag R6

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
    digit_flag = 1
	digit_flag <<= 8

    // R0 == 0x0400 // src_index0
    // R1 == 0x0000 // src_index1
    // R2 == 0x0800 // dst_index0
    // R3 == 0x0C00 // dst_index1
    // R4 == 0x0800 // src_limit0
    // R5 == 0x0000 // src_limit1
    // R6 == ?      // digit_flag
    // R7           // n

radix_main:
    if digit_flag == 0 then goto radix_exit

// loop for 0
radix_loop0:
    if src_limit0 <= src_index0 then goto radix_loop1

    R7 = [src_index0]
    src_index0 += 1
    [dst_index0] = R7
    [dst_index1] = R7

    R7 &= digit_flag
    if R7 != 0 then goto radix_loop0_else
        dst_index0 += 1
        goto radix_loop0_end
    radix_loop0_else:
        dst_index1 += 1
radix_loop0_end:
    goto radix_loop0

// loop for 1
radix_loop1:
    if src_limit1 <= src_index1 then goto radix_loops_end

    R7 = [src_index1]
    src_index1 += 1
    [dst_index0] = R7
    [dst_index1] = R7

    R7 &= digit_flag

    if R7 != 0 then goto radix_loop1_else
        dst_index0 += 1
        goto radix_loop1_end
    radix_loop1_else:
        dst_index1 += 1
radix_loop1_end:
    goto radix_loop1

radix_loops_end:
    src_limit0 = dst_index0
    src_limit1 = dst_index1

    R7 = INFO_ADDR
    R0 = [R7 + 4]
    if R0 != 0 then goto scene_else
        R0 = 1
        [R7 + 4] = R0   // set scene = 1

        src_index0 = [R7 + 0]
        src_index1 = [R7 + 1]
        dst_index0 = [R7 + 2]
        dst_index1 = [R7 + 3]
        goto radix_scene_end
    scene_else:
        R0 = 0
        [R7 + 4] = R0   // set scene = 0

        src_index0 = [R7 + 2]
        src_index1 = [R7 + 3]
        dst_index0 = [R7 + 0]
        dst_index1 = [R7 + 1]
radix_scene_end:
    digit_flag <<= 1
    goto radix_main

radix_exit:
	R7 = INFO_ADDR
	[R7 + 0] = src_index0
	[R7 + 1] = src_limit0

    undef src_index0
    undef src_index1
    undef dst_index0
    undef dst_index1
    undef src_limit0
    undef src_limit1
    undef digit_flag
// 1BIT RADIX SORT ---------- TO HERE

// INSERT SORT -------------- FROM HERE
	define src   R1
	define limit R5
	define i     R0
	define j     R2
	define val   R3
	define t     R4
	define di    R6
	define dst   R7

	dst = src	// dst <- src_index1

// INSERT SORT FOR 1  ------- FROM HERE
	t = [src]
	[dst] = t
	di = dst

	i = src

insert_for_1_main:
	i += 1
	di += 1
	if limit <= i then goto insert_for_1_end

	val = [i]
	t = [di - 1]
	[di] = val

	if t <= val then goto insert_for_1_main

	j = di
insert_for_1_inner_loop:
	[j] = t
	j += -1
	t = [j - 1]

	if j <= dst then goto insert_for_1_inner_loop_end
	if t <= val then goto insert_for_1_inner_loop_end
	goto insert_for_1_inner_loop
insert_for_1_inner_loop_end:
	[j] = val
	goto insert_for_1_main

insert_for_1_end:
// INSERT SORT FOR 1 -------- TO HERE

	dst = di
	R0 = INFO_ADDR
	src = [R0 + 0]
	limit = [R0 + 1]

// INSERT SORT FOR 0  ------- FROM HERE
	t = [src]
	[dst] = t
	di = dst

	i = src

insert_for_0_main:
	i += 1
	di += 1
	if limit <= i then goto insert_for_0_end

	val = [i]
	t = [di - 1]
	[di] = val

	if t <= val then goto insert_for_0_main

	j = di
insert_for_0_inner_loop:
	[j] = t
	j += -1
	t = [j - 1]

	if j <= dst then goto insert_for_0_inner_loop_end
	if t <= val then goto insert_for_0_inner_loop_end
	goto insert_for_0_inner_loop
insert_for_0_inner_loop_end:
	[j] = val
	goto insert_for_0_main

insert_for_0_end:
// INSERT SORT FOR 0 -------- TO HERE

	HLT

// INSERT SORT -------------- TO HERE
