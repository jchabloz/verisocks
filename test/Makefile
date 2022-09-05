#Makefile for unit tests using CUnit framework
SHELL = /usr/bin/bash
CC = gcc
CFLAGS += -Wall
CFLAGS += -Wextra -Wpedantic
CFLAGS += -fprofile-arcs -ftest-coverage
CFLAGS += -g -O1
# CFLAGS += -DVS_MSG_DEBUG
# CFLAGS += -DVS_SERVER_DEBUG
LDFLAGS += -lcunit

BUILDDIR = build
INCDIRS = -I../include -I../cjson
SRC_FILES = ../src/vs_msg.c ../src/vs_server.c
LIBSRC_FILES = ../cjson/cJSON.c

xml_file = $(BUILDDIR)/CUnitAutomated-Results.xml
xsl_file = /usr/share/CUnit/CUnit-Run.xsl
result_file = $(BUILDDIR)/cunit_results.html

tests = cunit_test

all: build run $(result_file)

build: $(addprefix $(BUILDDIR)/,$(tests))

run: build
	cd $(BUILDDIR) && $(foreach x,$(tests), valgrind --leak-check=full --log-file=$(x)_valgrind.rpt ./$(x);)

$(result_file): $(xml_file)
	xsltproc --novalid -o $(result_file) $(xsl_file) $(xml_file)

$(BUILDDIR)/%: %.c $(SRC_FILES) $(LIBSRC_FILES)
	@mkdir -p $(BUILDDIR)
	$(CC) -o $@ $^ $(CFLAGS) $(INCDIRS) $(LDFLAGS)

%.c.gcov: %.gcda
	gcov -s ../src $(patsubst %.c.gcov,%.c,$@)

gcov: $(patsubst %.c,%.c.gcov,$(notdir $(SRC_FILES)))

.PHONY: clean all build run gcov

clean:
	-$(RM) -r $(BUILDDIR)
	-$(RM) *.gcda *.gcno *.gcov
