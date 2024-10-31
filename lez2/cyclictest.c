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

// non è ciclico perché sleep e sveglia dipendono dalla frequenza della cpu
// e dallo stato in cui viene lasciata la cpu quando sospende il processo
// se sto eseguendo altro con gli spinlock a intr disabilitate il timer non lo vedo

// c'è tempo per decidere quale cpu assegnare al processo
// c'è tempo per decidere quale processo far partire o task a più alte priorità
// possibile interrupt
// overhead per ogni cambio contesto
// se prima della sleep c'è un interrupt, il processo non si addormenta in modo ciclico

/*
=> uso clock_gettime per misurare il tempo
#include <time.h>
int clock_gettime(clockid_t clockid, struct timespec *tp);
    clockid: 
        - CLOCK_REALTIME (clock di sistema, può essere continuamente 
            aggiustato dall'utente o dal software perché i clock si sfasano continuamente)
        - CLOCK_MONOTONIC (non fa salti discontinui, non può essere modificato)
        - CLOCK_MONOTONIC_RAW (solo hardware)
*/ 
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
        //clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &end, NULL);
        long left;
        do
        {
            struct timespec now;
            clock_gettime(CLOCK_MONOTONIC, &now);
            left = (end.tv_sec - now.tv_sec) * 1000000000 + (end.tv_nsec - now.tv_nsec);
        } while (left > 0);
    }
}

// la latency varia perché se disabilito le interruuzioni, il timer non scatta
// preempt RT: il kernel è in grado di garantire che il processo si svegli esattamente al tempo stabilito
// usa spinlock per evitare che il processo venga spostato su un'altra cpu
// demanda la gestione delle interruzioni delle periferiche a un kernel thread:
    // è schedulabile e diminuisce i periodi con interrupt-disabled

/*
// continua a esserci latenza non costante
// disabilito il turbo boost, che non controllo (2.6GHz è più stabile di 5GHz)
/sys/devices/system/cpu/intel_pstate
echo 1 | sudo tee no_turbo
- non c'è un modo di controllare che il turbo boost sia stabile anche solo su una CPU


// metto la scaling_min_freq = scaling_max_freq (frequenza costante)
/sys/devices/system/cpu/policy0/scaling_min_freq

// nice -20
sudo nice -n -20 ./cyclictest
    - tracker a priorità 19
    - kernel a priorità -20
// qualsiasi processo real-time ha priorità magggiore a prescindere
// metto il mio processo a max priorità così non viene bloccato da altri, con sched_fifo
sudo chrt -f 1 executable
*/
