# System independendent makefile.
# Since we want LDMX to work on any system, we can't be reliant on the native gcc.
# So this provides a model into how LDMX code will be compiled to achieve this independence.
# It builds $TARGET without relying on the system's glibc and using its own instead,
# hence the -nostdlib -nostartfiles flags.
# Reference:
# https://gcc.gnu.org/onlinedocs/gcc-7.3.0/gcc/Link-Options.html#Link-Options
# It only wortks for C sources at the moment.
# Custom binutils needs to be installed to have it's own linker (ld)
OBJ = $(TARGET).o
SRC = $(TARGET).c
CC = /home/cosminst/ldmx/ldmx_190706/ldmx-sw/lib/gcc-7.3.0/bin/gcc
CFLAGS = -g
LDFLAGS = -nostdlib -nostartfiles -static
GLIBCDIR = /home/cosminst/ldmx/ldmx_190706/ldmx-sw/lib/glibc-2.25/lib
STARTFILES = $(GLIBCDIR)/crt1.o $(GLIBCDIR)/crti.o `$(CC) --print-file-name=crtbegin.o`
ENDFILES = `$(CC) --print-file-name=crtend.o` $(GLIBCDIR)/crtn.o
LIBGROUP = -Wl,--start-group $(GLIBCDIR)/libc.a -lgcc -lgcc_eh -Wl,--end-group

$(TARGET): $(OBJ)
		$(CC) $(LDFLAGS) -o $@ $(STARTFILES) $^ $(LIBGROUP) $(ENDFILES) 

$(OBJ): $(SRC)
		$(CC) $(CFLAGS) -c $^

clean:
		rm -f *.o *.~ $(TARGET)