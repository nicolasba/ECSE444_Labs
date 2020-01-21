	AREA test, CODE, READONLY
	
	export asm_mult_array           ; label must be exported if it is to be used as a function in C
asm_mult_array
	PUSH{R4, R5, R6}                ; saving context according to calling convention	
	
loop
	SUBS R3, R3, #1                 ; loop counter (N = N-1)
	BLT done                        ; loop has finished?
	ADD R4, R0, R3, LSL #2          ; creating base address for the next element in array_a
	ADD R5, R1, R3, LSL #2          ; creating base address for the next element in array_b
	ADD R6, R2, R3, LSL #2          ; creating base address for the next element in array_res
	VLDR.f32 S0, [R4]               ; load element from array_a into S0
	VLDR.f32 S1, [R5]               ; load element from array_b into S1
	VMUL.f32 S2, S0, S1				; multiply elements from array_a and array_b
	VSTR.f32 S2, [R6]               ; store the mult result in the pointer to array_res
	B loop
	
done
	POP{R4, R5, R6}                 ; restore context
	BX LR                           ; return
	
	END