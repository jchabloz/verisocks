#Makefile for unit tests using Unity framework
SHELL = /usr/bin/bash
CC = gcc
CFLAGS += -Wall
CFLAGS += -Wextra -Wpedantic
# CFLAGS += -DVS_MSG_DEBUG

BUILDDIR = build
INCDIRS = -I../include -I../cjson -I../unity/src
LIBSRC_FILES = ../cjson/cJSON.c ../unity/src/unity.c

tests = test_vs_msg test_vs_server

all: run

$(BUILDDIR)/test_%: test_%.c ../include/%.h ../src/%.c $(LIBSRC_FILES)
	@mkdir -p $(BUILDDIR)
	$(CC) -o $@ $^ $(CFLAGS) $(INCDIRS) $(LDFLAGS)

sandbox: sandbox.c
	$(CC) -o $@ $^ $(CFLAGS) $(INCDIRS) $(LDFLAGS)
	./sandbox

run: $(addprefix $(BUILDDIR)/,$(tests))
	cd $(BUILDDIR) && $(foreach x,$(tests),./$(x);)

.PHONY: clean all run

clean:
	-$(RM) -rf $(BUILDDIR)
	-$(RM) -f sandbox
