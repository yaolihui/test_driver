test:
a=1
test2:
a=2
test:
b=1
test:
	echo a=$a b=$b
	gcc sizeof.c -w && ./a.out
	gcc method.c -w && ./a.out
	g++ main.cc -w && ./a.out
	gcc struct_array.c -w && ./a.out
	gcc struct2string.c -w && ./a.out
	gcc struct2struct.c -w && ./a.out

#rm -rf a.out
	
all: test
	echo all
