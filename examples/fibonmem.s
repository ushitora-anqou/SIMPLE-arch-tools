	MOV R7, 0x10	# SP
	MOV R0, 0
	MOV [R7 + 0], R0	# current
	MOV R0, 1
	MOV [R7 + 1], R0	# next
	MOV R0, 55
	MOV [R7 + 2], R0	# limit
loop:
	MOV R0, [R7 + 0]
	OUT R0
	MOV R1, [R7 + 2]
	CMP R1, R0
	JLE exit
	MOV R1, [R7 + 1]
	MOV [R7 + 0], R1
	ADD R0, R1
	MOV [R7 + 1], R0
	JMP loop
exit:
	HLT
