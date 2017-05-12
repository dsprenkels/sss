CFLAGS = -Wall -g -O2
SRCS = hazmat.c randombytes.c serialize.c sss.c keccak.c tweetnacl.c
OBJS := ${SRCS:.c=.o}

all: libsss.a

libsss.a: $(OBJS)
	$(AR) -rcs libsss.a $^

# Optimize hazmat.c as hard as possible
hazmat.o: CFLAGS += -O3 -funroll-loops

%.out: %.o
	$(CC) -o $@ $(CFLAGS) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS)
	valgrind -q --leak-check=full --error-exitcode=1 ./$@

test_hazmat.out: $(filter-out hazmat.o,$(OBJS))
test_sss.out: $(OBJS)
test_serialize.out: $(OBJS)

.PHONY: test
test: test_hazmat.out test_serialize.out test_sss.out

.PHONY: clean
clean:
	$(RM) *.o *.gch *.a *.out
