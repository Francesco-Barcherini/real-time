# Lez. 2 - 18/10/2024

## classi di scheduler
- sched normale
- sched_fifo (niceness)
- ... (da completare)
- sched_realtime: 99 classi
- sched_deadline
  - controllo della banda: somma C/deadline <= N. processori*0.95
  - non garantisci la schedulabilità
  - se la somma dei C/deadline < 1 => garantisci schedulabilità
  - lanci task deadline con chrt -D <ns>
  - => se viene lanciato un task con priorità minore, c'è interferenza

## cambio CPU
Se la CPU in cui ero è occupata, lo scheduler cambia CPU e cerca una nuova CPU idle.
Voglio che il real-time sia rispettato anche se ci sono altri task in parallelo.
Se cambio CPU perdo la cache e devo aspettare l'interrupt che risveglia l'altra CPU (inter-processor interrupt).

taskset -c 0 ./a.out (quale CPU)
  - -a : all threads figli
  - mette un affinity mask nel des_proc

/proc/sys/kernel/sched_rt_runtime_us = 950000
- ogni secondo lascio 50ms per task dell'OS
- rompe POSIX ma permette di evitare problemi in caso di loop o bug
- se lo metto a -1, non c'è limite: il task real-time può prendersi tutta la CPU (il kernel si lamenta in dmesg)

taskset -c 0 chrt 99 ./a.out

## cpuidle
/sys/devices/system/cpu/cpu*/cpuidle/state*/
- /disable => disabilita lo stato
- /latency => tempo per passare da idle a active
  - il kernel sceglie lo stato con latenza minore purché non consumi troppo
  - si basa sul timer (se il prossimo timer si attiva tra 100us, allora uso lo state0 con latency 1us)
  - il kernel si basa su un'euristica per scegliere lo stato
- /time e /usage sono contatori per vedere quanto tempo è stato in idle

Il problema è che il kernel sceglie lo stato idle in modo non predicibile.
Perciò aumenta la standard deviation delle latency.

echo 1 | sudo tee /sys/devices/system/cpu/cpu*/cpuidle/state*/disable

## irq affinity
Posso impedire che un interrupt esegua sulla cpu dove sto eseguendo il mio task.

/proc/irq/default_smp_affinity
- ff => tutte le CPU
/pro/irq/*/effective_affinity_list
- dov'è effettivamente l'interrupt
/proc/irq/*/smp_affinity oppure /proc/irq/*/smp_affinity_list
- dove può andare l'interrupt
- maschera effettiva: un daemon irq_balance bilancia le interrupt sulle cpu
- se lo cambio manualmente, irq_balance lo cambia di nuovo
  - systemctl stop irq_balance

In /etc/default/irqbalance posso disabilitare irq_balance
- IRQBALANCE_BANNED_CPULIST=4,10 // c'è l'hyperthreading: 4 e 10 sono la stessa CPU

### hyperthreading
Voglio che sulla cpu hyperthreading non giri nulla, altrimenti c'è condivisione delle ALU e uno slowdown.
echo 0 | sudo tee /sys/devices/system/cpu/cpu0/online (DA RIVEDERE)

Si può togliere turbo-boost e hyperthreading dal BIOS.

### evitare printf in programmi real-time

### evitare clock_nanosleep
- mettiamo la cpu in idle e perdiamo il controllo
- usiamo un'attesa attiva => non abbiamo più context switch
- questo serve ad esempio quando non posso permettermi la latenza del context switch (telecomunicazioni)

Se rimetto 950000 in sched_rt_runtime_us, il task real-time viene bloccato dopo 950ms. Se il periodo è sottomultiplo di 1s, se sono fortunato inizio il task sincronizzato con il secondo e i 50ms corrispondono al periodo di attesa. Altrimenti ho un'attesa randomizzata sul secondo.

