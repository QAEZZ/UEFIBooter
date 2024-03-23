#!/bin/sh

# qemu-system-x86_64 \
# -cdrom ~/ISOs/ubuntu-22.04.4-desktop-amd64.iso \
# -bios /usr/share/edk2-ovmf/x64/OVMF.fd \
# -m 4096 \
# -cpu qemu64 \
# -drive format=raw,file=test.hdd \
# -machine q35 \
# -vga std \
# -usb \
# -device usb-mouse \
# -rtc base=localtime \
# -net none

# qemu-system-x86_64 \
# -bios /usr/share/edk2-ovmf/x64/OVMF.fd \
# -cdrom ~/ISOs/ubuntu-22.04.4-desktop-amd64.iso \
# -hda ./BOOTX64.EFI \
# -m 4096 \
# -machine q35 \
# -vga std \
# -usb \
# -device usb-mouse \
# -rtc base=localtime \
# -net none

# qemu-system-x86_64 \
# -bios /usr/share/edk2-ovmf/x64/OVMF.fd \
# -drive format=raw,file=/home/qaezz/ISOs/ubuntu-22.04.4-desktop-amd64.iso \
# -m 4096

qemu-system-x86_64 \
-bios /usr/share/edk2-ovmf/x64/OVMF.fd \
-drive format=raw,file=/home/qaezz/ISOs/alpine-virt-3.19.1-x86_64.iso \
-m 4096