	MOV R0, 0
	MOV R1, 1
loop:
	MOV R3, R1
	ADD R1, R0
	MOV R0, R3
	JMP loop
