/*
 * Bryce Souers
 * process_scheduling.c - Emulate process scheduling algorithms and context switching
 * Usage: ./process_scheduling -alg [FIFO|SJF|PR|RR] -input input_file
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Basic PCB data structure
struct PCB_st {
	int ProcId;
	int ProcPR;
	int CPUburst;
	int myReg[8];
	int queueEnterClock, waitingTime;
	struct PCB_st *next;
};

// Forward declarations
void FIFO_Scheduling();
void SJF_Scheduling();
void PR_Scheduling();
void RR_Scheduling();

void list_print();
void list_remove_element(struct PCB_st* pcb);
struct PCB_st* pop_min_burst();
struct PCB_st* pop_max_priority();
struct PCB_st* pop_head();
int list_is_empty();
void list_append(struct PCB_st* pcb);

// CPU registers
int CPUreg[8] = {0};

// PCB linked list variables
struct PCB_st *Head = NULL;
struct PCB_st *Tail = NULL;

// Statistic data variables
int CLOCK = 0;
int Total_waiting_time = 0;
int Total_turnaround_time = 0;
int Total_job = 0;

// Argument variables (algorithm/input file)
char* alg = NULL;
char* input_file_name = NULL;
int quantum_value = -1;

int main(int argc, char* argv[]) {
	// Setup argument variables
	void* alg_func = NULL;
	int hit_alg_arg = 0;
	int hit_input_arg = 0;
	int hit_quantum_arg = 0;
	// Loop through all the arguments from the command line
	int i;
	for(i = 0; i < argc; i++) {
		// Easy access to the current argument
		char* temp_s = argv[i];
		// Handle errors to the -alg argument
		if(strcmp(temp_s, "-alg") == 0) {
			if(hit_alg_arg == 0) {
				hit_alg_arg = 1;
				continue;
			} else {
				fprintf(stderr, "ERROR >> Multiple -alg arguments found.\n");
				exit(-1);
			}
		}
		// If the previous argument is -alg, make sure that this argument is a valid algorithm specified
		if(hit_alg_arg == 1) {
			     if(strcmp(temp_s, "FIFO") == 0) alg = "FIFO";
			else if(strcmp(temp_s, "SJF")  == 0) alg = "SJF";
			else if(strcmp(temp_s, "PR")   == 0) alg = "PR";
			else if(strcmp(temp_s, "RR")   == 0) alg = "RR";
			else {
				fprintf(stderr, "ERROR >> Invalid -alg argument given.\n");
				exit(-1);
			}
		}
		hit_alg_arg = 0;

		// Handle errors to the -input argument
		if(strcmp(temp_s, "-input") == 0) {
			if(hit_input_arg == 0) {
				hit_input_arg = 1;
				continue;
			} else {
				fprintf(stderr, "ERROR >> Multiple -input arguments found.\n");
				exit(-1);
			}
		}
		// If previous argument was -input, set the input file variable to this argument
		if(hit_input_arg == 1) input_file_name = temp_s;
		hit_input_arg = 0;

		// Handle errors to the -quantum argument
		if(strcmp(temp_s, "-quantum") == 0) {
			if(hit_quantum_arg == 0) {
				hit_quantum_arg = 1;
				continue;
			} else {
				fprintf(stderr, "ERROR >> Multiple -quantum arguments found.\n");
				exit(-1);
			}
		}
		// If previous argument was -quantum, set the quantum_value variable to this argument
		if(hit_quantum_arg == 1)quantum_value = atoi(temp_s);
		hit_quantum_arg = 0;
	}
	// Check that the -alg argument was eventually set correctly
	if(alg == NULL) {
		fprintf(stderr, "ERROR >> Algorithm never given in argument list.\n");
		exit(-1);
	}
	// Check that the -input argument was eventually set correctly
	if(input_file_name == NULL) {
		fprintf(stderr, "ERROR >> Input file name never given in argument list.\n");
		exit(-1);
	}

	// Open the input file specified from the arguments list
	FILE* input_file = fopen(input_file_name, "r");
	// Make sure file opened without error
	if(input_file == NULL) {
		fprintf(stderr, "ERROR >> Could not open input file - most likely does not exist.\n");
		exit(-1);
	}
	// Setup input file loop variables
	int process_id = -1;
	int process_priority = -1;
	int cpu_burst_time_ms = -1;
	// Loop through each line of the input file and put data into correct variables
	while(fscanf(input_file, "%d %d %d",
				 &process_id, &process_priority, &cpu_burst_time_ms) != EOF) {
		// Allocate a new PCB and set default variables
		struct PCB_st* pcb = (struct PCB_st*) malloc(sizeof(struct PCB_st));
		pcb->ProcId = process_id;
		pcb->ProcPR = process_priority;
		pcb->CPUburst = cpu_burst_time_ms;
		int i;
		for(i = 0; i < 8; i++) pcb->myReg[i] = process_id;
		pcb->queueEnterClock = 0;
		pcb->waitingTime = 0;
		pcb->next = NULL;
		// Append the PCB to the linked list
		list_append(pcb);
	}
	// Close input file
	fclose(input_file);
	// Print out general information
	printf("Input File Name : %s\nCPU Scheduling Alg : %s\n", input_file_name, alg);
	// Call the correct function for each specified algorithm
	if(strcmp(alg, "FIFO") == 0) FIFO_Scheduling();
	if(strcmp(alg, "SJF") == 0) SJF_Scheduling();
	if(strcmp(alg, "PR") == 0) PR_Scheduling();
	if(strcmp(alg, "RR") == 0) RR_Scheduling();
}


/* ALGORITHM FUNCTIONS */

// Function to be performed for FIFO
void FIFO_Scheduling() {
	while(!list_is_empty()) {
		// Remove first process
		struct PCB_st* PCB = pop_head();
		// Perform some stuff on CPU
		int i;
		for(i = 0; i < 8; i++) CPUreg[i] = PCB->myReg[i];
		for(i = 0; i < 8; i++) CPUreg[i] += 1;
		for(i = 0; i < 8; i++) PCB->myReg[i] = CPUreg[i];
		// Update stats
		PCB->waitingTime = PCB->waitingTime + CLOCK - PCB->queueEnterClock;
		Total_waiting_time = Total_waiting_time + PCB->waitingTime;
		CLOCK = CLOCK + PCB->CPUburst;
		Total_turnaround_time = Total_turnaround_time + CLOCK;
		Total_job = Total_job + 1;
		printf("\nProcess %d completed at %d ms", PCB->ProcId, CLOCK);
		free(PCB);
	}
	// Print stats
	printf("\n\nAverage Waiting time =  %.2f ms      (%d/%d)", (float)Total_waiting_time / Total_job, Total_waiting_time, Total_job);
	printf("\nAverage Turnaround time =  %.2f ms  (%d/%d)", (float)Total_turnaround_time / Total_job, Total_turnaround_time, Total_job);
	printf("\nThroughput =  %.2f jobs per ms       (%d/%d)\n", (float)Total_job / CLOCK, Total_job, CLOCK);
}

// Function to be performed for SJF
void SJF_Scheduling() {
	while(!list_is_empty()) {
		// Remove process with smallest CPU burst
		struct PCB_st* PCB = pop_min_burst();
		// Perform some stuff on CPU
		int i;
		for(i = 0; i < 8; i++) CPUreg[i] = PCB->myReg[i];
		for(i = 0; i < 8; i++) CPUreg[i] += 1;
		for(i = 0; i < 8; i++) PCB->myReg[i] = CPUreg[i];
		// Update stats
		PCB->waitingTime = PCB->waitingTime + CLOCK - PCB->queueEnterClock;
		Total_waiting_time = Total_waiting_time + PCB->waitingTime;
		CLOCK = CLOCK + PCB->CPUburst;
		Total_turnaround_time = Total_turnaround_time + CLOCK;
		Total_job = Total_job + 1;
		printf("\nProcess %d completed at %d ms", PCB->ProcId, CLOCK);
		free(PCB);
	}
	// Print stats
	printf("\n\nAverage Waiting time =  %.2f ms      (%d/%d)", (float)Total_waiting_time / Total_job, Total_waiting_time, Total_job);
	printf("\nAverage Turnaround time =  %.2f ms  (%d/%d)", (float)Total_turnaround_time / Total_job, Total_turnaround_time, Total_job);
	printf("\nThroughput =  %.2f jobs per ms       (%d/%d)\n", (float)Total_job / CLOCK, Total_job, CLOCK);
}

// Function to be performed for PR
void PR_Scheduling() {
	while(!list_is_empty()) {
		// Remove process with maximum priority
		struct PCB_st* PCB = pop_max_priority();
		// Perform some stuff on CPU
		int i;
		for(i = 0; i < 8; i++) CPUreg[i] = PCB->myReg[i];
		for(i = 0; i < 8; i++) CPUreg[i] += 1;
		for(i = 0; i < 8; i++) PCB->myReg[i] = CPUreg[i];
		// Update stats
		PCB->waitingTime = PCB->waitingTime + CLOCK - PCB->queueEnterClock;
		Total_waiting_time = Total_waiting_time + PCB->waitingTime;
		CLOCK = CLOCK + PCB->CPUburst;
		Total_turnaround_time = Total_turnaround_time + CLOCK;
		Total_job = Total_job + 1;
		printf("\nProcess %d completed at %d ms", PCB->ProcId, CLOCK);
		free(PCB);
	}
	// Print stats
	printf("\n\nAverage Waiting time =  %.2f ms      (%d/%d)", (float)Total_waiting_time / Total_job, Total_waiting_time, Total_job);
	printf("\nAverage Turnaround time =  %.2f ms  (%d/%d)", (float)Total_turnaround_time / Total_job, Total_turnaround_time, Total_job);
	printf("\nThroughput =  %.2f jobs per ms       (%d/%d)\n", (float)Total_job / CLOCK, Total_job, CLOCK);
}

// Function to be performed for RR
void RR_Scheduling() {
	if(quantum_value == -1) {
		fprintf(stderr, "ERROR >> RR was requested but no -quantum argument was set.\n");
		exit(-1);
	}
	while(!list_is_empty()) {
		struct PCB_st* PCB = pop_head();
		printf("\nrunning: %d\n", PCB->ProcId);
		list_print();
		printf("\n");
		// Perform some stuff on CPU
		int i;
		for(i = 0; i < 8; i++) CPUreg[i] = PCB->myReg[i];
		for(i = 0; i < 8; i++) CPUreg[i] += 1;
		for(i = 0; i < 8; i++) PCB->myReg[i] = CPUreg[i];
		if(PCB->CPUburst <= quantum_value) {
			PCB->waitingTime = PCB->waitingTime + CLOCK - PCB->queueEnterClock;
			Total_waiting_time = Total_waiting_time + PCB->waitingTime;
			CLOCK = CLOCK + PCB->CPUburst;
			Total_turnaround_time = Total_turnaround_time + CLOCK;
			Total_job = Total_job + 1;
			printf("\nProcess %d completed at %d ms", PCB->ProcId, CLOCK);
			free(PCB);
		} else {
			PCB->waitingTime = PCB->waitingTime + CLOCK - PCB->queueEnterClock;
			CLOCK = CLOCK + quantum_value;
			PCB->CPUburst = PCB->CPUburst - quantum_value;
			PCB->queueEnterClock = CLOCK;
			list_append(PCB);
		}
	}
	// Print stats
	printf("\n\nAverage Waiting time =  %.2f ms      (%d/%d)", (float)Total_waiting_time / Total_job, Total_waiting_time, Total_job);
	printf("\nAverage Turnaround time =  %.2f ms  (%d/%d)", (float)Total_turnaround_time / Total_job, Total_turnaround_time, Total_job);
	printf("\nThroughput =  %.2f jobs per ms       (%d/%d)\n", (float)Total_job / CLOCK, Total_job, CLOCK);
}


/* LINKED LIST FUNCTIONS */

// Print out entire linked list (mostly for debug)
void list_print() {
	struct PCB_st* pcb = Head;
	while(pcb != NULL) {
		printf("procid: %d\n", pcb->ProcId);
		pcb = pcb->next;
	}
}

// Remove and return the PCB with the highest priority
struct PCB_st* pop_max_priority() {
	if(Head == NULL) return NULL;
	struct PCB_st* temp = Head;
	struct PCB_st* max_temp = NULL;
	int max_priority = temp->ProcPR;
	while(temp != NULL) {
		if(temp->ProcPR >= max_priority) {
			max_priority = temp->ProcPR;
			max_temp = temp;
		}
		temp = temp->next;
	}
	if(max_temp == NULL) return NULL;
	list_remove_element(max_temp);
	return max_temp;
}

// Remove and return the PCB with the shorted CPU burst
struct PCB_st* pop_min_burst() {
	if(Head == NULL) return NULL;
	struct PCB_st* temp = Head;
	struct PCB_st* min_temp = NULL;
	int min_cpu_burst = temp->CPUburst;
	while(temp != NULL) {
		if(temp->CPUburst <= min_cpu_burst) {
			min_cpu_burst = temp->CPUburst;
			min_temp = temp;
		}
		temp = temp->next;
	}
	if(min_temp == NULL) return NULL;
	list_remove_element(min_temp);
	return min_temp;
}

// Remove a matched PCB from the linked list
void list_remove_element(struct PCB_st* pcb) {
	struct PCB_st* temp = Head;
	struct PCB_st* prev = NULL;
	while(temp != NULL) {
		if(pcb == temp) {
			if(prev == NULL) {
				Head = temp->next;
			} else {
				prev->next = temp->next;
			}
			temp->next = NULL;
			return;
		}
		prev = temp;
		temp = temp->next;
	}
}

// Remove head from linked list and return old head
struct PCB_st* pop_head() {
	if(Head == NULL) return NULL;
	struct PCB_st* temp = Head;
	Head = Head->next;
	return temp;
}

// Check if linked list is empty
int list_is_empty() {
	if(Head == NULL) return 1;
	return 0;
}

// Add new PCB to the beginning of the linked list
void list_append(struct PCB_st* pcb) {
	struct PCB_st* last = Head;
	if(Head == NULL) {
		Head = pcb;
		return;
	}
	while(last->next != NULL) last = last->next;
	last->next = pcb;
}
