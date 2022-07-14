#include <stdio.h>


int add(int a, int b)
{
	return a + b;
}

int main(void)
{
	int (*fn)(int, int) = add;
	int a = fn(1, 2);

	printf("a=%d\n", a);

	return 0;
}