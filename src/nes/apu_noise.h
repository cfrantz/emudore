#ifndef EMUDORE_SRC_NES_APU_NOISE_H
#define EMUDORE_SRC_NES_APU_NOISE_H
#include <cstdint>
#include "proto/apu.pb.h"

class Noise {
  public:
    Noise();
    uint8_t Output();
    void StepTimer();
    void StepEnvelope();
    void StepLength();

    void set_enabled(bool val);
    void set_control(uint8_t val);
    void set_period(uint8_t val);
    void set_length(uint8_t val);
    inline uint16_t length() const { return length_value_; }
    void DebugStuff();
    void SaveState(proto::APUNoise *state);
    void LoadState(proto::APUNoise *state);
  private:
    uint8_t InternalOutput();
    bool enabled_;
    bool mode_;

    uint16_t shift_register_;

    bool length_enabled_;
    uint8_t length_value_;

    uint16_t timer_period_;
    uint16_t timer_value_;

    bool envelope_enable_;
    bool envelope_start_;
    bool envelope_loop_;
    uint8_t envelope_period_;
    uint8_t envelope_value_;
    uint8_t envelope_volume_;

    uint8_t constant_volume_;

    struct {
        uint8_t control, period, length;
    } reg_;
    const static int DBGBUFSZ = 1024;
    float dbgbuf_[DBGBUFSZ];
    int dbgp_;
};

#endif // EMUDORE_SRC_NES_APU_NOISE_H
