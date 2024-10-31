# Lez. 1 - 17/10/2024

## intro
RT e HPC hanno in comune la predicibilità delle tempistiche.

- Task paralleli 1: divido i job e vado in parallelo e poi unisco i risultati
- Task paralleli 2: i nodi comunicano tra di loro
  - es: fluidodinamica, simulazioni cosmiche
    - si taglia il volume in fette e simulo sullo stesso core i vari volumetti
    - ad ogni deltaT, le particelle si spostano tra un volume e l'altro
    - ogni risultato è influenzato da quelli precedenti (tipo rete neurale) => se anche solo uno ritarda, ritarda tutto il calcolo => bisogno di predicibilità

## Come dialogare con l'OS?

### syscall
- open/close
- read/write
- ioctl: controllo del flusso della periferica (es. configurare porte seriale)
cd /usr/include && grep -r SYS
poi ricompilo e faccio il reboot

### virtual file per comunicare con il kernel
Una volta c'era l'OS monolitico (un solo kernel space per dialogare con l'hardware). Però ci sono tantissimi driver, che cambiano continuamente. Con il kernel monolitico servirebbe inserire tutti i driver compilati insieme e sarebbe ingestibile. Inoltre basta un bug e si schianta tutta la macchina.

Esistono i driver caricati dinamicamente => monolite più piccolo (cpu, ram, scheduler memoria, net, proc... + driver caricati nel monolite) + driver linkati dinamicamente.
Se il driver ha un bug, comunque si schianta tutto.

Microkernel: kernel minimale a metà tra hardware e kernel + driver in user space. Se il driver va in crash, è come un processo cheviene isolato e terminato ma non si schianta tutto il sistema.

Su Linux si possono caricare i moduli dinamicamente.

#### procfs
/proc/784
o uso la syscall read, oppure uso la libproc (usata da top)
- cat /proc/$$/sched -> descrittore del processo riferito allo scheduler
    - $$ = pid processo corrente
- /proc/$$/task -> thread

#### sysfs
/sys/devices/system/cpu
- cpu sul sistema, online, offline, isolated, ecc.
  - multi-NUMA (non-uniform memory access) => ogni cpu ha la sua memoria

##### DVFS (dynamic voltage and frequency scaler)
/sys/devices/system/cpu/cpu0/cpufreq 
- grep "" * => stampa tutti i file in una directory
    affected_cpus:0
    base_frequency:2800000
    cpuinfo_max_freq:4700000
    cpuinfo_min_freq:400000
    cpuinfo_transition_latency:0
    energy_performance_available_preferences:default performance balance_performance balance_power power 
    energy_performance_preference:balance_performance
    related_cpus:0
    scaling_available_governors:performance powersave
    scaling_cur_freq:400218
    scaling_driver:intel_pstate
    scaling_governor:powersave
    scaling_max_freq:4700000
    scaling_min_freq:400000
    scaling_setspeed:<unsupported>
        

Negli ultimi anni due tipi di core:
- big: core potenti, consumano di più, ottimizzazione delle istruzioni
- LITTLE: core poco potenti e più lenti

Il governor è un algoritmo che decide la frequenza del core. Nel des_proc c'è la stima di carico del processo: se è alto, il governor aumenta la frequenza, se è basso la abbassa.
- echo performance | sudo tee scaling_governor

## DIP (deep idle state)
lez2/appunti.md

## Boosting
Disabilito il turbo boost, che non controllo (2.6GHz è più stabile di 5GHz)
/sys/devices/system/cpu/intel_pstate
echo 1 | sudo tee no_turbo
- non c'è un modo di controllare che il turbo boost sia stabile anche solo su una CPU


## cyclictest
sleep() non è ciclico perché sleep e sveglia dipendono 
- dalla frequenza della cpu
- dallo stato in cui viene lasciata la cpu quando sospende il processo
- se sto eseguendo altro con gli spinlock a intr disabilitate il timer non lo vedo
- c'è tempo per decidere quale cpu assegnare al processo
- c'è tempo per decidere quale processo far partire o task a più alte priorità
- possibile interrupt
- overhead per ogni cambio contesto
- se prima della sleep c'è un interrupt, il processo non si addormenta in modo ciclico


### clock_gettime
#include <time.h>
int clock_gettime(clockid_t clockid, struct timespec *tp);
    clockid: 
        - CLOCK_REALTIME (clock di sistema, può essere continuamente 
            aggiustato dall'utente o dal software perché i clock si sfasano continuamente)
        - CLOCK_MONOTONIC (non fa salti discontinui e non va all'indietro)
        - CLOCK_MONOTONIC_RAW (solo hardware)

## preempRT (da rivedere)
La latency varia perché se disabilito le interruzioni, il timer non scatta
preempt RT
- il kernel è in grado di garantire che il processo si svegli esattamente al tempo stabilito 
- usa spinlock per evitare che il processo venga spostato su un'altra cpu
- demanda la gestione delle interruzioni delle periferiche a un kernel thread:
  - è schedulabile e diminuisce i periodi con interrupt-disabled

## Frequenza costante
metto la scaling_min_freq = scaling_max_freq (frequenza costante)
/sys/devices/system/cpu/policy*/scaling_min_freq = /sys/devices/system/cpu/policy*/scaling_max_freq

## Priorità
nice -20
sudo nice -n -20 ./cyclictest
- tracker a priorità 19
- kernel a priorità -20
qualsiasi processo real-time ha priorità magggiore a prescindere
metto il mio processo a max priorità così non viene bloccato da altri, con sched_fifo
- sudo chrt -f 99 executable
