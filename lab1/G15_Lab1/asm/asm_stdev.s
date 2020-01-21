	AREA test, CODE, READONLY
	
	export asm_stdev        	   	; label must be exported if it is to be used as a function in C
asm_stdev
	PUSH{R3}						; saving context according to calling convention
	MOV R3, #0						; R3 temporarily stores value 0
	VMOV.f32 S0, R3					; Assign value 0 to register S0 (S0 is used to store the mean)
	VCVT.f32.s32 S0, S0				; Convert from signed int to 32-bit float
	PUSH{R1}					   	; backup size of array

loop1
	SUBS R1, R1, #1                 ; loop counter (N = N-1)
	BLT mean                        ; loop has finished?
	ADD R3, R0, R1, LSL #2          ; creating base address for the next element in array
	VLDR.f32 S1, [R3]				; load element from array into S1
	VADD.f32 S0, S1, S0				; compute sum which will be divided by N later
	B loop1

mean
	POP{R1}							; restore size of array
	VMOV.f32 S3, R1					; Move N to S3 temporarily to convert from int to float
	VCVT.f32.s32 S3, S3				; Convert from signed int to 32-bit float
	VDIV.f32 S0, S0, S3				; mean = total_sum / N
	PUSH{R1}						; backup size of array
	MOV R3, #0						; R3 temporarily stores value 0
	VMOV.f32 S2, R3					; Assign value 0 to register S2 (S2 is used to store the numerator of stdev formula)
	VCVT.f32.s32 S2, S2				; Convert from signed int to 32-bit float

loop2
	SUBS R1, R1, #1                 ; loop counter (N = N-1)
	BLT done                        ; loop has finished?
	ADD R3, R0, R1, LSL #2          ; creating base address for the next element in array
	VLDR.f32 S1, [R3]				; load element from array into S1
	VSUB.f32 S3, S1, S0				; register S3 will hold the value: A(i) - mean
	VMLA.f32 S2, S3, S3				; {multiply-accumulate: (A(i)-mean)^2} into S2 (numerator)
	B loop2
	
done
	POP{R1}							; restore size of array
	SUBS R1, R1, #1					; subtract 1 because we need N-1
	VMOV.f32 S3, R1					; Move N-1 to S3 temporarily to convert from int to float
	VCVT.f32.s32 S3, S3				; Convert from signed int to 32-bit float
	VDIV.f32 S2, S2, S3				; store (numerator/N-1) back into S2
	VSQRT.f32 S2, S2				; square root
	VSTR.f32 S2, [R2]              	; store the stdev into the res pointer stored in R2
	
	POP{R3}             			; restore context
	BX LR                           ; return
	
	END
