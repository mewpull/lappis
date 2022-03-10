#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdarg.h>

#include "tinyprintf.c"
#include "memcpy.c"

#include "ports.c"
#include "string.c"
#include "serial.c"
#include "multiboot.c"
#include "heap.c"
#include "terminal-font.h"
#include "video.c"
#include "mandel.c"
#include "print.c"
#include "pic.c"
#include "idt.c"
#include "fpu.c"
#include "format/zip.c"

void warn_interrupt(uint8_t interrupt_number) {
	error("Interrupt: %d/%x!", interrupt_number, interrupt_number);

	uint8_t  red[4]     = {255, 0, 0, 0xff};
	uint8_t* error_name = malloc(256);
	if (error_name == NULL) {
		error("Could not allocate memory for error name!");
		return;
	}
	num_to_error_name(interrupt_number, error_name);

	uint8_t* tmp = malloc(256);
	if (tmp == NULL) {
		error("Could not allocate memory for tmp!");
		return;
	}
	snprintf(tmp, 256, "Warning! Interrupt occurred: %s [%d/%x]", error_name, interrupt_number, interrupt_number);
	print_box(tmp, LARGE_FONT_CELL_WIDTH, 2, red);
}

void end_of_execution() {
	uint8_t green[4] = {0, 255, 0, 0xff};
	print_box("End of execution.", LARGE_FONT_CELL_WIDTH, 2, green);
}

void main(multiboot_info_t* boot_info) {
	init_serial(SERIAL_COM1_PORT);
	init_serial(SERIAL_COM2_PORT);

	init_printf(NULL, serial_debug_write_byte);

	debug("Kernel started");
	// Setup interrupts.
	idt_init();

	parse_multiboot_header(boot_info);

	// Enable floating point operations.
	enable_fpu();

	ide_dev dev = {
		ATA_PRIMARY_BUS,
		ATA_PRIMARY_DRIVE,
		ATA_PRIMARY_IO,
	};
	uint64_t fsbuf_size = 0x501000;
	uint8_t* fsbuf = malloc(fsbuf_size);
	if (fsbuf == NULL) {
		error("Could not allocate memory for file system buffer!");
		return;
	}
	// TODO: implement an actual file system.
	debug("> ATA");
	ata_read(fsbuf, 0, fsbuf_size / 512, &dev);
	debug("< ATA");

	debug("> zipfs");
	uint64_t zipfs_size = peek_zip(fsbuf);
	if (zipfs_size == 0) {
		error("Malformed zipfs! Maybe `fsbuf_size` is too small? Aborting.");
		return;
	}
	if (zipfs_size >= fsbuf_size) {
		error("Unable to read file system: `fsbuf_size` is too small!");
		return;
	}

	zip_fs_t zipfs;
	read_zip(fsbuf, zipfs_size, &zipfs);
	debug("< zipfs");

	debug("File system initialized.");
	for (uint32_t i = 0; i < zipfs.num_files; i++) {
		file_t* file = zipfs.files[i];
		debug(file->name);
		if (streq(file->name, "bg.raw") == false) {
			continue;
		}
		debug("found bg.raw!");
		set_frame(file->data);
	}

	while (1) {}
}
