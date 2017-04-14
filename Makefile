CFLAGS = -Wall -g -O2 -pedantic
SRCS = hazmat.c randombytes.c sss.c keccak.c tweetnacl.c
OBJS := ${SRCS:.c=.o}

all: libsss.a

libsss.a: $(OBJS)
	$(AR) -rcs libsss.a $^

%.out: %.o
	$(CC) -o $@ $(CFLAGS) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS)
	valgrind -q --leak-check=full --error-exitcode=1 ./$@

test_sss.out: $(OBJS)
test_hazmat.out: $(filter-out hazmat.o,$(OBJS))

.PHONY: test
test: test_hazmat.out test_sss.out

.PHONY: clean
clean:
	$(RM) *.o *.gch *.a *.out
