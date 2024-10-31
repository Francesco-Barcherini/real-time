# Lez. 3 - 24/10/2024

## cpufrequtils
sudo apt install cpufrequtils
cpufreq-info, cpufreq-set etc.
- sudo cpufrq-set -g performance -c 3
  - setta lo scaling-governor a performance nella cpu 3
- for c in $(seq 0 7); do sudo cpufreq-set -c $c -g performance

## cpupower
- sudo cpupower -c 0-7 monitor/info
- sudo cpupower -c 0-7 idle-set -D 0
  - disables all idle state with latency > 0

## isolcpu
- sudo vim /etc/default/grub
  - GRUB_CMDLINE_LINUX="isolcpus=4,10"
  - sudo update-grub
  - sudo reboot
  - al boot, selezionare Ubuntu e premere "e"
    - Ctrl+X per partire con il boot
- Ci gira comunque qualcosa, ma pochissimo rispetto alle altre CPU
- uso taskset -c 4

## irq/per_cpu_count
grep "" /sys/kernel/irq/*/
- per_cpu_count
  - nr. di irq per ogni cpu

## tuned
- configurazioni a più alto livello:
https://docs.redhat.com/en/documentation/red_hat_enterprise_linux/8/html/monitoring_and_managing_system_status_and_performance/getting-started-with-tuned_monitoring-and-managing-system-status-and-performance#tuned-cpu-partitioning-profile_getting-started-with-tuned

## irqtop
sudo irqtop
- htop per gli interrupt

## Classi di scheduling

### sched_setscheduler
syscall POSIX per cambiare la classe di scheduling di un processo
- int sched_setscheduler(pid, policy, &param)
  - pid: processo
  - policy: SCHED_FIFO, SCHED_RR, SCHED_DEADLINE
  - param: struct sched_param
    - sched_priority: priorità
    - sched_deadline: deadline
    - sched_runtime: tempo di esecuzione

SCHED_FIFO
struct sched_param params = {
        .sched_priority = 100
    };
sched_setscheduler(gettid(), SCHED_FIFO, &params);
- con getpid() non considero i thread -> uso gettid()
- con SCHED_FIFO a priorità RT comunque sono sotto SCHED_DEADLINE

### sched_setattr
int sched_setattr(pid, &attr, flags)

## memlock
Come posso dormire:
- intr
- sleep
- primitive bloccanti
- malloc -> mmap -> swap di altri porcessi -> mi addormenta e risveglia quando lo swap è terminato (meglio allocare la memoria prima)
- un altro processo fa malloc e il kernel mi mette in swap -> lock sulla memoria -> mlock

### mlock
mlockall(MLC_FUTURE);
- blocca la memoria ora e in futuro in RAM
mlockall(MLC_CURRENT):
- blocca solo ciò che è già allocato?
mlock(addr, size)
- blocca gli indirizzi specificati

### /etc/security/limits.conf
- rtprio: priorità max rt per processi non root
  - tommaso hard rtprio 50
  - @group  hard rtprio 50
    - a tommaso è concesso utilizzare priorità RT fino a 50
- memlock: quanti byte lockabili per utente/gruppo
  - tommaso hard memlock 16384
    - tommaso può lockare max 16384 Byte
Per programmi real-time audio. A noi basta runnare con sudo.

### disabilito lo swap
sudo swapon/swapoff
sudo mkswap /swapfile [size]
- in /etc/fstab
  - aggiungere /swapfile
  - fa il mount automatico al boot

dpkg -l | grep swap
- scopri se c'è qualche programma che fa swap in automatico