#include "imgui.h"
#include "src/nes/apu_noise.h"

static uint8_t length_table[32] = {
    10, 254, 20,  2, 40,  4, 80,  6, 160,  8, 60, 10, 14, 12, 26, 14,
    12,  16, 24, 18, 48, 20, 96, 22, 192, 24, 72, 26, 16, 28, 32, 30,
};

static uint16_t noise_table[] = {
    4, 8, 16, 32, 64, 96, 128, 160, 202, 254, 380, 508, 762, 1016, 2034, 4068,
};

Noise::Noise()
    : enabled_(false),
    mode_(false),
    shift_register_(1),
    length_enabled_(false),
    length_value_(0),
    timer_period_(0), timer_value_(0),
    envelope_enable_(false), envelope_start_(false), envelope_loop_(false),
    envelope_period_(0), envelope_value_(0), envelope_volume_(0),
    constant_volume_(0) {}

uint8_t Noise::InternalOutput() {
    if (!enabled_) return 0;
    if (length_value_ == 0) return 0;
    if (shift_register_ & 1) return 0;
    if (envelope_enable_) return envelope_volume_;
    return constant_volume_;
}

uint8_t Noise::Output() {
    uint8_t val = InternalOutput();
    dbgbuf_[dbgp_] = val;
    dbgp_ = (dbgp_ + 1) % DBGBUFSZ;
    return val;
}

void Noise::DebugStuff() {
    ImGui::BeginGroup();
    ImGui::PlotLines("", dbgbuf_, DBGBUFSZ, dbgp_, "Noise", 0.0f, 15.0f, ImVec2(0,80));
    ImGui::SameLine();
    ImGui::BeginGroup();
    ImGui::Text("Enabled %s", enabled_ ? "true" : "false");
    ImGui::Text("control: %02x", reg_.control);
    ImGui::Text("period:  %02x", reg_.period);
    ImGui::Text("length:  %02x", reg_.length);
    ImGui::EndGroup();
    ImGui::EndGroup();
}

void Noise::StepTimer() {
    if (timer_value_ == 0) {
        timer_value_ = timer_period_;
        unsigned shift = mode_ ? 6 : 1;
        uint16_t b1 = shift_register_ & 1;
        uint16_t b2 = (shift_register_ >> shift) & 1;
        shift_register_ = (shift_register_ >> 1) | ((b1 ^ b2) << 14);
    } else {
        timer_value_--;
    }
}

void Noise::StepEnvelope() {
    if (envelope_start_) {
        envelope_volume_ = 15;
        envelope_value_ = envelope_period_;
        envelope_start_ = false;
    } else if (envelope_value_ > 0) {
        envelope_value_--;
    } else {
        if (envelope_volume_ > 0) {
            envelope_volume_--;
        } else if (envelope_loop_) {
            envelope_volume_ = 15;
        }
        envelope_value_ = envelope_period_;
    }
}

void Noise::StepLength() {
    if (length_enabled_ && length_value_ > 0) {
        length_value_--;
    }
}

void Noise::set_enabled(bool val) {
    enabled_ = val;
    if (!enabled_)
        length_value_ = 0;
}

void Noise::set_control(uint8_t val) {
    reg_.control = val;
    length_enabled_ = !(val & 0x20);
    envelope_loop_ = !!(val & 0x20);
    envelope_enable_ = !(val & 0x10);
    envelope_period_ = (val & 0x0f);
    constant_volume_ = (val & 0x0f);
    envelope_start_ = true;
}

void Noise::set_period(uint8_t val) {
    reg_.period = val;
    mode_ = !!(val & 0x80);
    timer_period_ = noise_table[val & 0x0f];
}

void Noise::set_length(uint8_t val) {
    reg_.length = val;
    length_value_ = length_table[val >> 3];
    envelope_start_ = true;
}
