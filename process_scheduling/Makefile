all: prog

process_scheduling: process_scheduling.c 
	gcc -o process_scheduling process_scheduling.c -std=c90

clean:
	rm -f process_scheduling *.o

run:
	make process_scheduling
	make run1

run1:
	./process_scheduling -alg FIFO -input inputs/input1.txt
run2:
	./process_scheduling -alg SJF -input inputs/input1.txt
run3:
	./process_scheduling -alg PR -input inputs/input1.txt
run4:
	./process_scheduling -alg RR -quantum 5 -input inputs/input1.txt