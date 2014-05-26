VERSION = 0.0.1

CC = gcc
CFLAGS += -Wall -Wextra
CFLAGS += -Wfloat-equal -Wundef -Wshadow -Wpointer-arith -Wcast-align
CFLAGS += -Wstrict-prototypes -Wwrite-strings -Wpadded -ftrapv
CFLAGS += -fsanitize=address
CFLAGS += -march=native
SFLAGS = -std=c99 -pedantic
SRCDIR = src
OBJDIR = out
LDFLAGS += 
INCLUDES = -I.
LIBS =
SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS=$(SRCS:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
TARGET=dogwatch

PREFIX ?= /usr
BINPREFIX = $(PREFIX)/bin
MANPREFIX = $(PREFIX)/share/man
BASHCPL = $(PREFIX)/share/bash-completion/completions
ZSHCPL = $(PREFIX)/share/zsh/site-functions


all: CFLAGS += -O2
all: $(TARGET)

debug: CC = clang
debug: CFLAGS += -O0 -g -DDEBUG
debug: $(TARGET)

$(OBJS): Makefile

dummy := $(shell test -d $(builddir) || mkdir -p $(builddir))


$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(TARGET) $(OBJS) $(LDFLAGS) $(LIBS)

install:
	mkdir -p "$(DESTDIR)$(BINPREFIX)"
	cp -p $(TARGET) "$(DESTDIR)$(BINPREFIX)"
	mkdir -p "$(DESTDIR)$(MANPREFIX)"/man1
	cp -Pp doc/$(TARGET).1 "$(DESTDIR)$(MANPREFIX)"/man1
	mkdir -p "$(DESTDIR)$(BASHCPL)"
	cp -p contrib/bash_completion "$(DESTDIR)$(BASHCPL)"/$(TARGET)
	mkdir -p "$(DESTDIR)$(ZSHCPL)"
	cp -p contrib/zsh_completion "$(DESTDIR)$(ZSHCPL)"/_$(TARGET)

uninstall:
	$(RM) "$(DESTDIR)$(BINPREFIX)"/$(TARGET)
	$(RM) "$(DESTDIR)$(BINPREFIX)"/man1/$(TARGET)
	$(RM) "$(DESTDIR)$(BASHCPL)"/$(TARGET)
	$(RM) "$(DESTDIR)$(ZSHCPL)"/_$(TARGET)


doc:
	a2x -v -d manpage -f manpage -a revnumber=$(VERSION) doc/$(TARGET).1.txt

$(OBJS): $(OBJDIR)/%.o : $(SRCDIR)/%.c
	$(CC) $(SFLAGS) $(CFLAGS) $(INCLUDES) -c -o $@ $<

clean:
	$(RM) $(OBJS) $(TARGET)

.PHONY: all debug install uninstall doc clean
