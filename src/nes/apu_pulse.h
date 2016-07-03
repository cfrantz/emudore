#ifndef EMUDORE_SRC_NES_APU_PULSE_H
#define EMUDORE_SRC_NES_APU_PULSE_H
#include <cstdint>

class Pulse {
  public:
    Pulse(uint8_t channel);
    uint8_t Output();
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
    void DebugStuff();
  private:
    uint8_t InternalOutput();
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

    struct {
        uint8_t control, sweep, tlo, thi;
    } reg_;
    const static int DBGBUFSZ = 1024;
    float dbgbuf_[DBGBUFSZ];
    int dbgp_;
};

#endif // EMUDORE_SRC_NES_APU_PULSE_H
