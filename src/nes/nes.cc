#include <string.h>

#include "src/nes/nes.h"

#include "src/cpu.h"
#include "src/debugger.h"
#include "src/io.h"
#include "src/nes/apu.h"
#include "src/nes/cartridge.h"
#include "src/nes/controller.h"
#include "src/nes/mapper.h"
#include "src/nes/mem.h"
#include "src/nes/ppu.h"

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

NES::NES() {
    cpu_ = new Cpu();
    cart_ = new Cartridge(this);
    controller_[0] = new Controller(this);
    controller_[1] = new Controller(this);
    apu_ = new APU(this);
    mapper_ = nullptr;
    mem_ = new Mem(this);
    ppu_ = new PPU(this);
    io_ = new IO(256, 240, 60.0988);

    io_->init_audio(44100, 1, 4096, AUDIO_F32,
            [this](uint8_t* stream, int len) {
                apu_->PlayBuffer(stream, len);
            });
    io_->controller_config = "/usr/local/share/gamecontrollerdb.txt";
    io_->init_controllers([this](SDL_Event* event) {
        controller_[0]->set_buttons(event);
    });
    debugger_ = new Debugger();
    debugger_->cpu(cpu_);
    debugger_->memory(mem_);
    cpu_->memory(mem_);
    memcpy(palette_, standard_palette, sizeof(palette_));
}

void NES::LoadFile(const std::string& filename) {
    cart_->LoadFile(filename);
    mapper_ = MapperRegistry::New(this, cart_->mapper());
}

void NES::Run() {
    int c0 = 0, c1;
    int t0 = 0, t1;
    int n;
    cpu_->reset();
    ppu_->Reset();

    for(;;) {
        if (!debugger_->emulate())
            break;

        
        cpu_->emulate();
        t1 = c1 = cpu_->cycles();
        n = c1 - c0;
        c0 = c1;

        if (t1-t0 > 2000) {
            t0 = t1;
            if (!io_->emulate())
                break;
        }

        for(int i=0; i<n; i++) {
            // APU is clocked at the cpu speed (1.78 MHz)
            apu_->Emulate();
            for(int j=0; j<3; j++) {
                // The PPU is clocked at 3 dots per CPU clock
                ppu_->Emulate();
                mapper_->Emulate();
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
