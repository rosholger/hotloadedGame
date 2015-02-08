#clang -o dynamicTestMain dynamicTestMain.c -ldl -std=gnu99 -I/usr/include/SDL2 -D_REENTRANT -L/usr/lib/x86_64-linux-gnu -lSDL2
#clang -shared -fPIC -o dynamicTestSo.so dynamicTestSo.c && echo 1 >> file
tup && echo 1 > file