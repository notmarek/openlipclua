LIBNAME=libopenlipclua
LUADIR?=/usr/include/luajit-2.1

CC?=gcc

SRCS:=openlipclua.c
OBJS:=$(SRCS:.c=.o)

EXTRA_CPPFLAGS:=-I$(LUADIR) -Ilua-compat-5.3/c-api -Iopenlipc/include
EXTRA_CFLAGS:=-fvisibility=hidden -fPIC

%.o: %.c
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<

default: $(LIBNAME).so

# This is stupid. We just want a DT_NEEDED entry set to liblipc.so without having access to the real lib...
liblipc.so:
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -shared -Wl,-soname=$@ -o $@ /dev/null
EXTRA_LDFLAGS:=-L. -Wl,--warn-unresolved-symbols -Wl,--no-as-needed -l:liblipc.so

$(LIBNAME).so: $(OBJS) liblipc.so
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $(LDFLAGS) -shared -Wl,-soname=$@ $(EXTRA_LDFLAGS) -o $@ $<

clean:
	rm -f *.so
	rm -f *.o

.PHONY: default clean
