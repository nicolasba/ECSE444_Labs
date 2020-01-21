#include <stdio.h>
#include <add.h>

int main(){
	
	float a = 3.0;
	float b = 3.6;
	
	float ret_c = add_c(a, b);
	float ret_asm = add_asm(a, b);
	
	printf("C subroutine sum = %f\n", ret_c);
	printf("asm subroutine sum = %f\n", ret_asm);
	
	return 0;
}
