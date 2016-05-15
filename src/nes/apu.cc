#include <string.h>
#include <SDL2/SDL.h>

#include "src/nes/apu.h"
#include "src/nes/nes.h"


static float pulse_table[32];
static float other_table[204];

static void init_tables() {
    static int once;
    int i;
    if (!once) {
        once = 1;
        for(i=0; i<32; i++) {
            pulse_table[i] = 95.52 / (8128/double(i) + 100);
        }
        for(i=0; i<204; i++) {
            other_table[i] = 163.67 / (24329/double(i) + 100);
        }
    }
}

APU::APU(NES *nes)
    : nes_(nes),
    pulse_({1, 2}),
    cycle_(0),
    frame_period_(0),
    frame_value_(0),
    frame_irq_(0),
    data_{0, },
    len_(0) {
        mutex_ = SDL_CreateMutex();
        init_tables();
}

void APU::StepTimer() {
    if (cycle_ % 2 == 0) {
        pulse_[0].StepTimer();
        pulse_[1].StepTimer();
        noise_.StepTimer();
        dmc_.StepTimer();
    }
    triangle_.StepTimer();
}

void APU::StepEnvelope() {
    pulse_[0].StepEnvelope();
    pulse_[1].StepEnvelope();
    triangle_.StepCounter();
    noise_.StepEnvelope();
}

void APU::StepSweep() {
    pulse_[0].StepSweep();
    pulse_[1].StepSweep();
}

void APU::StepLength() {
    pulse_[0].StepLength();
    pulse_[1].StepLength();
    triangle_.StepLength();
    noise_.StepLength();
}

void APU::SignalIRQ() {
    nes_->IRQ();
}

void APU::StepFrameCounter() {
    if (frame_period_ == 4) {
        frame_value_ = (frame_value_ + 1) % 4;
        StepEnvelope();
        if (frame_value_ & 1) {
            StepSweep();
            StepLength();
            if (frame_value_ == 3)
                SignalIRQ();
        }
    } else {
        frame_value_ = (frame_value_ + 1) % 5;
        if (frame_value_ == 4)
            return;
        StepEnvelope();
        if (!(frame_value_ & 1)) {
            StepSweep();
            StepLength();
        }
    }
}

float APU::Output() {
    uint8_t p0 = pulse_[0].Output();
    uint8_t p1 = pulse_[1].Output();
    uint8_t t = triangle_.Output();
    uint8_t n = noise_.Output();
    uint8_t d = dmc_.Output();
    return pulse_table[p0+p1] + other_table[t*3 + n*2 + d];
}

void APU::Emulate() {
    double c1 = double(cycle_);
    double c2 = double(++cycle_);

    StepTimer();

    int f1 = int(c1 / NES::frame_counter_rate);
    int f2 = int(c2 / NES::frame_counter_rate);
    if (f1 != f2)
        StepFrameCounter();

    int s1 = int(c1 / NES::sample_rate);
    int s2 = int(c2 / NES::sample_rate);
    if (s1 != s2) {
        SDL_LockMutex(mutex_);
        if (len_ < DATALEN) {
            data_[len_++] = Output();
        } else {
            printf("Overrun\n");
        }
        SDL_UnlockMutex(mutex_);
    }
}

void APU::PlayBuffer(uint8_t* stream, int bufsz) {
    int n = bufsz / sizeof(float);
    if (len_ >= n) {
        SDL_LockMutex(mutex_);
        int rest = len_ - n;
        memcpy(stream, data_, bufsz);
        memmove(data_, data_ + n, rest * sizeof(float));
        len_ = rest;
        SDL_UnlockMutex(mutex_);
    } else {
        printf("Underrun\n");
        memset(stream, 0, bufsz);
    }
}

void APU::Write(uint16_t addr, uint8_t val) {
    switch(addr) {
    case 0x4000: pulse_[0].set_control(val); break;
    case 0x4001: pulse_[0].set_sweep(val); break;
    case 0x4002: pulse_[0].set_timer_low(val); break;
    case 0x4003: pulse_[0].set_timer_high(val); break;

    case 0x4004: pulse_[1].set_control(val); break;
    case 0x4005: pulse_[1].set_sweep(val); break;
    case 0x4006: pulse_[1].set_timer_low(val); break;
    case 0x4007: pulse_[1].set_timer_high(val); break;

    case 0x4008: triangle_.set_control(val); break;
    case 0x400a: triangle_.set_timer_low(val); break;
    case 0x400b: triangle_.set_timer_high(val); break;

    case 0x400c: noise_.set_control(val); break;
    case 0x400e: noise_.set_period(val); break;
    case 0x400f: noise_.set_length(val); break;

    case 0x4010: dmc_.set_control(val); break;
    case 0x4011: dmc_.set_value(val); break;
    case 0x4012: dmc_.set_address(val); break;
    case 0x4013: dmc_.set_length(val); break;

    case 0x4015: set_control(val); break;
    case 0x4017: set_frame_counter(val); break;

    default:
        ; // Nothing
    }
}

uint8_t APU::Read(uint16_t addr) {
    uint8_t result = 0;
    if (addr == 0x4015) {
        result |= (pulse_[0].length() > 0) << 0;
        result |= (pulse_[1].length() > 0) << 1;
        result |= (triangle_.length() > 0) << 2;
        result |= (noise_.length() > 0   ) << 3;
        result |= (dmc_.length() > 0      ) << 4;
    }
    return result;
}

void APU::set_frame_counter(uint8_t val) {
    frame_period_ = 4 + ((val >> 7) & 1);
    frame_irq_ = !(val & 0x40);
}

void APU::set_control(uint8_t val) {
    pulse_[0].set_enabled(!!(val & 0x01));
    pulse_[1].set_enabled(!!(val & 0x02));
    triangle_.set_enabled(!!(val & 0x04));
    noise_.set_enabled(!!(val & 0x08));
    dmc_.set_enabled(!!(val & 0x10));
}