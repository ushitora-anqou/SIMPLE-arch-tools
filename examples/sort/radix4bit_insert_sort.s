// 4BIT RADIX SORT FROM HERE
	define DST_ADDR 0x10

	alloc dst_addr   R4
	alloc dst        R5
	alloc digit_flag R6
	alloc i0x400     R7

	dst_addr = DST_ADDR
	digit_flag = 0x0F
	i0x400 = 4
	i0x400 <<= 8

inline radix_loop_unrolled_detail(shiftw, src_index, src_limit, n, tmp) {
	n = [src_index]
	src_index += 1
	tmp = n
	n >>= shiftw
	n &= digit_flag
	dst = [n + DST_ADDR]
	[dst] = tmp
	dst += 1
	[n + DST_ADDR] = dst
}

inline radix_loop_unrolled(shiftw, initial_src_index_lsr8) {
	alloc src_index R0
	alloc k         R1
	alloc n         R2
	alloc tmp       R3

	src_index = initial_src_index_lsr8
	src_index <<= 8
	k = 0

loop:
	tmp = 64
	if k != tmp then goto letsgo
	JMP exit
letsgo:
	<% 16.times do %>
		<%= "radix_loop_unrolled_detail(shiftw, src_index, src_limit, n, tmp)" %>
	<% end %>
	k += 1
	goto loop
exit:
}

inline radix_loop(shiftw, initial_src_index_lsr8, initial_src_limit_lsr8) {
	alloc src_index R0
	alloc src_limit R1
	alloc n         R2
	alloc tmp       R3

	src_index = initial_src_index_lsr8
	src_index <<= 8
	src_limit = initial_src_limit_lsr8
	src_limit <<= 8

loop:
	if src_limit <= src_index then goto exit
	n = [src_index]
	src_index += 1
	tmp = n
	n >>= shiftw
	n &= digit_flag
	dst = [n + DST_ADDR]
	[dst] = tmp
	dst += 1
	[n + DST_ADDR] = dst
	goto loop

exit:
}

inline radix_rearrange_detail(initial_src_index_lsr8, dst_addr_disp) {
	alloc src_index R0
	alloc src_limit R1
	alloc tmp       R2

	src_index = initial_src_index_lsr8
	src_index <<= 8
	src_limit = [dst_addr + dst_addr_disp]

loop:
	if src_limit <= src_index then goto exit
	tmp = [src_index]
	[dst] = tmp
	dst += 1
	src_index += 1
	goto loop

exit:
}

inline radix_rearrange() {
	dst = [dst_addr]
	radix_rearrange_detail(0x0C, 0x01)
	radix_rearrange_detail(0x10, 0x02)
	radix_rearrange_detail(0x14, 0x03)
	radix_rearrange_detail(0x18, 0x04)
	radix_rearrange_detail(0x1C, 0x05)
	radix_rearrange_detail(0x20, 0x06)
	radix_rearrange_detail(0x24, 0x07)
	radix_rearrange_detail(0x28, 0x08)
	radix_rearrange_detail(0x2C, 0x09)
	radix_rearrange_detail(0x30, 0x0A)
	radix_rearrange_detail(0x34, 0x0B)
	radix_rearrange_detail(0x38, 0x0C)
	radix_rearrange_detail(0x3C, 0x0D)
	radix_rearrange_detail(0x40, 0x0E)
	radix_rearrange_detail(0x44, 0x0F)
}

inline radix_rearrange_for_result() {
	dst = [dst_addr + 0x08]
	radix_rearrange_detail(0x2C, 0x09)
	radix_rearrange_detail(0x30, 0x0A)
	radix_rearrange_detail(0x34, 0x0B)
	radix_rearrange_detail(0x38, 0x0C)
	radix_rearrange_detail(0x3C, 0x0D)
	radix_rearrange_detail(0x40, 0x0E)
	radix_rearrange_detail(0x44, 0x0F)
	radix_rearrange_detail(0x08, 0x00)
	radix_rearrange_detail(0x0C, 0x01)
	radix_rearrange_detail(0x10, 0x02)
	radix_rearrange_detail(0x14, 0x03)
	radix_rearrange_detail(0x18, 0x04)
	radix_rearrange_detail(0x1C, 0x05)
	radix_rearrange_detail(0x20, 0x06)
	radix_rearrange_detail(0x24, 0x07)
}

inline radix_assign() {
	alloc tmp R0

	tmp = i0x400
	tmp += i0x400
	[dst_addr + 0x00] = tmp
	tmp += i0x400
	[dst_addr + 0x01] = tmp
	tmp += i0x400
	[dst_addr + 0x02] = tmp
	tmp += i0x400
	[dst_addr + 0x03] = tmp
	tmp += i0x400
	[dst_addr + 0x04] = tmp
	tmp += i0x400
	[dst_addr + 0x05] = tmp
	tmp += i0x400
	[dst_addr + 0x06] = tmp
	tmp += i0x400
	[dst_addr + 0x07] = tmp
	tmp += i0x400
	[dst_addr + 0x08] = tmp
	tmp += i0x400
	[dst_addr + 0x09] = tmp
	tmp += i0x400
	[dst_addr + 0x0A] = tmp
	tmp += i0x400
	[dst_addr + 0x0B] = tmp
	tmp += i0x400
	[dst_addr + 0x0C] = tmp
	tmp += i0x400
	[dst_addr + 0x0D] = tmp
	tmp += i0x400
	[dst_addr + 0x0E] = tmp
	tmp += i0x400
	[dst_addr + 0x0F] = tmp
}

radix_assign()
#radix_loop(8, 0x04, 0x08)
radix_loop_unrolled(8, 0x04)
radix_rearrange()

radix_assign()
#radix_loop(12, 0x08, 0x0C)
radix_loop_unrolled(12, 0x08)

free dst
free digit_flag
free i0x400
// 4BIT RADIX SORT TO HERE

// INSERT FROM HERE
inline insert_routine_detail(initial_src_index_lsr8, dst_addr_disp, dst_addr, dst) {
	super dst_addr, dst
	alloc src, src_limit, tmp, di, val, j

	src = initial_src_index_lsr8
	src <<= 8
	src_limit = [dst_addr + dst_addr_disp]
	tmp = [src]
	[dst] = tmp
	di = dst

loop:
	src += 1
	di += 1

	if src_limit <= src then goto exit

	val = [src]
	tmp = [di - 1]
	[di] = val

	if tmp <= val then goto loop

	j = di
	begin
	loop:
		[j] = tmp
		j += -1
		tmp = [j - 1]

		if j <= dst then goto exit
		if tmp <= val then goto exit
		goto loop
	exit:
	end
	[j] = val

	goto loop
exit:
	dst = di
}

alloc dst
dst = 0x28
dst <<= 8
insert_routine_detail(0x28, 0x08, dst_addr, dst)
insert_routine_detail(0x2C, 0x09, dst_addr, dst)
insert_routine_detail(0x30, 0x0A, dst_addr, dst)
insert_routine_detail(0x34, 0x0B, dst_addr, dst)
insert_routine_detail(0x38, 0x0C, dst_addr, dst)
insert_routine_detail(0x3C, 0x0D, dst_addr, dst)
insert_routine_detail(0x40, 0x0E, dst_addr, dst)
insert_routine_detail(0x44, 0x0F, dst_addr, dst)
insert_routine_detail(0x08, 0x00, dst_addr, dst)
insert_routine_detail(0x0C, 0x01, dst_addr, dst)
insert_routine_detail(0x10, 0x02, dst_addr, dst)
insert_routine_detail(0x14, 0x03, dst_addr, dst)
insert_routine_detail(0x18, 0x04, dst_addr, dst)
insert_routine_detail(0x1C, 0x05, dst_addr, dst)
insert_routine_detail(0x20, 0x06, dst_addr, dst)
insert_routine_detail(0x24, 0x07, dst_addr, dst)

// INSERT TO HERE

halt
