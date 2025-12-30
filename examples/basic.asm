ADDI  $1, $0, 41	; Sets $1 to 41 since 0 + 41 = 41
ADDI  $2, $0, 28	; Sets $2 to 28 since 0 + 28 = 28
SUB   $3, $1, $2	; Potential RaW data hazards of both $1 and $2
XORI  $4, $3, 13	; Potential RaW data hazard of $3