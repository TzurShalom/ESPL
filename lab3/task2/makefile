all: task2

task2 :util.o start.o main.o
	ld -m elf_i386 util.o start.o main.o -o task2

util.o : util.c
	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector util.c -o util.o

start.o : start.s
	nasm -g -f elf32 start.s -o start.o

main.o : main.c
	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector main.c -o main.o

.PHONY : clean

clean:
	rm -f *.o task2