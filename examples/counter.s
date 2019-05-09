	MOV R0, 0
loop:
	OUT R0
	ADD R0, 1
	CMP R0, 10
	JL  loop
exit:
	HLT
