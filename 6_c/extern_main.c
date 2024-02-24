#include <stdio.h>
extern int abc;
int main(void)
{
	printf("\nHello world!\n");
	printf("extern int abc=%d\n", abc);
	return 0;
}