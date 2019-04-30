MOV R1, 5
MOV R2, 42
MOV [R1 + 2], R2	# [7] <- 42
MOV R3, [R1 + 2]	# R3 <- 42
MOV [R2 - 2], R1	# [40] <- 5
MOV R4, [R2 - 2]	# R4 <- 5
HLT
