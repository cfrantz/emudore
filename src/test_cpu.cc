#include <cstdint>
#include <cstdio>
#include <gflags/gflags.h>

#include "src/cpu2.h"
#include "src/memory.h"

DEFINE_int32(end, 0, "End address");

class Mem: public Memory {
  public:
    Mem() {}

    uint8_t read_byte(uint16_t addr) override { return ram_[addr]; }
    void write_byte(uint16_t addr, uint8_t val) override { ram_[addr] = val; }

    uint8_t read_byte_no_io(uint16_t addr) override { return read_byte(addr); }
    void write_byte_no_io(uint16_t addr, uint8_t val) override { write_byte(addr, val); }

    uint16_t read_word(uint16_t addr) override {
        return read_byte(addr) | read_byte(addr+1) << 8;
    }
    uint16_t read_word_no_io(uint16_t addr) override {
        return read_word(addr);
    }
    void write_word(uint16_t addr, uint16_t val) override {
        write_byte(addr, val); write_byte(addr+1, val>>8);
    }
    void write_word_no_io(uint16_t addr, uint16_t val) override {
        write_word(addr, val);
    }

    void Load(const std::string& file, uint16_t addr) {
        FILE* fp = fopen(file.c_str(), "rb");
        fread(ram_+addr, 1, 65536, fp);
        fclose(fp);
    }
  private:
    uint8_t ram_[64*1024];
};

int main(int argc, char *argv[]) {
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    Mem mem;
    Cpu cpu(&mem);

    mem.Load(argv[1], 0x400);
    cpu.set_pc(0x400);

    for(;;) {
        printf("%04X: %02X %d\n", cpu.pc(), mem.read_byte(cpu.pc()), cpu.cycles());
        cpu.Emulate();
        if (cpu.pc() == FLAGS_end) {
            printf("SUCCESS!\n");
            break;
        }
    }
    return 0;
}
