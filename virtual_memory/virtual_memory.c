/*
 * Bryce Souers
 * virtual_memory.c - Dynamically manage a page table and perform logical
 * 					  to physical memory translations along with a page
 *					  replacement algorithm
 * Usage: ./virtual_memory input_sequence_file output_file
 */

#include <stdio.h>
#include <stdlib.h>

#define PHYSICAL_MEM_SIZE 1024
#define VIRTUAL_MEM_SIZE  4096
#define BYTES_PER_PAGE    128

// Structure for each page table entry
typedef struct PTE {
	int valid;
	int frame_number;
} PTE;

// Establish global variables to use
PTE page_table[32];
int clock = 0;
int free_frames[8] = {0,1,1,1,1,1,1,1};
int lru_count[8] =   {0,0,0,0,0,0,0,0};
int reverse_map[8] = {-1,-1,-1,-1,-1,-1,-1,-1};
unsigned long LA;

// Function forward declarations
int find_empty_frame();
int find_lru();

int main(int argc, char* argv[]) {
	int i;
	// Initialize each PTE's valid bit to 0
	for(i = 0; i < 32; i++) {
		page_table[i].valid = 0;
	}
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
	// Track number of page faults
	int page_faults = 0;
	// Loop over each address in the infile
	while(fread(&LA, sizeof(unsigned long), 1, infile) == 1) {
		// Get page number and offset from the virtual address
		int page_num = LA / BYTES_PER_PAGE;
		int page_offset = LA % BYTES_PER_PAGE;
		int frame_number;
		unsigned long PA;
		// Case for when there is a valid entry for the page number
		if(page_table[page_num].valid == 1) {
			frame_number = page_table[page_num].frame_number;
			PA = frame_number * BYTES_PER_PAGE + page_offset;
			if(fwrite(&PA, sizeof(PA), 1, outfile) != 1) {
				fprintf(stderr, "[ERROR] Unable to write to outfile.\n");
				exit(1);
			}
			lru_count[frame_number] = clock;
			free_frames[frame_number] = 0;
		// Case for when there is not a valid entry for the page number
		} else {
			// Find an available frame
			int x = find_empty_frame();
			page_faults++;
			// Case for when there is a frame available
			if(x > 0) {
				// Set PTE entry values
				page_table[page_num].frame_number = x;
				page_table[page_num].valid = 1;
				frame_number = x;
				// Calculate physical address
				PA = frame_number * BYTES_PER_PAGE + page_offset;
				// Write to outfile
				if(fwrite(&PA, sizeof(PA), 1, outfile) != 1) {
					fprintf(stderr, "[ERROR] Unable to write to outfile.\n");
					exit(1);
				}
				// Update reverse map
				reverse_map[x] = page_num;
				// Update LRU count array
				lru_count[frame_number] = clock;
				// Update free frames to show not available
				free_frames[frame_number] = 0;
			// Case for when there is not a frame available
			} else {
				// Get the LRU frame
				int y = find_lru();
				// Release the old one using reverse map
				page_table[reverse_map[y]].valid = 0;
				// Set PTE entry values
				page_table[page_num].frame_number = y;
				page_table[page_num].valid = 1;
				frame_number = page_table[page_num].frame_number;
				// Calculate physical address
				PA = frame_number * BYTES_PER_PAGE + page_offset;
				// Write to outfile
				if(fwrite(&PA, sizeof(PA), 1, outfile) != 1) {
					fprintf(stderr, "[ERROR] Unable to write to outfile.\n");
					exit(1);
				}
				// Update LRU count array
				lru_count[frame_number] = clock;
				// Update reverse map
				reverse_map[frame_number] = page_num;
				// Update free frames to show not available
				free_frames[frame_number] = 0;
			}
		}
		// Increment clock
		clock++;
	}
	// Clean up :)
	fclose(infile);
	fclose(outfile);
	// Print out for debugging
	printf("Part 2 page faults: %d\n", page_faults);
	return 0;
}

// Finds the LRU frame
int find_lru() {
	int i;
	int min = -1;
	int min_i = -1;
	for(i = 1; i < 8; i++) {
		if(min == -1 || lru_count[i] < min) {
			min = lru_count[i];
			min_i = i;
		}
	}
	return min_i;
}

// Finds an empty frame or return -1
int find_empty_frame() {
	int i;
	for(i = 1; i < 8; i++) if(free_frames[i] == 1) return i;
	return -1;
}
