#include <stdio.h>
#include "outer.h"
extern int abc;
int main(void)
{
	printf("\nthis is in main!\n");
	printf("call extern int add(a, b)=%d\n", add(1, 3));
	return 0;
}
