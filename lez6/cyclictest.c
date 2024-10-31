#define _GNU_SOURCE // gcc -D_GNU_SOURCE per essere sicuri che sia incluso (per progetti complessi)
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/mman.h>
#include <linux/types.h>
#include <sys/syscall.h>

struct sched_attr {
    __u32 size;              /* Size of this structure */
    __u32 sched_policy;      /* Policy (SCHED_*) */
    __u64 sched_flags;       /* Flags */
    __s32 sched_nice;        /* Nice value (SCHED_OTHER,
                                SCHED_BATCH) */
    __u32 sched_priority;    /* Static priority (SCHED_FIFO,
                                SCHED_RR) */
    /* Remaining fields are for SCHED_DEADLINE */
    __u64 sched_runtime;
    __u64 sched_deadline;
    __u64 sched_period;
};


const SLEEP_TIME = 100000000; // 500ms
main() {
    if (mlockall(MCL_FUTURE) != 0) {
        perror("mlock failed: ");
        exit(1);
    }
    struct sched_attr sa = {
        .size           = sizeof(sa),
        .sched_policy   = SCHED_DEADLINE,
        //.sched_priority = 1,
        .sched_runtime  = 10*1000*1000,
        .sched_deadline = 20*1000*1000,
        .sched_period   = 20*1000*1000
    };

    if (syscall(SYS_sched_setattr, gettid(), &sa, 0)) {
        perror("setattr failed: ");
        exit(1);
    }
    // measure latency time of wakeup
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    for(;;) {
        clock_gettime(CLOCK_MONOTONIC, &end);
        long elapsed = (end.tv_sec - start.tv_sec) * 1000000000 + (end.tv_nsec - start.tv_nsec);
        printf("Latency: %ld ns\n", (elapsed - SLEEP_TIME)/1000);
        start = end;
        // end = end + 1s
        end.tv_nsec += SLEEP_TIME;
        if (end.tv_nsec > 1000000000) {
            end.tv_sec++;
            end.tv_nsec -= 1000000000;
        }

        // calcoli
        long acc = 0;
        for (long i = 0; i < 1000; i++) {
            for (long j = 0; j < 1000; j++) {
                acc += i*j;
            }
        }

        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &end, NULL);
    }
}
