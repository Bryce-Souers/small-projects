all:
	make address_translation
	make virtual_memory

run:
	make address_translation
	make virtual_memory
	./address_translation inputs/part1sequence outputs/part1-output
	./virtual_memory inputs/part2sequence outputs/part2-output

address_translation: address_translation.c
	gcc address_translation.c -o address_translation

virtual_memory: virtual_memory.c
	gcc virtual_memory.c -o virtual_memory