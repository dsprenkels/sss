CFLAGS = -Wall -g -O2 -pedantic
SRCS = hazmat.c randombytes.c sss.c tweetnacl.c
OBJS := ${SRCS:.c=.o}

all: libsss.a

libsss.a: $(OBJS)
	$(AR) -rcs libsss.a $^

%.out: %.o
	$(CC) -o $@ $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS)
	./$@

test: test_hazmat.out

.PHONY: clean
clean:
	$(RM) *.o *.gch *.a *.out
