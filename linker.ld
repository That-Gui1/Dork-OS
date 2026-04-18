SECTIONS {
	. = 0x7C00;
	.text : {
		bootloader.o(.text)
	}

	/DISCARD/ : {
		*(.eh_frame)
		*(.comment)
	}

	. = 0x7C00 + 510;
	.bootsig : { *(.bootsig) }

	. = 0x1000;
	.kernel : AT(0x7C00 + 512) {
		kernel.o(.text)
		kernel.o(.rodata)
		kernel.o(.data)
		kernel.o(.bss)
		interrupts.o(.text)
	}
}
