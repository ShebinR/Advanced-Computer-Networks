#! /bin/bash
cat /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor

for file in /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor; do sudo echo ""performance"" > $file; done

cat /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor
