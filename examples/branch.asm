		ADDI  $4, $0, 1		; Sets $4 to 1 since 0 + 1 = 1
		ADDI  $2, $0, 4		; Sets $2 to 4 since 0 + 4 = 4
LOOP:	SUB   $2, $2, $4	; Potential RaW data hazards of both $2 and $4
		BNE   $4, $2, LOOP	; Potential RaW data hazard of $4, taken twice
		XOR   $1, $4, $2	; No data hazards
