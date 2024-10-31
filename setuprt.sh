#!/bin/sh
NCPU=8
CPUS=$(seq 0 $(($NCPU-1)))
CPU=${1:-1}
HYPERCPU=$((($CPU+$NCPU/2) % $NCPU))

# TURBO BOOST
if [ -w /sys/devices/system/cpu/intel_pstate ]; then
    echo "Disable turbo boost"
    echo 1 | sudo tee /sys/devices/system/cpu/intel_pstate/no_turbo > /dev/null
fi

# CPU GOVERNOR
for i in $CPU $HYPERCPU; do
    if [ -w /sys/devices/system/cpu/cpu$i/cpufreq/scaling_governor ]; then
        echo "Set CPU$i governor to performance"
        echo performance | sudo tee /sys/devices/system/cpu/cpu$i/cpufreq/scaling_governor > /dev/null
    fi
done

# CPU FREQUENCY
if [ -d /sys/devices/system/cpu/cpufreq ]; then
    # take the scaling_max_freq of CPU$CPU and apply it to all policies
    if [ -r /sys/devices/system/cpu/cpu$CPU/cpufreq/scaling_max_freq ]; then
        FREQ=$(cat /sys/devices/system/cpu/cpu$CPU/cpufreq/scaling_max_freq)
        echo "Set CPUs min_frequency and max_frequency to $FREQ"

        echo $FREQ | sudo tee /sys/devices/system/cpu/cpufreq/policy*/scaling_max_freq > /dev/null
        echo $FREQ | sudo tee /sys/devices/system/cpu/cpufreq/policy*/scaling_min_freq > /dev/null
    fi
fi

# CPU IDLE
for i in $CPU $HYPERCPU; do
    if [ -d /sys/devices/system/cpu/cpu$i/cpuidle ]; then
        echo "Disable CPU$i idle states"
        # prendi tutte le cartelle state* in /sys/devices/system/cpu/cpu$i/cpuidle
        STATES=$(ls /sys/devices/system/cpu/cpu$i/cpuidle | grep state)
        for s in $STATES; do
            if [ -w /sys/devices/system/cpu/cpu$i/cpuidle/$s/disable ]; then
                echo "Disable CPU$i idle $s"
                echo 1 | sudo tee /sys/devices/system/cpu/cpu$i/cpuidle/$s/disable > /dev/null
            fi
        done
    fi
done

# CPU IRQ AFFINITY
# set the property IRQBALANCE_BANNED_CPUS to the CPUs that should not be used by irqbalance
if [ -w /etc/default/irqbalance ]; then
    echo "Set irqbalance banned CPUs to $CPU,$HYPERCPU"
    if grep -q "^IRQBALANCE_BANNED_CPULIST=" /etc/default/irqbalance; then
        sudo sed -i "s/^IRQBALANCE_BANNED_CPULIST=.*/IRQBALANCE_BANNED_CPULIST=$CPU,$HYPERCPU/" /etc/default/irqbalance
    else
        echo "IRQBALANCE_BANNED_CPULIST=$CPU,$HYPERCPU" | sudo tee -a /etc/default/irqbalance > /dev/null
    fi
    sudo systemctl restart irqbalance
fi

# CPU HYPERTHREADING
if [ -w /sys/devices/system/cpu/cpu$HYPERCPU/online ]; then
    echo "Disable hyperthreaded CPU$HYPERCPU"
    echo 0 | sudo tee /sys/devices/system/cpu/cpu$HYPERCPU/online > /dev/null
fi

# SCHED RUNTIME
if [ -w /proc/sys/kernel/sched_rt_runtime_us ]; then
    echo "Set sched_rt_runtime_us to -1"
    echo -1 | sudo tee /proc/sys/kernel/sched_rt_runtime_us > /dev/null
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
