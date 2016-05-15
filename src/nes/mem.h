#ifndef EMUDORE_SRC_NES_MEM_H
#define EMUDORE_SRC_NES_MEM_H

#include "src/memory.h"
#include "src/nes/nes.h"

class Mem: public Memory {
  public:
    Mem(NES* nes);

    inline uint8_t Read(uint16_t addr) { return read_byte(addr); }
    inline void Write(uint16_t addr, int val) { return write_byte(addr, val); }

    uint8_t read_byte(uint16_t addr) override;
    uint8_t read_byte_no_io(uint16_t addr) override;
    void write_byte(uint16_t addr, uint8_t v) override;
    void write_byte_no_io(uint16_t addr, uint8_t v) override;
    uint16_t read_word(uint16_t addr) override;
    uint16_t read_word_no_io(uint16_t) override;
    void write_word(uint16_t addr, uint16_t v) override;
    void write_word_no_io(uint16_t addr, uint16_t v) override;

    uint8_t ReadPPU(uint16_t addr);
    void WritePPU(uint16_t addr, uint8_t val);
    uint8_t ReadPalette(uint16_t addr);
    void WritePalette(uint16_t addr, uint8_t val);
  private:
    uint16_t MirrorAddress(int mode, uint16_t addr);

    NES* nes_;
    uint8_t ram_[2048];
    uint8_t ppuram_[2048];
    uint8_t palette_[32];
};

#endif // EMUDORE_SRC_NES_MEM_H
