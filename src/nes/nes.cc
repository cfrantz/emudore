#include <string.h>
#include <gflags/gflags.h>

#include "src/nes/nes.h"

#include "src/cpu.h"
#include "src/debugger.h"
#include "src/io.h"
#include "src/nes/apu.h"
#include "src/nes/cartridge.h"
#include "src/nes/controller.h"
#include "src/nes/fm2.h"
#include "src/nes/gfx.h"
#include "src/nes/mapper.h"
#include "src/nes/mem.h"
#include "src/nes/ppu.h"

DEFINE_string(fm2, "", "FM2 Movie file.");

const uint32_t standard_palette[] = {
    0xFF666666, 0xFF002A88, 0xFF1412A7, 0xFF3B00A4,
    0xFF5C007E, 0xFF6E0040, 0xFF6C0600, 0xFF561D00,
    0xFF333500, 0xFF0B4800, 0xFF005200, 0xFF004F08,
    0xFF00404D, 0xFF000000, 0xFF000000, 0xFF000000,
    0xFFADADAD, 0xFF155FD9, 0xFF4240FF, 0xFF7527FE,
    0xFFA01ACC, 0xFFB71E7B, 0xFFB53120, 0xFF994E00,
    0xFF6B6D00, 0xFF388700, 0xFF0C9300, 0xFF008F32,
    0xFF007C8D, 0xFF000000, 0xFF000000, 0xFF000000,
    0xFFFFFEFF, 0xFF64B0FF, 0xFF9290FF, 0xFFC676FF,
    0xFFF36AFF, 0xFFFE6ECC, 0xFFFE8170, 0xFFEA9E22,
    0xFFBCBE00, 0xFF88D800, 0xFF5CE430, 0xFF45E082,
    0xFF48CDDE, 0xFF4F4F4F, 0xFF000000, 0xFF000000,
    0xFFFFFEFF, 0xFFC0DFFF, 0xFFD3D2FF, 0xFFE8C8FF,
    0xFFFBC2FF, 0xFFFEC4EA, 0xFFFECCC5, 0xFFF7D8A5,
    0xFFE4E594, 0xFFCFEF96, 0xFFBDF4AB, 0xFFB3F3CC,
    0xFFB5EBF2, 0xFFB8B8B8, 0xFF000000, 0xFF000000,
};

NES::NES() :
    pause_(false),
    step_(false),
    debug_(false),
    stall_(0)
{
    cpu_ = new Cpu();
    cart_ = new Cartridge(this);
    controller_[0] = new Controller(this);
    controller_[1] = new Controller(this);
    controller_[2] = new Controller(this);
    controller_[3] = new Controller(this);
    apu_ = new APU(this);
    mapper_ = nullptr;
    mem_ = new Mem(this);
    movie_ = new FM2Movie(this);
    ppu_ = new PPU(this);
    io_ = new IO(256, 240, 60.0988);

    io_->init_audio(44100, 1, APU::BUFFERLEN/2, AUDIO_F32,
            [this](uint8_t* stream, int len) {
                apu_->PlayBuffer(stream, len); });
    io_->init_controllers(
            [this](SDL_Event* event) { controller_[0]->set_buttons(event); });
    io_->set_refresh_callback([this](SDL_Renderer* r) { DebugStuff(r); });
    io_->set_keyboard_callback(
            [this](SDL_Event* event) { HandleKeyboard(event); });
    debugger_ = new Debugger();
    debugger_->cpu(cpu_);
    debugger_->memory(mem_);
    cpu_->memory(mem_);
    memcpy(palette_, standard_palette, sizeof(palette_));
}

void NES::LoadFile(const std::string& filename) {
    cart_->LoadFile(filename);
    mapper_ = MapperRegistry::New(this, cart_->mapper());
    if (!FLAGS_fm2.empty()) {
        movie_->Load(FLAGS_fm2);
    }
}

void NES::DebugStuff(SDL_Renderer* r) {
    if (!debug_)
        return;

    int x0=4, y0 = 4;
    uint8_t b = controller_[0]->buttons();
    uint32_t wh = 0xFFFFFFFF, gr = 0xFF808080;
    sdlutil::GFX g(r);

    g.Box(x0-4, y0-4, 128+8, 24+8, 0x80800000);
    g.Rectangle(x0-4, y0-4, 128+8, 24+8, wh);
    g.String(x0+8, y0+0, "U", (b & Controller::BUTTON_UP) ? wh : gr);
    g.String(x0+0, y0+8, "L", (b & Controller::BUTTON_LEFT) ? wh : gr);
    g.String(x0+16, y0+8, "R", (b & Controller::BUTTON_RIGHT) ? wh : gr);
    g.String(x0+8, y0+16, "D", (b & Controller::BUTTON_DOWN) ? wh : gr);
    g.String(x0+32, y0+8, "Sel", (b & Controller::BUTTON_SELECT) ? wh : gr);
    g.String(x0+64, y0+8, "Sta", (b & Controller::BUTTON_START) ? wh : gr);
    g.String(x0+96, y0+8, "B", (b & Controller::BUTTON_B) ? wh : gr);
    g.String(x0+112, y0+8, "A", (b & Controller::BUTTON_A) ? wh : gr);

    char buf[32];
    int n = sprintf(buf, "Fr: %" PRIu64, ppu_->frame());
    g.String(x0+128-n*8, y0+16, buf, wh);
}

void NES::HandleKeyboard(SDL_Event *event) {
    if (event->type == SDL_KEYUP) {
        switch(event->key.keysym.scancode) {
        case SDL_SCANCODE_PAUSE:
            pause_ = !pause_;
            break;
        case SDL_SCANCODE_BACKSLASH:
            pause_ = true; step_ = true;
            break;
        case SDL_SCANCODE_PERIOD:
            debug_ = !debug_;
            break;
        default:
            ;
        }
    }
}

int NES::cpu_cycles() {
    return int(cpu_->cycles());
}

void NES::Run() {
    int c0 = 0, c1;
    int t=0;
    int n;
    bool pauseable = false;
    cpu_->reset();
    ppu_->Reset();

    for(t=0;;t++) {
        if (!debugger_->emulate())
            break;

        if (t % 2000 == 0) {
            if (!io_->emulate())
                break;
        }
        if (pauseable && pause_) {
            if (!step_)
                continue;
            step_ = false;
        }
        
        if (stall_ == 0) {
            cpu_->emulate();
            c1 = cpu_->cycles();
            n = c1 - c0;
            c0 = c1;
        } else {
            n = stall_;
            stall_ = 0;
        }

        pauseable = false;
        for(int i=0; i<n; i++) {
            // APU is clocked at the cpu speed (1.78 MHz)
            apu_->Emulate();
            for(int j=0; j<3; j++) {
                // The PPU is clocked at 3 dots per CPU clock
                ppu_->Emulate();
                mapper_->Emulate();

                if (ppu_->scanline() == 261 && ppu_->cycle() == 1)
                    pauseable = true;
            }
        }
    }
}

void NES::IRQ() {
    cpu_->irq();
}

void NES::NMI() {
    cpu_->nmi();
}
