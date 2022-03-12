#!/bin/bash

set -e

cat /dev/null > /tmp/serial.log
cat /dev/null > /tmp/serial.raw
alacritty --class lapis-serial-log -e fish -c 'tail -f /tmp/serial.raw' &
alacritty --class lapis-serial-log -e tail -f /tmp/serial.log &
sleep 1

alacritty --class qemu-starter -e \
	qemu-system-x86_64 \
	-no-reboot \
	-no-shutdown \
	-S \
	-gdb tcp::1234 \
	-m size=128M \
	-serial file:/tmp/serial.log \
	-serial file:/tmp/serial.raw \
	-drive media=disk,index=0,file=bin/zipfs.img,format=raw,if=ide \
	-drive media=disk,index=1,file=bin/fat32.img,format=raw,if=ide \
	-device e1000,netdev=n1,mac=52:54:00:12:34:56 \
	-cdrom bin/kernel.iso &

gdb \
	--quiet \
	-command=debug.gdb

ps ax | grep alacritty | grep lapis-serial-log | awk '{print $1}' | xargs kill
ps ax | grep qemu-system-x86_64 | grep kernel\.iso | awk '{print $1}' | xargs kill
