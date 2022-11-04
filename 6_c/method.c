#include <stdio.h>
#define LIST_INNIT_LENTH 100

typedef int (*smp_call_func_t)(void *info);

typedef struct info {
	int arg0;
	int arg1;
} info_t;

int add(int a, int b)
{
	return a + b;
}

int handle(void* info){
	info_t* tmp = (info_t*)info;
	return tmp->arg0 + tmp->arg1;
}

int main(void)
{
	int (*fn)(int, int) = add;
	int a = fn(1, 2);
	printf("a=%d\n", a);

	smp_call_func_t func = handle;
	info_t info;
	info.arg0 = LIST_INNIT_LENTH;
	info.arg1 = 2;

	int b = func(&info);
	printf("b=%d\n", b);
	return 0;
}