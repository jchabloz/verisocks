#Makefile for unit tests using CUnit framework
SHELL = /usr/bin/bash
CC = gcc
CFLAGS += -Wall
CFLAGS += -Wextra -Wpedantic
# CFLAGS += -fprofile-arcs -ftest-coverage
CFLAGS += --coverage
CFLAGS += -g -O1
CFLAGS += -DVS_LOG_LEVEL=100
LDFLAGS += -lcunit

BUILDDIR = build
INCDIRS = -I../include -I../cjson
SRC_FILES = ../src/vs_msg.c ../src/vs_server.c
TEST_SRC_FILES = test_vs_msg.c test_vs_server.c
LIBSRC_FILES = ../cjson/cJSON.c

xml_file = $(BUILDDIR)/CUnitAutomated-Results.xml
xsl_file = /usr/share/CUnit/CUnit-Run.xsl
result_file = $(BUILDDIR)/cunit_results.html

tests = cunit_test

all: build run $(result_file) lcov

build: $(addprefix $(BUILDDIR)/,$(tests))

run: build
	cd $(BUILDDIR) && $(foreach x,$(tests), valgrind --leak-check=full --log-file=$(x)_valgrind.rpt ./$(x);)
	-mv -f *.gcda $(BUILDDIR)
	-mv -f *.gcno $(BUILDDIR)

$(result_file): $(xml_file)
	xsltproc --novalid -o $(result_file) $(xsl_file) $(xml_file)

$(BUILDDIR)/%: %.c $(SRC_FILES) $(LIBSRC_FILES) $(TEST_SRC_FILES)
	@mkdir -p $(BUILDDIR)
	$(CC) -o $@ $< $(SRC_FILES) $(LIBSRC_FILES) $(CFLAGS) $(INCDIRS) $(LDFLAGS)

gcov: $(SRC_FILES) $(TEST_SRC_FILES)
	cd $(BUILDDIR) && gcov -b -s../../src $(notdir $(SRC_FILES))

lcov: $(SRC_FILES) $(TEST_SRC_FILES)
	cd $(BUILDDIR) && lcov -c -d . -o coverage.info
	cd $(BUILDDIR) && lcov -r coverage.info -o coverage_filtered.info \
		'/usr/include/*' \
		'$(realpath ../cjson)/*'
	cd $(BUILDDIR) && genhtml coverage_filtered.info

.PHONY: clean all build run gcov lcov

clean:
	-$(RM) -r $(BUILDDIR)
	-$(RM) *.gcda *.gcno *.gcov
