#Makefile for unit tests using Unity framework
SHELL = /usr/bin/bash
CC = gcc
CFLAGS = -std=c99
CFLAGS += -Wall
CFLAGS += -Wextra -Wpedantic
# CFLAGS += -DVS_MSG_DEBUG
INCDIRS = -I../include -I../cjson -I../unity/src
SRC_FILES = ../src/vs_msg.c ../cjson/cJSON.c ../unity/src/unity.c

all: test_vs_msg

test_vs_msg: test_vs_msg.c $(SRC_FILES)
	$(CC) -o $@ $^ $(CFLAGS) $(INCDIRS) $(LDFLAGS)
	./test_vs_msg

.PHONY: clean all

clean:
	-$(RM) -f test_vs_msg
