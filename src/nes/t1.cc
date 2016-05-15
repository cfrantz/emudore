#include <cstdio>
#include <gflags/gflags.h>

#include "src/nes/nes.h"
#include "src/nes/mem.h"

int main(int argc, char *argv[]) {
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    NES a;

    a.LoadFile(argv[1]);
    printf("Reset vector = %04x\n", a.memory()->read_word(0xFFFC));
    a.Run();
    return 0;
}
