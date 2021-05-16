/*
 * Bryce Souers
 * wordcount.c - Use multiple processes to count words in files.
 * Usage: ./wordcount input_file1 input_file2 ...
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

char *read_line();
int get_word_count(char *file_name);

int main(int argc, char *argv[]) {
	// Loop that creates a new child process for each given argument in argv.
	for(int i = 1; i < argc; i++) {
		if(fork() == 0) {
			/*
			 *	Get word count of the file in the child process and deal with errors.
			 *	Each child process exits with a code of success (0) or failure (1).
			 */
			int wc = get_word_count(argv[i]);
			if(wc != -1) {
				printf("Child process %d for %s: number of words is %d\n", getpid(), argv[i], wc);
				exit(0);
			}
			printf("Child process %d for %s: does not exist\n", getpid(), argv[i]);
			exit(1);
		}
	}
	// Local variables for parent process to keep track of children's success
	int status = 0;
	pid_t child_pid;
	int num_proccesses = 0;
	int num_successful = 0;
	int num_failed = 0;
	// Parent waits for each child process
	while((child_pid = wait(&status)) > 0) {
		// Increment local variables accordingly by checking the exit status 
		num_proccesses++;
		if(status == 0) num_successful++;
		else num_failed++;
	}
	// Print out the results from the parent process
	printf("Parent proccess created %d child processes to count words in %d files\n", num_proccesses, (argc - 1));
	printf("\t%d files have been counted successfully!\n", num_successful);
	printf("\t%d files did not exist!\n", num_failed);
	return 0;
}

/*
 *	This function takes in a file name then tries to open it and count the words in the file.
 *
 *	Parameters:
 *		char* file_name -- file name to try to open
 *	Return:
 *		If successful: int with number of words in file
 *		If failed: -1
 */
int get_word_count(char *file_name) {
	int wc = 0;
	FILE *f = fopen(file_name, "r");
	if(f != NULL) {
		char *line;
		int status = 0;
		while(status == 0) {
			line = read_line(f, &status);
			char *c;
			int hit_word = 0;
			for(c = line; *c != '\0'; c++) {
				if((*c == ' ' || *c == '\t' || *c == '\n' || *c == '\r')) {
					if(hit_word == 1) {
						wc++;
						hit_word = 0;
					}
				} else hit_word = 1;
			}
			if(hit_word) wc++;
			free(line);
		}
	} else {
		return -1;
	}
	fclose(f);
	return wc;
}

/*
 *	This function takes in a file pointer and returns one valid, null-terminated line.
 *	The status integer is set to tell the caller that the file stream has reached its end.
 *
 *	Parameters:
 *		FILE* f -- file pointer to read from
 *		int* status -- int pointer that tells caller the status of read
 *	Return:
 *		If successful: null-terminated char*
 *		If failed: NULL
 */
char *read_line(FILE* f, int* status) {
	// Set base minimum memory size to allocate
	int mem_size = 10;
	int str_size = 0;
	char* in = (char*) malloc(mem_size);
	char c;
	// Loop through each character in the file until you reach end of line
	while((c = getc(f)) != '\n') {
		// Notify the caller that file pointer has been finished
		if(c == EOF) {
			*status = -1;
			break;
		}
		// Allocate more memory if needed
		str_size++;
		if(str_size + 1 > mem_size) {
			mem_size *= 2;
			in = (char*) realloc(in, mem_size);
		}
		if(in == NULL) {
			free(in);
			return NULL;
		}
		// Add character to return char*
		in[str_size - 1] = c;
	}
	// Make the return char* null-terminated
	in[str_size] = '\0';
	in = (char*) realloc(in, str_size + 1);
	if(in == NULL) {
		free(in);
		return NULL;
	}
	return in;
}
