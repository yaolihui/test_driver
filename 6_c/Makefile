test:
a=1
test2:
a=2
test:
b=1
test:
	echo a=$a b=$b
	gcc sizeof.c && ./a.out
	gcc method.c  && ./a.out
	g++ main.cc   && ./a.out

#rm -rf a.out
	
all: test
	echo all
