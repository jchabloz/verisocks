#Makefile for unit tests using CUnit framework
SHELL = /usr/bin/bash
CC = gcc
CFLAGS += -Wall
CFLAGS += -Wextra -Wpedantic
# CFLAGS += -DVS_MSG_DEBUG
# CFLAGS += -DVS_SERVER_DEBUG
LDFLAGS += -lcunit

BUILDDIR = build
INCDIRS = -I../include -I../cjson
SRC_FILES = ../src/vs_msg.c ../src/vs_server.c
LIBSRC_FILES = ../cjson/cJSON.c

tests = cunit_test

all: build run

build: $(addprefix $(BUILDDIR)/,$(tests))

run: build
	cd $(BUILDDIR) && $(foreach x,$(tests),./$(x);)

$(BUILDDIR)/%: %.c $(SRC_FILES) $(LIBSRC_FILES)
	@mkdir -p $(BUILDDIR)
	$(CC) -o $@ $^ $(CFLAGS) $(INCDIRS) $(LDFLAGS)

sandbox: sandbox.c
	$(CC) -o $@ $^ $(CFLAGS) $(INCDIRS) $(LDFLAGS)
	./sandbox

.PHONY: clean all build run report

clean:
	-$(RM) -rf $(BUILDDIR)
	-$(RM) -f sandbox
