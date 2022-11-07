#Makefile

SHELL = /usr/bin/sh

# Edit the following path to vpi_user.h location
INCDIRS += /usr/local/include/iverilog
# Edit the following path to libvpi.a
LIBDIRS += /usr/local/lib

SRCDIR = .
BUILDDIR = build
INCDIRS += $(SRCDIR)/include
INCDIRS += $(SRCDIR)/cjson
LIBVPI = $(BUILDDIR)/verisocks.vpi

CC = /usr/bin/gcc

CFLAGS += -fPIC
CFLAGS += -Wall
CFLAGS += -Wextra -Wpedantic
CFLAGS += $(addprefix -I,$(INCDIRS))
CFLAGS += -DVS_LOG_LEVEL=30
CFLAGS += -DVS_VPI_LOG_LEVEL=20
LDFLAGS += $(addprefix -L,$(LIBDIRS))

VPATH = $(SRCDIR)/cjson $(SRCDIR)/src

SRCS = \
	cJSON.c \
	vs_msg.c \
	vs_server.c \
	vs_vpi.c \
	vs_vpi_get.c \
	vs_vpi_run.c \
	vs_utils.c \
	verisocks.c \
	verisocks_startup.c

OBJS = $(addprefix $(BUILDDIR)/,$(subst .c,.o,$(SRCS)))

all: $(LIBVPI)

$(LIBVPI): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -shared -o $@ $^ -lvpi

$(BUILDDIR)/%.o: %.c
	@mkdir -p $(BUILDDIR)
	$(CC) -o $@ -c $(CFLAGS) $<

.PHONY: clean all

clean:
	-$(RM) $(OBJS)
	-$(RM) $(LIBVPI)
