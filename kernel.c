#include <stdint.h>
#include <stdbool.h>

#define VGA_CTRL_REGISTER 0x3d4
#define VGA_DATA_REGISTER 0x3d5
#define VGA_OFFSET_LOW 0x0f
#define VGA_OFFSET_HIGH 0x0e

#define VIDEO_ADDR 0xb8000
#define MAX_ROWS 25
#define MAX_COLS 80
#define WHITE_ON_BLACK 0x0f

#define low_16(address) (uint16_t)((address) & 0xFFFF)
#define high_16(address) (uint16_t)(((address) >> 16) & 0xFFFF)

#define IRQ1 33

#define IDT_ENTRIES 256

#define SC_MAX 57
#define BACKSPACE 0x0E
#define ENTER 0x1C

extern void isr0();	extern void isr1();	extern void isr2();	extern void isr3();
extern void isr4();	extern void isr5();	extern void isr6();	extern void isr7();
extern void isr8();	extern void isr9();	extern void isr10();	extern void isr11();
extern void isr12();	extern void isr13();	extern void isr14();	extern void isr15();
extern void isr16();	extern void isr17();	extern void isr18();	extern void isr19();
extern void isr20();	extern void isr21();	extern void isr22();	extern void isr23();
extern void isr24();	extern void isr25();	extern void isr26();	extern void isr27();
extern void isr28();	extern void isr29();	extern void isr30();	extern void isr31();

extern void irq0();	extern void irq1();	extern void irq2();	extern void irq3();
extern void irq4();	extern void irq5();	extern void irq6();	extern void irq7();
extern void irq8();	extern void irq9();	extern void irq10();	extern void irq11();
extern void irq12();	extern void irq13();	extern void irq14();	extern void irq15();

typedef struct {
	uint16_t low_offset;
	uint16_t selector;
	uint8_t always0;
	uint8_t flags;
	uint16_t high_offset;
} __attribute__((packed)) idt_gate_t;

typedef struct {
	uint32_t gs,fs,es,ds;
	uint32_t edi,esi,ebp,esp,ebx,edx,ecx,eax;
	uint32_t int_no,err_code;
	uint32_t eip,cs,eflags,useresp,ss;
} registers_t;

typedef struct {
	uint16_t limit;
	uint32_t base;
} __attribute__((packed)) idt_register_t;

typedef void (*isr_t)(registers_t *);

isr_t interrupt_handlers[256];
idt_gate_t idt[256];
static char key_buffer[256];

const char scancode_to_char[] = {
	'?', '?', '1', '2', '3', '4', '5',
	'6', '7', '8', '9', '0', '-', '=',
	'?', '?', 'Q', 'W', 'E', 'R', 'T',
	'Y', 'U', 'I', 'O', 'P', '[', ']',
       	'?', '?', 'A', 'S', 'D', 'F', 'G',
	'H', 'J', 'K', 'L', ';', '\\', '`',
	'?', '\\', 'Z', 'X', 'C', 'V', 'B',
	'N', 'M', ',', '.', '/', '?', '?',
	'?', ' '
};

void set_char_at_videomem(char character, int offset);
unsigned char port_byte_in(unsigned short port);
void port_byte_out(unsigned short port, unsigned char data);
void set_cursor(int offset);
int get_cursor();
void print_string(char *string);
int get_row_from_offset(int offset);
int get_offset(int col, int row);
int move_offset_to_newline(int offset);
void memory_copy(char *source, char *dest, int nbytes);
int scroll_ln(int offset);
void clear_screen();
void set_idt_gate(int n, uint32_t handler);
void isr_handler(registers_t *r);
void isr_install();
void irq_handler(registers_t *r);
void load_idt();
void print_letter(uint8_t scancode);
static void keyboard_callback(registers_t *regs);
void init_keyboard();
void print_nl();
void register_interrupt_handler(uint8_t n, isr_t handler);
int string_length(char s[]);
void append(char s[], char n);
bool backspace(char buffer[]);
void print_backspace();
int compare_string(char s1[], char s2[]);
void execute_command(char *input);

idt_register_t idt_reg;

void main() {
	clear_screen();
	print_string("DORK OS v0.1-beta (build-ver: rel-01)\n");
	print_string("TIP: type HELP for a list of commands!\n\n");
	print_string("Installing ISR requirements...\n");
	isr_install();
	print_string("ISR requirements installed!\n");

	print_string("Enabling external interrupts...\n");
	print_string("External interrupts enabled!\n");

	print_string("Initializing keyboard (IRQ 1)...\n");
	print_string("Finished initializing keyboard (IRQ 1)!\n\n");
	init_keyboard();

	print_string("> ");
	asm volatile("sti");

	for (;;) {
		asm volatile("hlt");
	}
}

void execute_command(char *input) {
	if (compare_string(input, "EXIT") == 0) {
		print_string("Stopping OS emulation. Goodbye!\n");
		asm volatile("hlt");
		return;
	} else if (compare_string(input, "CLEAR") == 0) {
		clear_screen();
		print_string("DORK OS v0.1-beta (build-ver: tst-01)\n\n");
		return;
	} else if (compare_string(input, "HELP") == 0) {
		print_string("EXIT      - Leave the OS\n");:
		print_string("CLEAR     - Clear the screen\n");
		print_string("HELP      - List of available commands\n");
		print_string("CRAFT ..  - Print what comes after CRAFT\n");
		print_string("INFO      - Provide info on the OS and development\n";
		return;
	} else if (string_length(input) >= 6 && input[0] == 'C' && input[1] == 'R' && input[2] == 'A' && input[3] == 'F' && input[4] == 'T' && input[5] == ' ') {
		print_string(input + 6);
       		print_nl();
		return;
	} else if (compare_string(input, "INFO") == 0) {
		print_string("BEGAN DEVELOPMENT: 4th or 5th April , 2026\n");
		print_string("FIRST RELEASED: 18/04/26\n");
		print_string("DEVELOPED BY: A nerd\n");
		return;
	}

	if (string_length(input) > 0) {
		print_string("Unknown command found: ");
		print_string(input);
		print_string("\n");
	}
}

bool backspace(char buffer[]) {
	int len = string_length(buffer);
	if (len > 0) {
		buffer[len - 1] = '\0';
		return true;
	} else {
		return false;
	}
}

void print_backspace() {
	int offset = get_cursor();
	if (offset > 4) {
		set_char_at_videomem(' ', offset - 2);
		set_cursor(offset - 2);
	}
}

void init_keyboard() {
	register_interrupt_handler(IRQ1, keyboard_callback);
}

void set_idt_gate(int n, uint32_t handler) {
	idt[n].low_offset = low_16(handler);
	idt[n].selector = 0x08;
	idt[n].always0 = 0;

	idt[n].flags = 0x8E;
	idt[n].high_offset = high_16(handler);
}

int string_length(char s[]) {
	int i = 0;
	while (s[i] != '\0') {
		++i;
	}
	return i;
}

void append(char s[], char n) {
	int len = string_length(s);
	s[len] = n;
	s[len + 1] = '\0';
}

int compare_string(char s1[], char s2[]) {
	int i;
	for (i = 0; s1[i] == s2[i]; i++) {
		if (s1[i] == '\0')
			return 0;
	}
	return s1[i] - s2[i];
}

void isr_install() {
	for (int i = 0; i < IDT_ENTRIES; i++) {
		idt[i].low_offset = 0;
		idt[i].high_offset = 0;
		idt[i].selector = 0;
		idt[i].always0 = 0;
		idt[i].flags = 0;
	}

	for (int j = 0; j < 256; j++) {
		interrupt_handlers[j] = 0;
	}

	set_idt_gate(0, (uint32_t) isr0);
	set_idt_gate(1, (uint32_t) isr1);
	set_idt_gate(2, (uint32_t) isr2);
	set_idt_gate(3, (uint32_t) isr3);
	set_idt_gate(4, (uint32_t) isr4);
	set_idt_gate(5, (uint32_t) isr5);
	set_idt_gate(6, (uint32_t) isr6);
	set_idt_gate(7, (uint32_t) isr7);
	set_idt_gate(8, (uint32_t) isr8);
	set_idt_gate(9, (uint32_t) isr9);
	set_idt_gate(10, (uint32_t) isr10);
	set_idt_gate(11, (uint32_t) isr11);
	set_idt_gate(12, (uint32_t) isr12);
	set_idt_gate(13, (uint32_t) isr13);
	set_idt_gate(14, (uint32_t) isr14);
	set_idt_gate(15, (uint32_t) isr15);
	set_idt_gate(16, (uint32_t) isr16);
	set_idt_gate(17, (uint32_t) isr17);
	set_idt_gate(18, (uint32_t) isr18);
	set_idt_gate(19, (uint32_t) isr19);
	set_idt_gate(20, (uint32_t) isr20);
	set_idt_gate(21, (uint32_t) isr21);
	set_idt_gate(22, (uint32_t) isr22);
	set_idt_gate(23, (uint32_t) isr23);
	set_idt_gate(24, (uint32_t) isr24);
	set_idt_gate(25, (uint32_t) isr25);
	set_idt_gate(26, (uint32_t) isr26);
	set_idt_gate(27, (uint32_t) isr27);
	set_idt_gate(28, (uint32_t) isr28);
	set_idt_gate(29, (uint32_t) isr29);
	set_idt_gate(30, (uint32_t) isr30);
	set_idt_gate(31, (uint32_t) isr31);

	port_byte_out(0x20,0x11);
	port_byte_out(0xA0,0x11);

	port_byte_out(0x21,0x20);
	port_byte_out(0xA1,0x28);

	port_byte_out(0x21,0x04);
	port_byte_out(0xA1,0x02);

	port_byte_out(0x21,0x01);
	port_byte_out(0xA1,0x01);

	port_byte_out(0x21,0x00);
	port_byte_out(0xA1,0x00);

	set_idt_gate(32, (uint32_t) irq0);
	set_idt_gate(33, (uint32_t) irq1);
	set_idt_gate(34, (uint32_t) irq2);
	set_idt_gate(35, (uint32_t) irq3);
	set_idt_gate(36, (uint32_t) irq4);
	set_idt_gate(37, (uint32_t) irq5);
	set_idt_gate(38, (uint32_t) irq6);
	set_idt_gate(39, (uint32_t) irq7);
	set_idt_gate(40, (uint32_t) irq8);
	set_idt_gate(41, (uint32_t) irq9);
	set_idt_gate(42, (uint32_t) irq10);
	set_idt_gate(43, (uint32_t) irq11);
	set_idt_gate(44, (uint32_t) irq12);
	set_idt_gate(45, (uint32_t) irq13);
	set_idt_gate(46, (uint32_t) irq14);
	set_idt_gate(47, (uint32_t) irq15);

	load_idt();
}

static void keyboard_callback(registers_t *regs) {
	uint8_t scancode = port_byte_in(0x60);
	if (scancode & 0x80)
		return;

	if (scancode > SC_MAX)
		return;

	if (scancode == BACKSPACE) {
		if (backspace(key_buffer)) {
			print_backspace();
		}
	} else if (scancode == ENTER) {
		print_nl();
		execute_command(key_buffer);
		key_buffer[0] = '\0';
		print_string("> ");
	} else {
		char letter = scancode_to_char[(int) scancode];
		append(key_buffer, letter);
		char str[2] = {letter, '\0'};
		print_string(str);
	}
}

void print_letter(uint8_t scancode) {
	switch (scancode) {
		case 0x0:
			print_string("ERROR");
			break;
		case 0x1:
			print_string("ESC");
			break;
		case 0x2:
			print_string("1");
			break;
		case 0x3:
			print_string("2");
			break;
		case 0x4:
			print_string("3");
			break;
		case 0x5:
			print_string("4");
			break;
		case 0x6:
			print_string("5");
			break;
		case 0x7:
			print_string("6");
			break;
		case 0x8:
			print_string("7");
			break;
		case 0x9:
			print_string("8");
			break;
		case 0x0A:
			print_string("9");
			break;
		case 0x0B:
			print_string("0");
			break;
		case 0x10:
			print_string("Q");
			break;
		case 0x11:
			print_string("W");
			break;
		case 0x12:
			print_string("E");
			break;
		case 0x13:
			print_string("R");
			break;
		case 0x14:
			print_string("T");
			break;
		case 0x15:
			print_string("Y");
			break;
		case 0x16:
			print_string("U");
			break;
		case 0x17:
			print_string("I");
			break;
		case 0x18:
			print_string("O");
			break;
		case 0x19:
			print_string("P");
			break;
		case 0x1E:
			print_string("A");
			break;
		case 0x1F:
			print_string("S");
			break;
		case 0x20:
			print_string("D");
			break;
		case 0x21:
			print_string("F");
			break;
		case 0x22:
			print_string("G");
			break;
		case 0x23:
			print_string("H");
			break;
		case 0x24:
			print_string("J");
			break;
		case 0x25:
			print_string("K");
			break;
		case 0x26:
			print_string("L");
			break;
		case 0x2C:
			print_string("Z");
			break;
		case 0x2D:
			print_string("X");
			break;
		case 0x2E:
			print_string("C");
			break;
		case 0x2F:
			print_string("V");
			break;
		case 0x30:
			print_string("B");
			break;
		case 0x31:
			print_string("N");
			break;
		case 0x32:
			print_string("M");
			break;
		case 0x2A:
			print_string("LShift");
			break;
		case 0x1C:
			print_string("Enter");
			break;
		case 0x0E:
			print_string("Backspace");
			break;
		case 0x39:
			print_string("Space");
			break;
		default:
			if (scancode <= 0x7f) {
				print_string("Unknown key down");
			} else {
				print_string("Unknown key up");
			}
	}
}

void register_interrupt_handler(uint8_t n, isr_t handler) {
	interrupt_handlers[n] = handler;
}

void print_nl() {
	print_string("\n");
}

void load_idt() {
	idt_reg.base = (uint32_t) &idt;
	idt_reg.limit = IDT_ENTRIES * sizeof(idt_gate_t) - 1;
	asm volatile("lidt (%0)" : : "r" (&idt_reg));
}

void irq_handler(registers_t *r) {
	if (interrupt_handlers[r->int_no] != 0) {
		isr_t handler = interrupt_handlers[r->int_no];
		handler(r);
	}

	port_byte_out(0x20,0x20);
	if (r->int_no >= 40) {
		port_byte_out(0xA0,0x20);
	}
}

char *exception_messages[] = {
	"Division by zero",
	"Debug",
	"NMI",
	"breakpoint",
	"Overflow",
	"Bound range",
	"Invalid opcode",
	"Device not available",
	"Double fault",
	"Coprocessor segment",
	"Invalid TSS",
	"segment not present",
	"Stack fault",
	"General protection fault",
	"Page fault",
	"Reserved",
	"x87 FP exception",
	"Alignment check",
	"Machine check",
	"SIMD FP exception",
	"Virtualization",
	"Security",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved"
};

void isr_handler(registers_t *r) {
	print_string(exception_messages[r->int_no]);
	print_nl();
}

void set_char_at_videomem(char character, int offset) {
	unsigned char *videomem = (unsigned char *) VIDEO_ADDR;
	videomem[offset] = character;
	videomem[offset + 1] = WHITE_ON_BLACK;
}

unsigned char port_byte_in(unsigned short port) {
	unsigned char result;
	__asm__("in %%dx, %%al" : "=a" (result) : "d" (port));
	return result;
}

void port_byte_out(unsigned short port, unsigned char data) {
	__asm__("out %%al, %%dx" : : "a" (data), "d" (port));
}

void set_cursor(int offset) {
	offset /= 2;
	port_byte_out(VGA_CTRL_REGISTER,VGA_OFFSET_HIGH);
	port_byte_out(VGA_DATA_REGISTER,(unsigned char)(offset >> 8));
	port_byte_out(VGA_CTRL_REGISTER,VGA_OFFSET_LOW);
	port_byte_out(VGA_DATA_REGISTER,(unsigned char)(offset & 0xff));
}

int get_cursor() {
	port_byte_out(VGA_CTRL_REGISTER,VGA_OFFSET_HIGH);
	int offset = port_byte_in(VGA_DATA_REGISTER) << 8;
	port_byte_out(VGA_CTRL_REGISTER,VGA_OFFSET_LOW);
	offset += port_byte_in(VGA_DATA_REGISTER);

	return offset * 2;
}

void print_string(char *string) {
	int offset = get_cursor();
	int i = 0;
	while (string[i] != 0) {
		if (offset >= MAX_ROWS * MAX_COLS * 2) {
			offset = scroll_ln(offset);
		}
		if (string[i] == '\n') {
			offset = move_offset_to_newline(offset);
		} else {
			set_char_at_videomem(string[i],offset);
			offset += 2;
		}
		i++;
	}
	set_cursor(offset);
}

int get_row_from_offset(int offset) {
	return offset / (2 * MAX_COLS);
}

int get_offset(int col, int row) {
	return 2 * (row * MAX_COLS + col);
}

int move_offset_to_newline(int offset) {
	return get_offset(0, get_row_from_offset(offset) + 1);
}

void memory_copy(char *source, char *dest, int nbytes) {
	if (dest < source) {
		for (int i = 0; i < nbytes; i++) {
			dest[i] = source[i];
		}
	} else {
		for (int i = nbytes - 1; i >= 0; i--) {
			dest[i] = source[i];
		}
	}
}

int scroll_ln(int offset) {
	memory_copy(
			(char *)(VIDEO_ADDR + get_offset(0,1)),
			(char *)(VIDEO_ADDR + get_offset(0,0)),
			MAX_COLS * (MAX_ROWS - 1) * 2
		   );

	for (int col = 0; col < MAX_COLS; col++) {
		set_char_at_videomem(' ', get_offset(col,MAX_ROWS - 1));
	}

	if (offset < 2 * MAX_COLS) {
		return get_offset(0, MAX_ROWS - 1);
	}
	return offset - 2 * MAX_COLS;
}

void clear_screen() {
	for (int i = 0; i < MAX_COLS * MAX_ROWS; i++) {
		set_char_at_videomem(' ', i * 2);
	}
	set_cursor(get_offset(0,0));
}
