CFLAGS += -g -O2 -m64 -std=c99 -pedantic \
	-Wall -Wshadow -Wpointer-arith -Wcast-qual -Wformat -Wformat-security \
	-Werror=format-security -Wstrict-prototypes -Wmissing-prototypes \
	-D_FORTIFY_SOURCE=2 -fPIC -fno-strict-overflow
SRCS = hazmat.c randombytes.c sss.c tweetnacl.c
OBJS := ${SRCS:.c=.o}
UNAME_S := $(shell uname -s)

all: libsss.a

libsss.a: randombytes/librandombytes.a $(OBJS)
    ifeq ($(UNAME_S),Linux)
		$(AR) -rcs libsss.a $^
    endif
    ifeq ($(UNAME_S),Darwin)
		libtool -static -o libsss.a $^
    endif

randombytes/librandombytes.a:
	$(MAKE) -C randombytes librandombytes.a

# Force unrolling loops on hazmat.c
hazmat.o: CFLAGS += -funroll-loops

%.out: %.o randombytes/librandombytes.a
	$(CC) -o $@ $(CFLAGS) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS)
	$(MEMCHECK) ./$@

test_hazmat.out: $(OBJS)
test_sss.out: $(OBJS)

.PHONY: check
check: test_hazmat.out test_sss.out

.PHONY: clean
clean:
	$(MAKE) -C randombytes $@
	$(RM) *.o *.gch *.a *.out
