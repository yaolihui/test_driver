#include <stdio.h>
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

typedef struct A {
	int i;
	char* s;
} A_t;


typedef struct B {
	int i;
	struct A *as;
} B_t;

void main(void)
{
	printf("\nHello world!\n");
	struct A a = {0, "aaa"};
	printf("a.i=%d, a.s=%s\n\n", a.i, a.s);


	A_t aa[] = {
		{1, "bbb"},
		{2, "ccc"},
		{3, "ddd"},
	};
	printf("aa0.i=%d, aa0.s=%s\n", aa[0].i, aa[0].s);

	A_t* aa1_p = &aa[1];
	printf("aa1.i=%d, aa1.s=%s\n",aa1_p->i, aa1_p->s);
	
	A_t* aa2_p = ++aa1_p;
	printf("aa2.i=%d, aa2.s=%s\n\n", aa2_p->i, aa2_p->s);


	B_t b[]= {
		{
			.i = 0,
			.as = (A_t []){
				{.i = 123, .s = "b123"},
				{.i = 234, .s = "b234"},
			}
		},
		{
			.i = 1,
			.as = (struct A []){
				{1123, "b1123"},
				{1234, "b1234"},
			}
		},
	};
	printf("b0.i=%d, b0.as.i=%d, b0.as.s=%s\n", b[0].i,  b[0].as[0].i, b[0].as[0].s);
	printf("b0.i=%d, b0.as.i=%d, b0.as.s=%s\n", b->i,  b->as[0].i, b->as[0].s);
	B_t* bp = b + 1;
	printf("b=%p, b[0]=%p, b[1]=%p, bp=%p, &bp[0]=%p\n",b, &b[0], &b[1], bp, &bp[0]);
	printf("bp->i=%d, bp->as.i=%d, bp->as.s=%s\n", bp->i, bp->as[1].i, bp->as[1].s);
	printf("bp[0].i=%d, bp[0].as.i=%d, bp[0].as.s=%s\n", bp[0].i, bp[0].as[1].i, bp[0].as[1].s);

};