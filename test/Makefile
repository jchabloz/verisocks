#Makefile for unit tests using CUnit framework
SHELL = /usr/bin/bash
CC = gcc
CFLAGS += -Wall
CFLAGS += -Wextra -Wpedantic
CFLAGS += --coverage
CFLAGS += -g -O1
CFLAGS += -DVS_LOG_LEVEL=100
LDFLAGS += -lcunit

BUILDDIR = build
INCDIRS = -I../include -I../cjson
SRC_FILES = ../src/vs_msg.c ../src/vs_server.c
TEST_SRC_FILES = src/test_vs_msg.c src/test_vs_server.c
LIBSRC_FILES = ../cjson/cJSON.c

xml_file = $(BUILDDIR)/CUnitAutomated-Results.xml
xsl_file = /usr/share/CUnit/CUnit-Run.xsl
result_file = $(BUILDDIR)/cunit_test_results.html

tests = cunit_test

# Intermediate files needed for coverage analysis
gcno_files = $(addprefix $(BUILDDIR)/,$(patsubst %.c,%.gcno,$(notdir $(SRC_FILES) $(LIBSRC_FILES))))
gcno_files += $(addprefix $(BUILDDIR)/,$(patsubst %,%.gcno,$(tests)))
gcda_files = $(addprefix $(BUILDDIR)/,$(patsubst %.c,%.gcda,$(notdir $(SRC_FILES) $(LIBSRC_FILES))))
gcda_files += $(addprefix $(BUILDDIR)/,$(patsubst %,%.gcda,$(tests)))
run_rpt_files += $(addprefix $(BUILDDIR)/,$(patsubst %,%_valgrind.rpt,$(tests)))

all: build run

build: $(addprefix $(BUILDDIR)/,$(tests))

run: $(run_rpt_files) $(result_file) $(gcno_files) $(gcda_files)

lcov: run $(BUILDDIR)/lcov/index.html

$(BUILDDIR)/%: src/%.c $(SRC_FILES) $(LIBSRC_FILES) $(TEST_SRC_FILES)
	@mkdir -p $(BUILDDIR)
	$(CC) -o $@ $< $(SRC_FILES) $(LIBSRC_FILES) $(CFLAGS) $(INCDIRS) $(LDFLAGS)

$(BUILDDIR)/%_valgrind.rpt: $(BUILDDIR)/%
	cd $(BUILDDIR) && valgrind --leak-check=full --log-file=$(notdir $@) ./$(notdir $<)

$(result_file): $(xsl_file) $(xml_file)
	xsltproc --novalid -o $@ $^

$(gcno_files) $(gcda_files):
	-mv -u $(notdir $@) $(BUILDDIR)

$(BUILDDIR)/lcov/index.html: $(SRC_FILES) $(TEST_SRC_FILES) $(gcda_files) $(gcno_files)
	cd $(BUILDDIR) && lcov -c -d . -o coverage.info
	cd $(BUILDDIR) && lcov -r coverage.info -o coverage_filtered.info \
		'/usr/include/*' \
		'$(realpath ../cjson)/*'
	cd $(BUILDDIR) && genhtml -o lcov coverage_filtered.info

.PHONY: clean all build run lcov

clean:
	-$(RM) -r $(BUILDDIR)
	-$(RM) *.gcda *.gcno *.gcov
