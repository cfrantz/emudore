#ifndef EMUDORE_SRC_NES_NES_H
#define EMUDORE_SRC_NES_NES_H
#include <string>
#include <map>
#include <SDL2/SDL.h>
#include "src/io.h"
#include "src/nes/debug_console.h"

class APU;
class Cpu;
class Cartridge;
class Controller;
class Debugger;
class FM2Movie;
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
    inline int controller_size() const {
        return int(sizeof(controller_) / sizeof(controller_[0]));
    }
    inline APU* apu() { return apu_; }
    inline Cpu* cpu() { return cpu_; }
    inline IO* io() { return io_; }
    inline Mapper* mapper() { return mapper_; }
    inline Mem* memory() { return mem_; }
    inline FM2Movie* movie() { return movie_; }
    inline PPU* ppu() { return ppu_; }
    inline uint32_t palette(uint8_t c) { return palette_[c % 64]; }

    int cpu_cycles();
    inline void yield() const { io_->yield(); }
    inline void Stall(int s) { stall_ += s; }

    static const int frequency = 1789773;
    static constexpr double frame_counter_rate = frequency / 240.0;
    static constexpr double sample_rate = frequency / 44100.0;
  private:
    void DebugStuff(SDL_Renderer* r);
    void DebugPalette(bool* active);
    void HandleKeyboard(SDL_Event* event);
    APU* apu_;
    Cpu *cpu_;
    Cartridge* cart_;
    Controller* controller_[4];
    Debugger* debugger_;
    IO* io_;
    Mapper* mapper_;
    Mem* mem_;
    FM2Movie* movie_;
    PPU* ppu_;

    uint32_t palette_[64];
    bool pause_, step_, debug_, reset_;
    int stall_;
    int frame_;

    DebugConsole console_;
    std::map<uint16_t, uint8_t> nailed_;
    void HexdumpBytes(int argc, char **argv);
    void HexdumpWords(int argc, char **argv);
    void WriteBytes(int argc, char **argv);
    void WriteWords(int argc, char **argv);

    void NailByte(int argc, char **argv);
    void UnnailByte(int argc, char **argv);
};

#endif // EMUDORE_SRC_NES_NES_H


