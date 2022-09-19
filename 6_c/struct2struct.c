#include <stdio.h>

struct A {
	char* name;
	int age;
};

struct B {
	struct A a;
	int weight;
};

void main(void)
{
	printf("\nHello world!\n");


	struct B b = {
		.a = {
			.name = "A_name",
			.age = 18,
		},
		.weight = 89,
	};	
	struct A* a = &b;
	printf("name=%s\n", a->name);


	struct A a2 = {
		.name = "A2",
		.age = 19,
	};
	struct B* b2 = &a2;
	printf("name=%s\n", b2->a.name);
}

