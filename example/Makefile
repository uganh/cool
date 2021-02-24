CC       = gcc
CXX      = g++
CFLAGS   = -g
CXXFLAGS = $(CFLAGS)

cgen: x86codegen.cc
	${CXX} ${CXXFLAGS} -o $@ $<

fib.s: cgen fib.in
	./cgen < fib.in > fib.s

wrap_fib: fib.s wrap_fib.c
	${CC} ${CFLAGS} -o $@ $^

clean:
	rm -f cgen fib.[so] wrap_fib

.PHONY: clean
