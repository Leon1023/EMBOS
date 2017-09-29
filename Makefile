CC=arm_elf_gcc
LD=arm_elf_ld
OBJCOPY=arm_elf_objcopy

CFLAGS= -O2 -g
ASFLAGS= -O2 -g
LDFLAGS= -TLeonOS.lds -Ttext 0x30000000

OBJS=init.o start.o boot.o abnormal.o mmu.o

.c.o:
	$(CC) $(CFLAGS) -c $<
.s.o:
	$(CC) $(ASFLAGS) -c $<

LeonOS:$(OBJS)
	$(CC) -static -nostartfiles -nostdlib $(LDFLAGS) $? -o $@ -lgcc
	$(OBJCOPY) -O -binary $@ LeonOS.bin

clean:
	rm -f *.o LeonOS LeonOS.bin 
