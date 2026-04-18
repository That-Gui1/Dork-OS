all: run

kernel.o: kernel.c
	gcc -m32 -ffreestanding -fno-pic -fno-asynchronous-unwind-tables -fno-unwind-tables -c $< -o $@

bootloader.o: bootloader.asm
	nasm -f elf $< -o $@

interrupts.o: interrupts.asm
	nasm -f elf $< -o $@

dorkos_image.bin: bootloader.o interrupts.o kernel.o
	ld -m elf_i386 -T linker.ld -o $@ $^ --oformat binary

dorkos_64bit.iso: dorkos_image.bin
	mkdir -p iso/boot
	cp os_image.bin iso/boot/os_image.bin
	truncate -s 1474560 iso/boot/os_image.bin
	genisoimage -R -J -V "DORK_OS" \
		-b boot/os_image.bin \
		-no-emul-boot \
		-boot-load-size 4 \
		-boot-info-table \
		-c boot/boot.cat \
		-o $@ iso 2>/dev/null

run: os_image.bin
	qemu-system-i386 -drive format=raw,file=dorkos_image.bin,if=floppy

clean:
	$(RM) *.bin *.o *.iso
	rm -rf iso
