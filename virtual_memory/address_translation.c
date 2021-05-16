/*
 * Bryce Souers
 * address_translation.c - Translate each logical address in the input
 * 						   file to a corresponding physical address
 *						   using a static page table
 * Usage: ./address_translation input_sequence_file output_file
 */

#include <stdio.h>
#include <stdlib.h>

#define PHYSICAL_MEM_SIZE 1024
#define VIRTUAL_MEM_SIZE  4096
#define BYTES_PER_PAGE    128

int main(int argc, char* argv[]) {
	// Check for proper argument usage
	if(argc < 3) {
		fprintf(stderr, "[ERROR] Usage: ./part1 infile outfile\n");
		exit(1);
	}
	// Static pagetable
	int PT[8] = {2, 4, 1, 7, 3, 5, 6, -1};
	// Open infile for reading in binary mode
	FILE* infile;
	infile = fopen(argv[1], "rb");
	if(infile == NULL) {
		fprintf(stderr, "[ERROR] Unable to open infile.\n");
		exit(1);
	}
	// Open outfile for writing in binary mode
	FILE* outfile;
	outfile = fopen(argv[2], "wb");
	if(outfile == NULL) {
		fprintf(stderr, "[ERROR] Unable to open outfile.\n");
		exit(1);
	}
	unsigned long address;
	// Loop over each address in the infile
	while(fread(&address, sizeof(unsigned long), 1, infile) == 1) {
		// Get page number and offset
		int page_num = address / BYTES_PER_PAGE;
		int page_offset = address % BYTES_PER_PAGE;
		// Get frame number according to page table
		int frame_num = PT[page_num];
		// Calculate physical address from the virtual address
		unsigned long PA = frame_num * BYTES_PER_PAGE + page_offset;
		// Write the calculated physical address to the outfile
		if(fwrite(&PA, sizeof(PA), 1, outfile) != 1) {
			fprintf(stderr, "[ERROR] Unable to write to outfile.\n");
			exit(1);
		}
		// Print out for debugging
		printf("The LA is %-3lx and Translated PA is %lx\n", address, PA);
	}
	// Print out total pages for debugging
	printf("total number of pages = %d\n", PHYSICAL_MEM_SIZE / BYTES_PER_PAGE);
	// Clean up :)
	fclose(infile);
	fclose(outfile);
	return 0;
}
