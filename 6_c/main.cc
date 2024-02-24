#include <stdio.h>

struct {
	int add() {
		return 1 + 2;
	};
	int a = add();
}s;

int main() 
{
	
	if(int a=1){
		printf("%d\n", s.add());
		printf("%d\n", s.a);
	}
	return 0;
};
