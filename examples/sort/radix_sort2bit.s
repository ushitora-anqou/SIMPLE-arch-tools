define INFO_ADDR 0x7F
define DST_ADDR  0x10
define SRC_ADDR  0x30

define src_addr     R7
define digit_flag   R4
define src_limit10  R2
define i0x0400      R6
src_addr = SRC_ADDR
i0x0400 = 0x04
i0x0400 <<= 8

inline radix_loop_for (no, shiftw) {
define src_index  R0
define src_limit  R1
define t          R2
define n          R3
define dst        R5

	src_index = [src_addr + no]
	src_limit = [src_addr - no]

loop:
	if src_limit <= src_index then goto exit
	n = [src_index]
	src_index += 1
	t = n
	n &= digit_flag
	n >>= shiftw
	dst = [n + DST_ADDR]
	[dst] = t
	dst += 1
	[n + DST_ADDR] = dst
	goto loop
exit:

undef src_index
undef src_limit
undef t
undef n
undef dst
}

inline radix_assign0 () {
define src_limit00  R0
define src_limit01  R1
define src_limit11  R3
define n            R5
// !!! CAUTION !!! SAME AS src_limit10
define t            R2

	n = DST_ADDR
	src_limit00 = [n + 0]
	src_limit01 = [n + 1]
	src_limit10 = [n + 2]
	src_limit11 = [n + 3]
	[src_addr - 1] = src_limit00
	[src_addr - 2] = src_limit01
	[src_addr - 3] = src_limit10
	[src_addr - 4] = src_limit11

	t = i0x0400
	t += i0x0400	// 0x0800
	[src_addr + 1] = t
	t += i0x0400	// 0x0C00
	[src_addr + 2] = t
	t += i0x0400    // 0x1000
	[src_addr + 3] = t
	t += i0x0400    // 0x1400
	[src_addr + 4] = t
	t += i0x0400	// 0x1800
	[n + 0] = t
	t += i0x0400	// 0x1C00
	[n + 1] = t
	t += i0x0400    // 0x2000
	[n + 2] = t
	t += i0x0400    // 0x2400
	[n + 3] = t

undef src_limit00
undef src_limit01
undef src_limit11
undef n
undef t
}

inline radix_assign1 () {
define src_limit00  R0
define src_limit01  R1
define src_limit11  R3
define n            R5
// !!! CAUTION !!! SAME AS src_limit10
define t            R2

	n = DST_ADDR
	src_limit00 = [n + 0]
	src_limit01 = [n + 1]
	src_limit10 = [n + 2]
	src_limit11 = [n + 3]
	[src_addr - 1] = src_limit00
	[src_addr - 2] = src_limit01
	[src_addr - 3] = src_limit10
	[src_addr - 4] = src_limit11

	t = i0x0400
	t += i0x0400	// 0x0800
	[n + 0] = t
	t += i0x0400	// 0x0C00
	[n + 1] = t
	t += i0x0400    // 0x1000
	[n + 2] = t
	t += i0x0400    // 0x1400
	[n + 3] = t
	t += i0x0400	// 0x1800
	[src_addr + 1] = t
	t += i0x0400	// 0x1C00
	[src_addr + 2] = t
	t += i0x0400    // 0x2000
	[src_addr + 3] = t
	t += i0x0400    // 0x2400
	[src_addr + 4] = t

undef src_limit00
undef src_limit01
undef src_limit11
undef n
undef t
}

inline radix_loop_for_result_of(no) {
define src_index    R0
define src_limit    R1
define n            R3

	src_index = [src_addr + no]
	src_limit = [src_addr - no]

loop:
	if src_limit <= src_index then goto exit
	n = [src_index]
	src_index += 1
	[src_limit10] = n
	src_limit10 += 1
	goto loop

exit:

undef src_index
undef src_limit
undef n
}


define t       R1
define n       R2
	t = i0x0400
	[src_addr + 1] = t	// 0x0400
	t <<= 1
	[src_addr - 1] = t	// 0x0800
	n = DST_ADDR
	[n + 0] = t	// 0x0800
	t += i0x0400
	[n + 1] = t	// 0x0C00
	t += i0x0400
	[n + 2] = t	// 0x1000
	t += i0x0400
	[n + 3] = t	// 0x1400

	t = 0
	[src_addr + 2] = t	// 0
	[src_addr + 3] = t	// 0
	[src_addr + 4] = t	// 0
	[src_addr - 2] = t	// 0
	[src_addr - 3] = t	// 0
	[src_addr - 4] = t	// 0
undef t
undef n

digit_flag = 3

radix_loop_for(1, 0)
radix_assign0()

digit_flag <<= 2
radix_loop_for(1, 2)
radix_loop_for(2, 2)
radix_loop_for(3, 2)
radix_loop_for(4, 2)
radix_assign1()

digit_flag <<= 2
radix_loop_for(1, 4)
radix_loop_for(2, 4)
radix_loop_for(3, 4)
radix_loop_for(4, 4)
radix_assign0()

digit_flag <<= 2
radix_loop_for(1, 6)
radix_loop_for(2, 6)
radix_loop_for(3, 6)
radix_loop_for(4, 6)
radix_assign1()

digit_flag <<= 2
radix_loop_for(1, 8)
radix_loop_for(2, 8)
radix_loop_for(3, 8)
radix_loop_for(4, 8)
radix_assign0()

digit_flag <<= 2
radix_loop_for(1, 10)
radix_loop_for(2, 10)
radix_loop_for(3, 10)
radix_loop_for(4, 10)
radix_assign1()

digit_flag <<= 2
radix_loop_for(1, 12)
radix_loop_for(2, 12)
radix_loop_for(3, 12)
radix_loop_for(4, 12)
radix_assign0()

digit_flag <<= 2
radix_loop_for(1, 14)
radix_loop_for(2, 14)
radix_loop_for(3, 14)
radix_loop_for(4, 14)
radix_assign1()

src_limit10 = [src_addr - 3]
radix_loop_for_result_of(4)
radix_loop_for_result_of(1)
radix_loop_for_result_of(2)

HLT
