	MOV R0, 0
	MOV R1, 1
loop:
	OUT R0
	ADD R0, R1
	JMP loop
