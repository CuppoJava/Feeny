mkdir -p bin
mkdir -p build
stanza build feeny
gcc -O3 src/cfeeny.c src/utils.c src/bytecode.c src/vm.c src/ast.c -o bin/cfeeny -Wno-int-to-void-pointer-cast
