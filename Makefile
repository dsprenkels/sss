CFLAGS = -Wall -g -O2
SRCS = hazmat.c serialize.c sss.c keccak.c tweetnacl.c
OBJS = ${SRCS:.c=.o}
LDFLAGS = -L./randombytes
LDLIBS = -lrandombytes

all: libsss.a

libsss.a: randombytes/librandombytes.a $(OBJS)
	$(AR) -rcs libsss.a $^

randombytes/librandombytes.a:
	$(MAKE) -C randombytes librandombytes.a

# Force unrolling loops on hazmat.c
hazmat.o: CFLAGS += -funroll-loops

%.out: %.o randombytes/librandombytes.a
	$(CC) -o $@ $(CFLAGS) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS)
	$(MEMCHECK) ./$@

test_hazmat.out: $(filter-out hazmat.o,$(OBJS))
test_sss.out: $(OBJS)
test_serialize.out: $(OBJS)

.PHONY: test
test: test_hazmat.out test_serialize.out test_sss.out

.PHONY: clean
clean:
	$(MAKE) -C randombytes $@
	$(RM) *.o *.gch *.a *.out
