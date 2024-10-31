#include <stdio.h>
#include <time.h>

int quanti = 0;

int main () {
    struct timespec now;

    for (int i = 0; i < 1000; i++) {
        clock_gettime(CLOCK_REALTIME, &now);
        if (now.tv_nsec > 100e6) {
            quanti++;
        }

        if (i % 50 == 0) {
            printf("Quanti: %d\n", quanti);
            printf("ns: %ld\n", now.tv_nsec);
        }

        now.tv_nsec += 400e4;
        if (now.tv_nsec >= 1e9) {
            now.tv_nsec -= 1e9;
            now.tv_sec++;
        }
        clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &now, NULL);
    }
}