#include <stdio.h>
#include "outer.h"

int abc = 123;

int add(int a, int b) {
	printf("this is in outer: (a+b)=%d\n", (a+b));
	return a+b;
}
