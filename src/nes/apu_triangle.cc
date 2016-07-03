#include "imgui.h"
#include "src/nes/apu_triangle.h"

static uint8_t length_table[32] = {
    10, 254, 20,  2, 40,  4, 80,  6, 160,  8, 60, 10, 14, 12, 26, 14,
    12,  16, 24, 18, 48, 20, 96, 22, 192, 24, 72, 26, 16, 28, 32, 30,
};

static uint8_t triangle_table[32] = {
    15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
    0, 1, 2, 3, 4, 5, 6, 7, 8 ,9, 10, 11, 12, 13, 14, 15,
};

Triangle::Triangle()
    : enabled_(false),
    length_enabled_(false),
    length_value_(0),
    timer_period_(0), timer_value_(0),
    duty_value_(0),
    counter_reload_(false),
    counter_period_(0), counter_value_(0)
{}

uint8_t Triangle::InternalOutput() {
    if (!enabled_) return 0;
    if (length_value_ == 0) return 0;
    if (counter_value_ == 0) return 0;
    return triangle_table[duty_value_];
}

uint8_t Triangle::Output() {
    uint8_t val = InternalOutput();
    dbgbuf_[dbgp_] = val;
    dbgp_ = (dbgp_ + 1) % DBGBUFSZ;
    return val;
}

void Triangle::DebugStuff() {
    ImGui::BeginGroup();
    ImGui::PlotLines("", dbgbuf_, DBGBUFSZ, dbgp_, "Triangle", 0.0f, 15.0f, ImVec2(0,80));
    ImGui::SameLine();
    ImGui::BeginGroup();
    ImGui::Text("Enabled %s", enabled_ ? "true" : "false");
    ImGui::Text("control: %02x", reg_.control);
    ImGui::Text("timer:   %02x%02x", reg_.thi, reg_.tlo);
    ImGui::EndGroup();
    ImGui::EndGroup();
}

void Triangle::StepTimer() {
    if (timer_value_ == 0) {
        timer_value_ = timer_period_;
        if (length_value_ > 0 && counter_value_ > 0)
            duty_value_ = (duty_value_ + 1) % 32;
    } else {
        timer_value_--;
    }
}

void Triangle::StepLength() {
    if (length_enabled_ && length_value_ > 0)
        length_value_--;
}

void Triangle::StepCounter() {
    if (counter_reload_) {
        counter_value_ = counter_period_;
    } else if (counter_value_ > 0) {
        counter_value_--;
    }
    if (length_enabled_)
        counter_reload_ = false;
}

void Triangle::set_enabled(bool val) {
    enabled_ = val;
    if (!enabled_)
        length_value_ = 0;
}

void Triangle::set_control(uint8_t val) {
    reg_.control = val;
    length_enabled_ = !(val & 0x80);
    counter_period_ = val & 0x7f;
}

void Triangle::set_timer_low(uint8_t val) {
    reg_.tlo = val;
    timer_period_ = (timer_period_ & 0xFF00) | val;
}

void Triangle::set_timer_high(uint8_t val) {
    reg_.thi = val;
    length_value_ = length_table[val >> 3];
    timer_period_ = (timer_period_ & 0x00FF) | (uint16_t(val & 0x07) << 8);
    timer_value_ = timer_period_;
    counter_reload_ = true;
}
