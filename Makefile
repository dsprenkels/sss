CC = clang
CFLAGS = -Wall -g -O2 -ansi -pedantic
SRCS = sss.c hazmat.c
OBJS := ${SRCS:.c=.o}

all: libsss.a

libsss.a: $(OBJS)
	$(AR) -rcs libsss.a $^

.PHONY: clean
clean:
	rm -f *.o *.gch *.a *.out
