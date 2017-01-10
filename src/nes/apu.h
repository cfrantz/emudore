#ifndef EMUDORE_SRC_NES_APU_H
#define EMUDORE_SRC_NES_APU_H
#include <cstdint>
#include <atomic>
#include <SDL2/SDL.h>

#include "src/nes/apu_dmc.h"
#include "src/nes/apu_noise.h"
#include "src/nes/apu_pulse.h"
#include "src/nes/apu_triangle.h"
#include "src/nes/nes.h"

class APU {
  public:
    APU(NES* nes);

    void Write(uint16_t addr, uint8_t val);
    uint8_t Read(uint16_t addr);

    void PlayBuffer(uint8_t* stream, int len);

    void StepEnvelope();
    void StepLength();
    void StepSweep();
    void StepTimer();
    void StepFrameCounter();
    void SignalIRQ();
    float Output();
    void Emulate();
    void DebugStuff();

    static const int BUFFERLEN = 1024;
  private:
    void set_frame_counter(uint8_t val);
    void set_control(uint8_t val);

    NES* nes_;
    Pulse pulse_[2];
    Triangle triangle_;
    Noise noise_;
    DMC dmc_;
    SDL_mutex *mutex_;
    SDL_cond *cond_;

    uint64_t cycle_;
    uint8_t frame_period_;
    uint8_t frame_value_;;
    bool frame_irq_;
    float volume_;

    float data_[BUFFERLEN];
    std::atomic<int> len_;
};

#endif // EMUDORE_SRC_NES_APU_H
