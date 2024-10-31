#!/bin/sh
NCPU=8
CPUS=$(seq 0 $(($NCPU-1)))
CPU=${1:-1}
HYPERCPU=$((($CPU+$NCPU/2) % $NCPU))
MINFREQ=400000
MAXFREQ=4700000

# SCHED RUNTIME
if [ -w /proc/sys/kernel/sched_rt_runtime_us ]; then
    echo "Set sched_rt_runtime_us to 950000"
    echo 950000 | sudo tee /proc/sys/kernel/sched_rt_runtime_us
fi

# CPU HYPERTHREADING
if [ -w /sys/devices/system/cpu/cpu$HYPERCPU/online ]; then
    echo "Enable hyperthreaded CPU$HYPERCPU"
    echo 1 | sudo tee /sys/devices/system/cpu/cpu$HYPERCPU/online
fi

# CPU IRQ AFFINITY
# unset the property IRQBALANCE_BANNED_CPUS
if [ -w /etc/default/irqbalance ]; then
    echo "Remove CPUs $CPU,$HYPERCPU from banned irqbalance CPUs"

    if grep -q "^IRQBALANCE_BANNED_CPULIST=" /etc/default/irqbalance; then
        sudo sed -i "s/^IRQBALANCE_BANNED_CPULIST=.*/#IRQBALANCE_BANNED_CPULIST=/" /etc/default/irqbalance
    fi

    sudo systemctl restart irqbalance
fi

# CPU IDLE
for i in $CPU $HYPERCPU; do
    if [ -d /sys/devices/system/cpu/cpu$i/cpuidle ]; then
        echo "Enable CPU$i idle states"
        # prendi tutte le cartelle state* in /sys/devices/system/cpu/cpu$i/cpuidle
        STATES=$(ls /sys/devices/system/cpu/cpu$i/cpuidle | grep state)
        for s in $STATES; do
            if [ -w /sys/devices/system/cpu/cpu$i/cpuidle/$s/disable ]; then
                echo "Enable CPU$i idle $s"
                echo 0 | sudo tee /sys/devices/system/cpu/cpu$i/cpuidle/$s/disable
            fi
        done
    fi
done

# CPU FREQUENCY
if [ -d /sys/devices/system/cpu/cpufreq ]; then
    echo "Set CPUs min_frequency and max_frequency to $MINFREQ and $MAXFREQ"
    echo $MAXFREQ | sudo tee /sys/devices/system/cpu/cpufreq/policy*/scaling_max_freq
    echo $MINFREQ | sudo tee /sys/devices/system/cpu/cpufreq/policy*/scaling_min_freq
fi

# CPU GOVERNOR
for i in $CPU $HYPERCPU; do
    if [ -w /sys/devices/system/cpu/cpu$i/cpufreq/scaling_governor ]; then
        echo "Set CPU$i governor to powersave"
        echo powersave | sudo tee /sys/devices/system/cpu/cpu$i/cpufreq/scaling_governor
    fi
done

# TURBO BOOST
if [ -w /sys/devices/system/cpu/intel_pstate ]; then
    echo "Enable turbo boost"
    echo 0 | sudo tee /sys/devices/system/cpu/intel_pstate/no_turbo
fi

# CHECK
echo ""
grep -H "" /sys/devices/system/cpu/intel_pstate/no_turbo

echo ""
grep -H "" /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor

echo ""
grep -H "" /sys/devices/system/cpu/cpufreq/policy*/scaling_m*_freq

echo ""
grep -H "" /sys/devices/system/cpu/cpu*/cpuidle/*/disable
.
echo ""
grep -H "^IRQBALANCE_BANNED_CPULIST" /etc/default/irqbalance
grep -H $CPU /proc/irq/*/effective_affinity_list
grep -H $HYPERCPU /proc/irq/*/effective_affinity_list

echo ""
grep -H "" /sys/devices/system/cpu/cpu*/online

echo ""
grep -H "" /proc/sys/kernel/sched_rt*

echo ""
grep -H "isolcpus" /etc/default/grub