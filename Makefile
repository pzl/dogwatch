VERSION = 0.0.1

CC = gcc
CFLAGS += -Wall -Wextra
CFLAGS += -Wfloat-equal -Wundef -Wshadow -Wpointer-arith -Wcast-align
CFLAGS += -Wstrict-prototypes -Wwrite-strings -ftrapv
#CFLAGS += -Wpadded
#CFLAGS += -fsanitize=address
#CFLAGS += -march=native
CFLAGS += -pthread
#CFLAGS += $(shell pkg-config --cflags portaudio-2.0 ncurses)
CFLAGS += $(shell pkg-config --cflags cairo)
CFLAGS += -D_XOPEN_SOURCE
SFLAGS = -std=c99 -pedantic
SRCDIR = src
OBJDIR = out
LDFLAGS += 
INCLUDES = -I.
#LIBS = -lpthread $(shell pkg-config --libs portaudio-2.0 ncurses)
LIBS = -lpthread -lm -lportaudio -lncurses
LIBS += $(shell pkg-config --libs cairo)
SRCS = $(filter-out $(SRCDIR)/dog2raw.c, $(wildcard $(SRCDIR)/*.c))
OBJS=$(SRCS:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
TARGET=dogwatch

HSRC = $(SRCDIR)/dog2raw.c
HOBJ = $(OBJDIR)/dog2raw.o $(OBJDIR)/audioin.o $(OBJDIR)/file.o
HTARGT=dog2raw

PREFIX ?= /usr
BINPREFIX = $(PREFIX)/bin
MANPREFIX = $(PREFIX)/share/man
BASHCPL = $(PREFIX)/share/bash-completion/completions
ZSHCPL = $(PREFIX)/share/zsh/site-functions


all: CFLAGS += -O2
all: $(TARGET) $(HTARGT)

debug: CC = clang
debug: CFLAGS += -O0 -g -DDEBUG
debug: $(TARGET) $(HTARGT)

$(OBJS): Makefile

dummy := $(shell test -d $(OBJDIR) || mkdir -p $(OBJDIR))


$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(TARGET) $(OBJS) $(LDFLAGS) $(LIBS)

$(HTARGT): $(HOBJ)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(HTARGT) $(HOBJ) $(LDFLAGS) $(LIBS)

install:
	install -D -m 755  $(TARGET) "$(DESTDIR)$(BINPREFIX)"
	install -D -m 644 doc/$(TARGET).1 "$(DESTDIR)$(MANPREFIX)"/man1
	install -D -m 644 contrib/bash_completion "$(DESTDIR)$(BASHCPL)"/$(TARGET)
	#install -D -m 644 contrib/zsh_completion "$(DESTDIR)$(ZSHCPL)"/_$(TARGET)

uninstall:
	$(RM) "$(DESTDIR)$(BINPREFIX)"/$(TARGET)
	$(RM) "$(DESTDIR)$(BINPREFIX)"/man1/$(TARGET)
	$(RM) "$(DESTDIR)$(BASHCPL)"/$(TARGET)
	$(RM) "$(DESTDIR)$(ZSHCPL)"/_$(TARGET)


doc:
	a2x -v -d manpage -f manpage -a revnumber=$(VERSION) doc/$(TARGET).1.txt

$(OBJS): $(OBJDIR)/%.o : $(SRCDIR)/%.c
	$(CC) $(SFLAGS) $(CFLAGS) $(INCLUDES) -c -o $@ $<

$(OBJDIR)/dog2raw.o: $(SRCDIR)/dog2raw.c
	$(CC) $(SFLAGS) $(CFLAGS) $(INCLUDES) -c -o $@ $<

clean:
	$(RM) $(OBJS) $(TARGET) $(HTARGT) $(HOBJ)

.PHONY: all debug install uninstall doc clean
