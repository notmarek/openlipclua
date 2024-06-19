${CROSS_TC}-gcc ${CFLAGS} -fvisibility=hidden -c -Ilua-compat-5.3/c-api -I/usr/include/luajit-2.1 -Iopenlipc/include -fPIC lualipc.c -o liblualipc.o

# This is stupid. I just want a DT_NEEDED entry set to liblipc.so without having access to the real lib!
touch foo.c
${CROSS_TC}-gcc ${CFLAGS} ${LDFLAGS} -shared -Wl,-soname=liblipc.so foo.c -o liblipc.so
rm foo.c

${CROSS_TC}-gcc ${CFLAGS} ${LDFLAGS} liblualipc.o -shared -o liblualipc.so -Wl,--warn-unresolved-symbols -Wl,--no-as-needed -L. -l:liblipc.so