CC ?= cc
CFLAGS ?= -O2 -Wall -Wextra -Wpedantic -Iinclude
LDFLAGS ?=
LDLIBS ?= -lssl -lcrypto

TARGET = bannerhunter
SRC = src/main.c src/ports.c src/net.c src/tls.c src/probe.c src/scan.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(SRC) $(LDLIBS)

clean:
	rm -f $(TARGET)

.PHONY: all clean
