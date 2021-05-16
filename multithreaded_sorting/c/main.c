/*
 * Bryce Souers
 * main.c - Multithreaded sorting program
 * Usage: ./prog array_size
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>

typedef struct sort_pkg {
	double* a;
	int n;
} sort_pkg;

typedef struct merge_pkg {
	double* a;
	double* a_1;
	double* a_2;
	int n;
} merge_pkg;

void sel_sort(double* a, int n) {
	int i = 0, j = 0, min = 0;
	for(i = 0; i < (n - 1); i++) {
		min = i;
		for(j = (i + 1); j < n; j++) if(a[j] < a[min]) min = j;
		double t = a[min];
		a[min] = a[i];
		a[i] = t;
	}
}

void* sort(void* arg) {
	sort_pkg* tp = (sort_pkg* ) arg;
	sel_sort(tp->a, tp->n);
	return NULL;
}

void* merge(void* arg) {
	merge_pkg* mp = (merge_pkg* ) arg;
	int i = 0, j = 0, k = 0;
	while(i < (mp->n / 2) && j < (mp->n / 2)) {
		if(mp->a_1[i] < mp->a_2[j]) mp->a[k++] = mp->a_1[i++];
		else mp->a[k++] = mp->a_2[j++];
	}
	while(i < (mp->n / 2)) mp->a[k++] = mp->a_1[i++];
	while(j < (mp->n / 2)) mp->a[k++] = mp->a_2[j++];
	return NULL;
}

int main(int argc, char* argv[]) {
	if(argc < 2) {
		fprintf(stderr, "[ERROR] Missing argument number.\n");
		exit(EXIT_FAILURE);
	}
	int n = atoi(argv[1]);
	struct timespec ts_begin, ts_end;
	double elapsed;

	srand(time(NULL));

	double* a = (double* ) malloc(sizeof(double) * n);
	if(a == NULL) {
		fprintf(stderr, "[ERROR] Cannot allocate double array.\n");
		exit(EXIT_FAILURE);
	}

	int i;
	for(i = 0; i < n; i++) a[i] = 1.0f + (rand() / (RAND_MAX / (1000.0f - 1.0f)));

	//-------------------- ONE THREAD CASE --------------------
	double* b = (double* ) malloc(sizeof(double) * n);
	for(i = 0; i < n; i++) b[i] = a[i];
	clock_gettime(CLOCK_MONOTONIC, &ts_begin);
	
	pthread_t thB;
	sort_pkg* tpb = (sort_pkg* ) malloc(sizeof(sort_pkg));
	tpb->a = b;
	tpb->n = n;
	pthread_create(&thB, NULL, sort, (void*) tpb);
	pthread_join(thB, NULL);

	clock_gettime(CLOCK_MONOTONIC, &ts_end);
	elapsed = ts_end.tv_sec - ts_begin.tv_sec;
	elapsed += (ts_end.tv_nsec - ts_begin.tv_nsec) / 1000000000.0;

	printf("Sorting by ONE thread is done in %.1lfms.\n", elapsed * 1000);


	//-------------------- TWO THREAD CASE --------------------
	double* a_1 = (double* ) malloc(sizeof(double) * (n / 2));
	if(a_1 == NULL) {
		fprintf(stderr, "[ERROR] Cannot allocate a_1 array.\n");
		exit(EXIT_FAILURE);
	}
	double* a_2 = (double* ) malloc(sizeof(double) * (n / 2));
	if(a_2 == NULL) {
		fprintf(stderr, "[ERROR] Cannot allocate a_2 array.\n");
		exit(EXIT_FAILURE);
	}
	for(i = 0; i < (n / 2); i++) {
		a_1[i] = a[i];
		a_2[i] = a[i + (n / 2)];
	}
	clock_gettime(CLOCK_MONOTONIC, &ts_begin);
	
	pthread_t thA1;
	sort_pkg* tp1 = (sort_pkg* ) malloc(sizeof(sort_pkg));
	tp1->a = a_1;
	tp1->n = n / 2;
	pthread_create(&thA1, NULL, sort, (void* ) tp1);

	pthread_t thA2;
	sort_pkg* tp2 = (sort_pkg* ) malloc(sizeof(sort_pkg));
	tp2->a = a_2;
	tp2->n = n / 2;
	pthread_create(&thA2, NULL, sort, (void* ) tp2);

	pthread_join(thA1, NULL);
	pthread_join(thA2, NULL);

	pthread_t thM;
	merge_pkg* mp = (merge_pkg* ) malloc(sizeof(merge_pkg));
	mp->a = a;
	mp->a_1 = a_1;
	mp->a_2 = a_2;
	mp->n = n;
	pthread_create(&thM, NULL, merge, (void* ) mp);
	pthread_join(thM, NULL);

	clock_gettime(CLOCK_MONOTONIC, &ts_end);
	elapsed = ts_end.tv_sec - ts_begin.tv_sec; 
	elapsed += (ts_end.tv_nsec - ts_begin.tv_nsec) / 1000000000.0;

	printf("Sorting by TWO threads is done in %.1lfms.\n", elapsed * 1000);

	free(a);
	free(a_1);
	free(a_2);
}
