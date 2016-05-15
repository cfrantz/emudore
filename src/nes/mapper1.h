#ifndef EMUDORE_SRC_NES_MAPPER1_H
#define EMUDORE_SRC_NES_MAPPER1_H
#include <cstdint>

#include "src/nes/mapper.h"

class Mapper1: public Mapper {
  public:
    Mapper1(NES* nes);
    uint8_t Read(uint16_t addr) override;
    void Write(uint16_t addr, uint8_t val) override;
    void Emulate() override;

  private:
    int PrgBankOffset(int index);
    int ChrBankOffset(int index);
    void WriteControl(uint8_t val);
    void LoadRegister(uint16_t addr, uint8_t val);
    void WriteRegister(uint16_t addr, uint8_t val);
    void UpdateOffsets();

    uint8_t shift_register_;
    uint8_t control_;
    uint8_t prg_mode_;
    uint8_t chr_mode_;
    uint8_t prg_bank_;
    uint8_t chr_bank0_;
    uint8_t chr_bank1_;

    int prg_offset_[2];
    int chr_offset_[2];
};

#endif // EMUDORE_SRC_NES_MAPPER1_H
