#include <stdio.h>

struct pinctrl_desc {
	const char *name;
	unsigned int npins;
};

void main(void)
{
	printf("\nHello world!\n");
	int a = 1;
	int* aa = &a;
	printf("a=%x, a=%p, aa=%x, aa=%p\n\n", a, a, aa,aa);


	struct A {
		int i;
	};
	struct B {
		struct A a;
	};
	struct B b = {123};
	struct A* pa =&b;
	printf("b.a=%d\n", b.a);
	printf("&b=%p, &b.a=%p, &pa=%p, &pa->i=%p\n\n", &b, &b.a, pa, &pa->i);


	struct pinctrl_desc desc = {
		 .name = "test-pinctrl_desc",
		 .npins = 100,
	};
	
	struct pinctrl_desc* pdesc = &desc;
	char *name = pdesc;
	
	printf("pdesc=%p, &pdesc->name=%p, &desc.name=%p\n", pdesc, &pdesc->name, &desc.name);
	printf("pdesc->name=%p, desc.name=%p\n", pdesc->name, desc.name);
	printf("pdesc->name=%s, desc.name=%s\n", pdesc->name, desc.name);
}