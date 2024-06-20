${CROSS_TC}-gcc ${CFLAGS} -fvisibility=hidden -c -Ilua-compat-5.3/c-api -I/usr/include/luajit-2.1 -Iopenlipc/include -fPIC openlipclua.c -o libopenlipclua.o

# This is stupid. I just want a DT_NEEDED entry set to liblipc.so without having access to the real lib!
${CROSS_TC}-gcc ${CFLAGS} ${LDFLAGS} -shared -Wl,-soname=liblipc.so /dev/null -o liblipc.so
${CROSS_TC}-gcc ${CFLAGS} ${LDFLAGS} libopenlipclua.o -shared -o libopenlipclua.so -Wl,--warn-unresolved-symbols -Wl,--no-as-needed -L. -l:liblipc.so