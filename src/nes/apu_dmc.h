#ifndef EMUDORE_SRC_NES_APU_DMC_H
#define EMUDORE_SRC_NES_APU_DMC_H
#include <cstdint>

class DMC {
  public:
    DMC();
    
    uint8_t Output();
    void StepReader();
    void StepShifter();
    void StepTimer();

    void set_enabled(bool val);
    void set_control(uint8_t val);
    void set_value(uint8_t val);
    void set_address(uint8_t val);
    void set_length(uint8_t val);
    inline uint16_t length() const { return current_length_; }
    void restart();
  private:
    bool enabled_;
    uint8_t value_;

    uint16_t sample_address_;
    uint16_t sample_length_;
    uint16_t current_address_;
    uint16_t current_length_;

    uint8_t shift_register_;
    uint8_t bit_count_;
    uint8_t tick_value_;
    uint8_t tick_period_;

    bool loop_;
    bool irq_;
};

#endif // EMUDORE_SRC_NES_APU_DMC_H
