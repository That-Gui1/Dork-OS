all: run

kernel.o: kernel.c
	gcc -m32 -ffreestanding -fno-pic -fno-asynchronous-unwind-tables -fno-unwind-tables -c $< -o $@

bootloader.o: bootloader.asm
	nasm -f elf $< -o $@

interrupts.o: interrupts.asm
	nasm -f elf $< -o $@

dorkos_image.bin: bootloader.o interrupts.o kernel.o
	ld -m elf_i386 -T linker.ld -o $@ $^ --oformat binary

run: dorkos_image.bin
	qemu-system-i386 -drive format=raw,file=dorkos_image.bin,if=floppy

clean:
	$(RM) *.bin *.o *.iso
	rm -rf iso
