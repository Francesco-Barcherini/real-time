# Lez. 5 - 29/10/2024
# Performance Counters

## SONO ARRIVATO TARDI, RECUPERARE LE SLIDE
## perf
`sudo perf top` -> top delle statistiche
- es. page fault, context-switch, instructions etc.
`sudo perf stat -e instructions,ref-cycles ls`
- statistiche sul numero di istruzioni e cicli nel programma
### front-end e back-end
`sudo perf list | grep front-end/back-end`
Front-end: fetch e decodifica delle istruzioni
Back-end: esecuzione

## branch predictor
### if (likely(cond))
Specifica al kernel di predire quel branch

### gcc -branchqualcosa
1) Compilo il codice
`gcc -fprofile-generate -o exe exe.c`
2) Eseguo il codice generando un log dei branch più usati
`./exe` -> *.gcda
3) Ricompilo il codice a cui gcc aggiunge likely e unlikely in base al profiling
`gcc -fprofile-use -o exe_opt exe.c`
gcc aggiunge le direttive `__builtin_expect`


# tracing (rt-tests)
`cat /sys/kernel/tracing/available_tracers`


## cyclictest
`sudo cyclictest -qmu -h 100 -p 95 -b 100 -t 8 --tracemark`
- q: Quiet mode
- m: mlockall
- u: high-resolution timers

- h 100: Histogram size. This sets the size of the histogram to 100. The histogram is used to record the distribution of latencies observed during the test.

- p 95: Priority

- b 100: Number of loops

- t 8: Number of threads. This runs 8 test threads.

- --tracemark: Add trace markers. This option adds markers to the tracing subsystem. These markers can be useful for correlating cyclictest events with other system events in the trace logs. This helps in diagnosing and understanding the causes of latency spikes.

## hwlatdetect
Rileva la latenza hardware, es. del BIOS o del firmware.
Disabilitano le IRQ e misurano la latenza proveniente dall'hw.

## timerlat
`sudo timerlat hist -a 100 -u --event sched:sched_wakeup`
- a: autoanalysis

`sudo rteval --onlyload --debug`
Stress testing con compilazione del kernel e pipe tra processori

## os-noise
Usato nei sistemi telco: traccia quando i task eseguiti sulle singole CPU si interrompono