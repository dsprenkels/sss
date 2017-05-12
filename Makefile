CFLAGS = -Wall -g -O3
SRCS = hazmat.c randombytes.c serialize.c sss.c keccak.c tweetnacl.c
OBJS := ${SRCS:.c=.o}

all: libsss.a

libsss.a: $(OBJS)
	$(AR) -rcs libsss.a $^

%.out: %.o
	$(CC) -o $@ $(CFLAGS) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS)
	$(MEMCHECK) ./$@

test_hazmat.out: $(filter-out hazmat.o,$(OBJS))
test_sss.out: $(OBJS)
test_serialize.out: $(OBJS)

.PHONY: test
test: test_hazmat.out test_serialize.out test_sss.out

.PHONY: clean
clean:
	$(RM) *.o *.gch *.a *.out
