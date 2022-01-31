.PHONY: all kernel debug clean

all: debug

kernel: bin/lapis.img

bin/boot.bin: src/boot.asm | bin
	nasm \
		-f bin \
		-o $@ \
		$<

bin/kernel_entry.o: src/kernel.asm | bin
	nasm \
		-f elf64 \
		-o $@ \
		$<

bin/kernel.o: src/kernel.c | bin
	gcc \
		-g \
		-nostdlib \
		-ffreestanding \
		-c $< \
		-o $@

bin:
	mkdir -p $@

# For debug symbols.
bin/kernel.dbg: bin/kernel_entry.o bin/kernel.o | bin
	ld -o $@ -Ttext 0x1000 $^

# For build purposes.
bin/kernel.bin: bin/kernel_entry.o bin/kernel.o | bin
	ld -o $@ -Ttext 0x1000 $^ --oformat binary

bin/lapis.img: bin/boot.bin bin/kernel.bin | bin
	dd if=bin/boot.bin   of=bin/lapis.img seek=0
	dd if=bin/kernel.bin of=bin/lapis.img seek=1 conv=notrunc

debug: bin/lapis.img bin/kernel.dbg
	./debug.sh

clean:
	rm -rf bin
