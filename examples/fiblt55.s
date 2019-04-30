	MOV R0, 0
	MOV R1, 1
	MOV R4, 55
loop:
	OUT R0
	MOV R3, R1
	ADD R1, R0
	MOV R0, R3
	CMP R0, R4
	JL  loop
exit:
	HLT
