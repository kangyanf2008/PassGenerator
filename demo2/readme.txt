编译动态库
gcc -c -fPIC pwKey.c -o pwKey.o -I. -L.  -lWibuCm64
gcc -c -fPIC pwKey.c -o pwKey.dll -I. -L.  -lWibuCm64
生成动态连接库
gcc -shared  -o libpwKey.so pwKey.o -I. -L. -lWibuCm64