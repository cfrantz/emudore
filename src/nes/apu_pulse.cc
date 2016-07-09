#include "imgui.h"
#include "src/nes/apu_pulse.h"

#include <cstdint>

/*
 * Implementation of the NES APU Pulse channel.  This is basically a direct port
 * of the Nimes Pulse implementation.
 */

static uint8_t duty_table[4][8] = {
    { 0, 1, 0, 0, 0, 0, 0, 0 },
    { 0, 1, 1, 0, 0, 0, 0, 0 },
    { 0, 1, 1, 1, 1, 0, 0, 0 },
    { 1, 0, 0, 1, 1, 1, 1, 1 },
};

static uint8_t length_table[32] = {
    10, 254, 20,  2, 40,  4, 80,  6, 160,  8, 60, 10, 14, 12, 26, 14,
    12,  16, 24, 18, 48, 20, 96, 22, 192, 24, 72, 26, 16, 28, 32, 30,
};

Pulse::Pulse(uint8_t channel)
    : enabled_(false),
    channel_(channel),
    length_enabled_(false), length_value_(0),
    timer_period_(0), timer_value_(0),
    duty_mode_(0), duty_value_(0),
    sweep_enable_(false), sweep_reload_(false), sweep_negate_(false),
    sweep_shift_(0), sweep_period_(0), sweep_value_(0),
    envelope_enable_(false), envelope_start_(false), envelope_loop_(false),
    envelope_period_(0), envelope_value_(0), envelope_volume_(0),
    constant_volume_(0),
    dbgp_(0) {}

uint8_t Pulse::InternalOutput() {
    if (!enabled_) return 0;
    if (length_value_ == 0) return 0;
    if (duty_table[duty_mode_][duty_value_] == 0) return 0;
    if (timer_period_ < 8 || timer_period_ > 0x7ff) return 0;

    if (envelope_enable_) return envelope_volume_;
    return constant_volume_;
}

uint8_t Pulse::Output() {
    uint8_t val = InternalOutput();
    dbgbuf_[dbgp_] = val;
    dbgp_ = (dbgp_ + 1) % DBGBUFSZ;
    return val;
}

void Pulse::DebugStuff() {
    ImGui::BeginGroup();
    ImGui::PlotLines("", dbgbuf_, DBGBUFSZ, dbgp_, "Pulse", 0.0f, 15.0f, ImVec2(0,80));
    ImGui::SameLine();
    ImGui::BeginGroup();
    ImGui::Text("Enabled %s", enabled_ ? "true" : "false");
    ImGui::Text("control: %02x", reg_.control);
    ImGui::Text("sweep:   %02x", reg_.sweep);
    ImGui::Text("timer:   %02x%02x", reg_.thi, reg_.tlo);
    ImGui::EndGroup();
    ImGui::EndGroup();
}

void Pulse::Sweep() {
    uint16_t delta = timer_period_ >> sweep_shift_;
    if (sweep_negate_) {
        timer_period_ -= delta;
        if (channel_ == 1)
            timer_period_--;
    } else {
        timer_period_ += delta;
    }
}

void Pulse::StepTimer() {
    if (timer_value_ == 0) {
        timer_value_ = timer_period_;
        duty_value_ = (duty_value_ + 1) % 8;
    } else {
        timer_value_--;
    }
}

void Pulse::StepEnvelope() {
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

void Pulse::StepSweep() {
    if (sweep_reload_) {
        if (sweep_enable_ && sweep_value_ == 0)
            Sweep();
        sweep_value_ = sweep_period_;
        sweep_reload_ = false;
    } else if (sweep_value_ > 0) {
        sweep_value_--;
    } else {
        if (sweep_enable_)
            Sweep();
        sweep_value_ = sweep_period_;
    }
}

void Pulse::StepLength() {
    if (length_enabled_ && length_value_ > 0)
        length_value_--;
}

void Pulse::set_enabled(bool val) {
    enabled_ = val;
    if (!enabled_)
        length_value_ = 0;
}

void Pulse::set_control(uint8_t val) {
    reg_.control = val;
    duty_mode_ = (val >> 6) & 3;
    length_enabled_ = !(val & 0x20);
    envelope_loop_ = !!(val & 0x20);
    envelope_enable_ = !(val & 0x10);
    envelope_period_ = val & 0x0f;
    constant_volume_ = val & 0x0f;
    envelope_start_ = true;
}

void Pulse::set_sweep(uint8_t val) {
    reg_.sweep = val;
    sweep_enable_ = !!(val & 0x80);
    sweep_period_ = (val >> 4) & 0x07;
    sweep_negate_ = !!(val & 0x08);
    sweep_shift_ = (val & 0x07);
    sweep_reload_ = true;
}

void Pulse::set_timer_low(uint8_t val) {
    reg_.tlo = val;
    timer_period_ = (timer_period_ & 0xFF00) | val;
}

void Pulse::set_timer_high(uint8_t val) {
    reg_.thi = val;
    length_value_ = length_table[val >> 3];
    timer_period_ = (timer_period_ & 0x00FF) | (uint16_t(val & 0x07) << 8);
    envelope_start_ = true;
    duty_value_ = 0;
}
