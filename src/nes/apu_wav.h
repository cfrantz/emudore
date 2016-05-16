#ifndef EMUDORE_SRC_NES_APU_WAV_H
#define EMUDORE_SRC_NES_APU_WAV_H
#include <cstdint>
#include "src/sdlutil/audiosample.h"

class Wave {
  public:
    Wave(uint8_t channel);
    float Output();
    void Sweep();
    void StepTimer();
    void StepEnvelope();
    void StepSweep();
    void StepLength();

    void set_enabled(bool val);
    void set_control(uint8_t val);
    void set_sweep(uint8_t val);
    void set_timer_low(uint8_t val);
    void set_timer_high(uint8_t val);
    inline uint16_t length() const { return length_value_; }
    inline void Load(const std::string& filename) {
        instrument_.Load(filename);
    }
    inline void SetBaseFrequency(const std::string& notename) {
        instrument_.set_base_frequency(notename);
    }
  private:
    bool enabled_;
    uint8_t channel_;

    bool length_enabled_;
    uint8_t length_value_;

    uint16_t timer_period_;
    uint16_t timer_value_;

    uint8_t duty_mode_;
    uint8_t duty_value_;

    bool sweep_enable_;
    bool sweep_reload_;
    bool sweep_negate_;
    uint8_t sweep_shift_;
    uint8_t sweep_period_;
    uint8_t sweep_value_;

    bool envelope_enable_;
    bool envelope_start_;
    bool envelope_loop_;
    uint8_t envelope_period_;
    uint8_t envelope_value_;
    uint8_t envelope_volume_;

    uint8_t constant_volume_;
    sdlutil::AudioSampleInstrument instrument_;

};

#endif // EMUDORE_SRC_NES_APU_WAV_H
