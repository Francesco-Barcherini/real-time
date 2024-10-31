// task periodico
// periodo predefinito
// sveglia al tempo t, misura il tempo che è passato da t
// lo stampa e si addormenta

#include <stdio.h>
#include <unistd.h>
#include <time.h>

main_wrong() {
    for(;;) {
        sleep(1);
        int t = time(0);
        printf("Tempo: %d\n", t);
    }
}


const SLEEP_TIME = 100000000; // 500ms
main() {
    // measure latency time of wakeup
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    for(;;) {
        clock_gettime(CLOCK_MONOTONIC, &end);
        long elapsed = (end.tv_sec - start.tv_sec) * 1000000000 + (end.tv_nsec - start.tv_nsec);
        printf("Latency: %ld ns\n", (elapsed - SLEEP_TIME)/1);
        start = end;
        // end = end + 1s
        end.tv_nsec += SLEEP_TIME;
        if (end.tv_nsec > 1000000000) {
            end.tv_sec++;
            end.tv_nsec -= 1000000000;
        }
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &end, NULL);
    }
}
