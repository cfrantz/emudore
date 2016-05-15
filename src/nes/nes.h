#ifndef EMUDORE_SRC_NES_NES_H
#define EMUDORE_SRC_NES_NES_H
#include <string>
#include "src/io.h"

class APU;
class Cpu;
class Cartridge;
class Controller;
class Debugger;
class Mapper;
class Mem;
class PPU;

class NES {
  public:
    NES();
    void LoadFile(const std::string& filename);
    void Run();
    void IRQ();
    void NMI();

    inline Cartridge* cartridge() { return cart_; }
    inline Controller* controller(int n) { return controller_[n]; }
    inline APU* apu() { return apu_; }
    inline IO* io() { return io_; }
    inline Mapper* mapper() { return mapper_; }
    inline Mem* memory() { return mem_; }
    inline PPU* ppu() { return ppu_; }
    inline uint32_t palette(uint8_t c) { return palette_[c % 64]; }

    inline void yield() const { io_->yield(); }

    static const int frequency = 1789773;
    static constexpr double frame_counter_rate = frequency / 240.0;
    static constexpr double sample_rate = frequency / 44100.0;
  private:
    APU* apu_;
    Cpu *cpu_;
    Cartridge* cart_;
    Controller* controller_[2];
    Debugger* debugger_;
    IO* io_;
    Mapper* mapper_;
    Mem* mem_;
    PPU* ppu_;

    uint32_t palette_[64];
};

#endif // EMUDORE_SRC_NES_NES_H


