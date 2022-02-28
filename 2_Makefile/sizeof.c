#include <stdio.h>

struct A {			// 16 Byte
	short a;		// 2 Byte
	int b;			// 4 Byte
	long c;			// 8 Byte
};
struct B {			// 16 Byte
	short a;
	struct A* A;	// 8 Byte
};
struct C {			// 24 Byte
	int a;			// 4 Byte
	struct A b;		// 16 Byte
};
struct D {			// 24 Byte
	short c;		// 2 Byte
	int a;			// 4 Byte
	struct A b;		// 16 Byte
};
struct E {			// 32 Byte
	int a;			// 4 Byte
	struct A b;		// 16 Byte
	short c;		// 2 Byte
};
struct F {			// 16 Byte
	struct Z{		// 0 Byte
		short c;
		int a;
	};
	struct A b;		// 16 Byte
};
struct G {			// 0 Byte
	struct Y {		// 0 Byte
		struct X {	// 0 Byte
			int a;
		};
	};
};
struct H {			// 0 Byte
	struct U {		// 0 Byte
	} u_t;
};

int main(void)
{
	int* a;
	printf("sizeof(a)=%ld\n", sizeof(a));
	
	printf("sizeof(short)=%ld\n", sizeof(short));
	printf("sizeof(int)=%ld\n", sizeof(int));
	printf("sizeof(float)=%ld\n", sizeof(float));
	printf("sizeof(long)=%ld\n", sizeof(long));
	printf("sizeof(long long)=%ld\n", sizeof(long long));
	
	printf("sizeof(struct A)=%ld\n", sizeof(struct A));
	printf("sizeof(struct B)=%ld\n", sizeof(struct B));
	printf("sizeof(struct C)=%ld\n", sizeof(struct C));
	printf("sizeof(struct D)=%ld\n", sizeof(struct D));
	printf("sizeof(struct E)=%ld\n", sizeof(struct E));
	printf("sizeof(struct F)=%ld\n", sizeof(struct F));
	printf("sizeof(struct G)=%ld\n", sizeof(struct G));
	printf("sizeof(struct H)=%ld\n", sizeof(struct H));

	return 0;
}
