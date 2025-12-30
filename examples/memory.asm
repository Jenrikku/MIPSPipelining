DEFB  $2, 4			; Defines a variable with value 4 and puts address into $2

LB	  $1, 0($2)		; Loads from memory, gets the result in the memory phase
ADDI  $1, $1, 4		; Guaranteed RaW data hazard of $1
SB	  $1, 0($2)		; Stores into memory, potential RaW data hazard of $1
SUB	  $1, $1, $0	; No data hazards
