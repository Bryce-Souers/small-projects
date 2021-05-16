all: wordcount

run:
	make
	./wordcount inputs/anthem.txt inputs/preamble.txt

clean:
	rm -f *.o wordcount

wordcount: wordcount.c
	gcc wordcount.c -o wordcount

