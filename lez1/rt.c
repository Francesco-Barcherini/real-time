/*
 * rt.c
 *
 * This program tests the latency charateristics of a Linux system by doing some
 * deterministic calculations, measuring the time it takes to do so, goes to sleep for
 * 10 ms and then repeats the test for 100 times. It then calculates and prints a report
 * with average, minimum, maximum, standard deviation and dev/avg for the times measured.
 * It repeats the test indefinitely.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <limits.h>

#define LOOPS 100 // Number of loops for each test
#define SLEEP_TIME 10000000 // Time to sleep in nanoseconds (10ms)
#define N 100000

int arr[N];

void calculations() {
	long a = 0;
	for (int j = 0; j < 2; j++) {
		for (int i = 0; i < N; i++) {
			arr[i] = i;
			a += i * i;
		}
	}
}

int main(int argc, char *argv[]) {
	unsigned int i = 0;
	unsigned long sum_us = 0, sum2_us = 0, min_us = UINT_MAX, max_us = 0;

	struct timespec next, end;

	while (1) {
		clock_gettime(CLOCK_MONOTONIC, &next);
		for (; i < LOOPS; i++) {
			// Do some deterministic calculations
			calculations();
			clock_gettime(CLOCK_MONOTONIC, &end);
	
			// beware of overflow
			long elapsed_us = (end.tv_sec - next.tv_sec) * 1000000 + (end.tv_nsec - next.tv_nsec) / 1000;

			if (elapsed_us < min_us) min_us = elapsed_us;
			if (elapsed_us > max_us) max_us = elapsed_us;
			sum_us += elapsed_us;
			sum2_us += elapsed_us * elapsed_us;
	
			// handle overflow
			next.tv_nsec += SLEEP_TIME;
			if (next.tv_nsec > 1000000000) {
				next.tv_sec++;
				next.tv_nsec -= 1000000000;
			}
			clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next, NULL);
		}

		unsigned long avg = sum_us / LOOPS;
		double dev = sqrt((sum2_us - sum_us * sum_us / LOOPS) / LOOPS);
		double devavg = dev / avg;

		// print results in microseconds
		printf("avg: %3lu us, std: %7.3f us, min: %3lu us, max: %4lu us, std/avg: %f, n: %d\n", avg, dev, min_us, max_us, devavg, LOOPS);

		// reset variables
		i = 0;
		sum_us = 0;
		sum2_us = 0;
		min_us = UINT_MAX;
		max_us = 0;
	}

	return 0;
}
