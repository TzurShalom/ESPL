all: myshell mypipeline 

myshell: myshell.o 
	gcc -m32 -g -Wall -o myshell myshell.o 

myshell.o: myshell.c
	gcc -g -Wall -m32 -c -o myshell.o myshell.c

mypipeline: mypipeline.o
	gcc -m32 -g -Wall -o mypipeline mypipeline.o

mypipeline.o: mypipeline.c
	gcc -g -Wall -m32 -c -o mypipeline.o mypipeline.c

.PHONY: clean 

clean:
	rm -f *.o lab
