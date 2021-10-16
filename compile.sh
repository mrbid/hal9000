clang hal9000.c -Ofast -lSDL2 -lm -o hal9000
i686-w64-mingw32-gcc -std=c17 hal9000.c -ISDL2/include -LSDL2/lib -Ofast -Wall -lmingw32 -lSDL2main -lSDL2 -o hal9000.exe
upx hal9000
upx hal9000.exe
