#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void ppoint(char * as) {
	printf("as=%p, *as=%c\n", as, *as);
	as[0] = 'a';
}

void ppoint2(char ** m) {
	printf("m=%p, *m=%p, (*m)[1]=%c\n", m, *m, (*m)[1]);
//	(*m)[1] = 'a';

}

int main()
{
	printf("-------------------------------------\n");
	char as[4] = "123";as[3] = '\0';
	char *ss = as;
	printf("ss=%p, &ss=%p, ss=%s, as=%s\n", ss, &ss, ss, as);	
	printf("as=%p, &as=%p, &as[0]=%p, &as[1]=%p\n", as, &as, &(as[0]), &(as[1]));
	printf("(char*)\"123\"=%p, &\"123\"=%p\n", (char *)"123", &"123");
	
	ppoint(as);
	printf("%s\n", as);
	printf("%d, %d\n", 1 == 1, 1 == 2);
	printf("%d, %p, %p\n", *(as+1) == as[1], as+1, &as[1]);

	char **aa = &ss;
	printf("aa=%p, &ss=%p, *aa=%p, ss=%p, *aa=%s\n", aa, &ss, *aa, ss, *aa);
	ppoint2(aa);
	printf("aa=%p, &ss=%p, *aa=%p, ss=%p, *aa=%s, ss=%s, as=%s\n", aa, &ss, *aa, ss, *aa, ss, as);

	char *a = (char*)"123";
	char *const K = (char *)"123";
	//a[1] = 'H'; //常量不能改
	printf("a[2]=%c\n", a[2]);

	
	int b = 456;
	int *bp = &b;
	printf("bp=%p, &bp=%p\n", bp, &bp);

	int c[] = {1,2,3};
	int *d = c;
	int **e = &d;
	printf("c[1]=%d, d[1]=%d, (*e)[1]=%d \n", c[1], d[1], (*e)[1]);
	

	int (*f)[3] =&c;
	printf("f=%p, c=%p, &c=%p, *f[1]=%d, (*f)[1]=%d, c[1]=%d\n", f, c, &c, *f[1], (*f)[1], c[1]);

	char *g = (char*)"123";
	char **h = &g;
	printf("&g=%p, g=%p\n", &g, g);
	ppoint2(h);

	return 0;
}
