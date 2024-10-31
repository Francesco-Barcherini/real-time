# Lez. 4 - 25/10/2024

## WCET
Per calcoli real-time serve il Worst-Case-Execution-Time.
- uso un tool di simulazione: Super-Scala
- non uso le chiamate di sistema
- priorità RT 99
- suppongo che la cache sia vuota
  - le interruzioni cambiano la cache
    - ogni tanto aggiungo un Preemption Point in cui getisco le interruzioni
    - ogni volta devo calcolare la computazione e riconsidero la cache vuota
    - Problema: se ho più CPU la cache è sporcata dagli altri processori
      - SCRATCH PAD MEMORY specifica del processore, i dati non vengono toccati

## Real-Time OS (Slides)

## CBS (https://lwn.net/Articles/398470)
Sospendo il task che supera il budget.
L'eccedenza la riattivo al prossimo periodo.

Come sappiamo che il tempo del kernel è allineato a quello della linea dei tempi?
### Virtual time
Parte dall'istante di schedulazione ed ha un rate che è 1/U rispetto al normale. Se il task è (C,T)=(2,10) avanza al quintuplo della velocità.
- Il virtual time arriva alla deadline quando il task dovrebbe esaurire il budget
- Quando il task si risveglia
  - Se il tempo attuale è minore del virtual time, continua
  - Se il tempo attuale è maggiore del virtual time, il task ha superato il budget
    - Ripristino il budget
    - Imposto la deadline al periodo successivo t + T

### sched_yield (SATANA)
Syscall per rinunciare al proprio budget e ripartire al successivo periodo (sia da solo che con altri task concorrenti).
Throttling volontario richiesto dall'utente.
Così l'utente può allineare la finestra temporale a come la vede il kernel.
MAI USARLA
- in un ciclo potrei fare yield invece che uare la nanosleep
- se sono vicino alla fine del budget, faccio la syscall e esaurisco il budget
- c'è throttling appena prima dello yield e salto al periodo successivo
- appena dopo, c'è yield e salto al periodo ancora successivo
  - corsa critica tra la syscall e il throttling
- La nanosleep con istante assoluto non va in esecuzione se c'è throttling => non c'è corsa critica, o meglio c'è ma non è problematica

## Global EDF
Caso con N processori
Somma U <= 1 
- in EDF è <= N

Global EDF non rispetta i vincoli real-time se U > 1
Meglio sched_deadline sui singoli processori

## cgroup
Partizionamento delle risorse e definizione delle risorse per i task nel cgroup
/sys/fs/cgroup/cpuset
mkdir p0
cd p0
echo 4 | sudo tee cpuset.cpus
echo 0 | sudo tee cpuset.mems
echo $pid | sudo tee cgroup.procs

Le risorse non sono esclusive (altri gruppi possono usare le stesse risorse).
- posso settare mem_exclusive e cpu_exclusive

A cosa serve? Mi permette di applicare global edf su un sottoinsieme di cpu e applicando la regola di schedulabilità su meno cpu.