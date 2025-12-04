SHELL := /bin/sh
CC := gcc

# Packages we want; you can override PKGS when invoking make
PKGS ?= libmicrohttpd libcurl cjson libsodium

# Probe pkg-config for available cflags/libs (ignore missing packages)
PKG_CFLAGS := $(shell for p in $(PKGS); do pkg-config --cflags $$p 2>/dev/null || true; done)
PKG_LIBS := $(shell for p in $(PKGS); do pkg-config --libs $$p 2>/dev/null || true; done)

CFLAGS := -Wall -O2 $(PKG_CFLAGS)
LDFLAGS := $(PKG_LIBS) -lcjson -lm

SOURCES := main.c teable.c auth.c judge.c utils.c
TARGET := backend

.PHONY: all clean
all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET)