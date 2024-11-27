BASEDIR := /Users/adavari/csc429/llvm-install
BINDIR := $(BASEDIR)/bin

CLANG := $(BINDIR)/clang
OPT := $(BINDIR)/opt
lld := $(BINDIR)/lld

CFLAGS := -isysroot $(shell xcrun --show-sdk-path)

C_SRCS := $(wildcard *.c)

CBINS := $(C_SRCS:%.c=%)
CLLS := $(C_SRCS:%.c=%.c.ll)

all: $(CLLS)

%: %.c
	$(CLANG) $(CFLAGS) -g -o $@ $<

%.c.ll: %.c
	$(CLANG) $(CFLAGS) -S -g -o $@ $<
	$(CLANG) $(CFLAGS) -S -g -emit-llvm -o $@ $<
	$(CLANG) $(CFLAGS) -O1 -g -S -emit-llvm -o $<.o1.ll $<
	$(CLANG) $(CFLAGS) -O2 -g -S -emit-llvm -o $<.o2.ll $<

exec:
	/Users/adavari/csc429/llvm-install/bin/opt -S main.c.ll -passes=taint-analysis -o main.c.instrumented.ll
	/Users/adavari/csc429/llvm-install/bin/clang++ main.c.instrumented.ll shadowlib.ll -L /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/lib

run:
	/Users/adavari/csc429/llvm-install/bin/opt -disable-output main.c.o1.ll -passes=taint-analysis

.PHONY: clean
clean:
	rm -rf *.o main.*.ll *.s a.out $(CBINS)