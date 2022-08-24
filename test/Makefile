#Makefile for unit tests using Unity framework
SHELL = /usr/bin/bash
CC = gcc
CFLAGS += -Wall
CFLAGS += -Wextra -Wpedantic
# CFLAGS += -DVS_MSG_DEBUG
INCDIRS = -I../include -I../cjson -I../unity/src
SRC_FILES = ../src/vs_msg.c ../cjson/cJSON.c ../unity/src/unity.c

all: run

test_vs_msg: test_vs_msg.c $(SRC_FILES)
	$(CC) -o $@ $^ $(CFLAGS) $(INCDIRS) $(LDFLAGS)

sandbox: sandbox.c
	$(CC) -o $@ $^ $(CFLAGS) $(INCDIRS) $(LDFLAGS)
	./sandbox

run: test_vs_msg
	./test_vs_msg

.PHONY: clean all run

clean:
	-$(RM) -f test_vs_msg
	-$(RM) -f sandbox
