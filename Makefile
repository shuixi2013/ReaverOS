all: chdd bochs

q: chdd qemu

u: cuefi qemu-uefi

bochs:
	cd builds && bochs -q

qemu:
	cd builds && qemu-system-x86_64 -hda hdd.img -monitor stdio -no-kvm -m 2048

uefi-qemu:
	cd builds/efi && qemu-system-x86_64 -L . -bios OVMF.fd -m 2048 -cpu kvm64 -hda efidisk.hdd -enable-kvm

chdd:
	colormake hdd

cuefi:
	colormake uefi

hdd:
	cd loader/hdd; \
	yasm stage1.asm -o ../../builds/stage1.img
	cd loader/hdd/stage2; \
	yasm stage2.asm -o ../../../builds/stage2.img
	cd loader/booter; \
	colormake; \
	mv builds/booter.img ../../builds/
	cd kernel; \
	colormake; \
	mv builds/kernel.img ../builds/
	cd builds; \
	./mkrfloppy a.img stage1.img stage2.img booter.img kernel.img stage3.img; \
	dd if=a.img of=hdd.img conv=notrunc

uefi:
	cd loader/uefi; \
	colormake
	cd builds/efi; \
	cp $(UDKPATH)/MyWorkSpace/Build/MdeModule/RELEASE_GCC46/X64/roseuefi.efi ./EFI/BOOT/BOOTX64.EFI; \
	sudo losetup --offset 1048576 --sizelimit 66060288 /dev/loop0 efidisk.hdd; \
	sudo mount /dev/loop0 ./mount; \
	sudo mv ./EFI/BOOT/BOOTX64.EFI ./mount/EFI/BOOT/BOOTX64.EFI
	sleep 3s
	make unmount

unmount:
	cd builds/efi && sudo umount ./mount && sudo losetup -d /dev/loop0

clean:
	cd loader/booter; \
	colormake clean
	cd kernel; \
	colormake clean

uefi-clean: clean
	cd loader/uefi; \
	colormake clean
