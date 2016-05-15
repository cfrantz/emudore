#include <cstdio>

#include "src/nes/nes.h"
#include "src/nes/mem.h"

int main(int argc, char *argv[]) {
    NES a;

    a.LoadFile(argv[1]);
    printf("Reset vector = %04x\n", a.memory()->read_word(0xFFFE));
    a.Run();
    return 0;
}
