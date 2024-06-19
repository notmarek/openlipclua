
KINDLE_ROOTDIR=/home/bob/kindlere/KindlePW5-5169/rootfs

${CROSS_TC}-gcc -c -Ilua-compat-5.3/c-api -Iinclude -fPIC library.c -o liblualipc.o 
${CROSS_TC}-gcc liblualipc.o -shared -o liblualipc.so  -L${KINDLE_ROOTDIR}/usr/lib -Wl,-rpath=${KINDLE_ROOTDIR}/usr/lib -llipc -lglib-2.0 -ldbus-1 -lgthread-2.0 -lcjson
