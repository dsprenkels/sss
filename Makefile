CC = clang
CFLAGS = -Wall -g -O2 -pedantic
SRCS = sss.c hazmat.c tweetnacl.c
OBJS := ${SRCS:.c=.o}

all: libsss.a test

libsss.a: $(OBJS)
	$(AR) -rcs libsss.a $^

%.o: tweetnacl.h %.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $^

tweetnacl.c: tweetnacl.h
	wget -q https://tweetnacl.cr.yp.to/20140427/tweetnacl.c

tweetnacl.h:
	wget -q https://tweetnacl.cr.yp.to/20140427/tweetnacl.h

%.out: %.o
	$(CC) -o $@ $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS)
	./$@

test: test_hazmat.out

.PHONY: clean
clean:
	$(RM) *.o *.gch *.a *.out tweetnacl.c tweetnacl.h
