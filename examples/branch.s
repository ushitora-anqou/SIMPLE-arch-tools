	MOV R2, 1
	MOV R1, 1
	MOV R0, 0
loop:
	ADD R0, R1
	CMP R0, R2
	JLE loop
	HLT
