#include "src/nes/mem.h"

#include "src/nes/apu.h"
#include "src/nes/cartridge.h"
#include "src/nes/controller.h"
#include "src/nes/mapper.h"
#include "src/nes/ppu.h"

Mem::Mem(NES* nes)
    : Memory(),
    nes_(nes),
    ram_{0, },
    ppuram_{0, } {}

uint8_t Mem::read_byte(uint16_t addr) {
    if (addr < 0x2000) {
        return ram_[addr];
    } else if (addr < 0x4000 || addr == 0x4014) {
        return nes_->ppu()->Read(addr);
    } else if (addr == 0x4015) {
        return nes_->apu()->Read(addr);
    } else if (addr == 0x4016) {
        return nes_->controller(0)->Read();
    } else if (addr == 0x4017) {
        return nes_->controller(1)->Read();
    } else if (addr >= 0x6000) {
        return nes_->mapper()->Read(addr);
    } else {
        fprintf(stderr, "Unknown read at %04x\n", addr);
    }
    return 0;
}

uint8_t Mem::read_byte_no_io(uint16_t addr) {
    return 0;
}

void Mem::write_byte(uint16_t addr, uint8_t v) {
    if (addr < 0x2000) {
        ram_[addr] = v;
    } else if (addr < 0x4000 || addr == 0x4014) {
        return nes_->ppu()->Write(addr, v);
    } else if (addr == 0x4016) {
        return nes_->controller(0)->Write(v);
        return nes_->controller(1)->Write(v);
    } else if (addr >= 0x4000 && addr <= 0x4017) {
        nes_->apu()->Write(addr, v);
    } else if (addr >= 0x6000) {
        nes_->mapper()->Write(addr, v);
    } else {
        fprintf(stderr, "Unknown write at %04x\n", addr);
    }
}

void Mem::write_byte_no_io(uint16_t addr, uint8_t v) {
}

uint16_t Mem::read_word(uint16_t addr) {
    uint16_t result = read_byte(addr);
    result |= read_byte(addr+1) << 8;
    return result;
}

uint16_t Mem::read_word_no_io(uint16_t) {
    return 0;
}
void Mem::write_word(uint16_t addr, uint16_t v) {
}
void Mem::write_word_no_io(uint16_t addr, uint16_t v) {
}

uint16_t Mem::MirrorAddress(int mode, uint16_t addr) {
    static const uint16_t lookup[5][4] = {
        { 0, 0, 1, 1, },
        { 0, 1, 1, 0, },
        { 0, 0, 0, 0, },
        { 1, 1, 1, 1, },
        { 0, 1, 2, 3, },
    };
    addr = (addr - 0x2000) % 0x1000;
    int table = addr / 0x400;
    int offset = addr % 0x400;
    return lookup[mode][table] * 0x400 + offset + 0x2000;
}

uint8_t Mem::PPURead(uint16_t addr) {
    addr %= 0x4000;
    if (addr < 0x2000) {
        return nes_->mapper()->Read(addr);
    } else if (addr < 0x3F00) {
        int mode = int(nes_->cartridge()->mirror());
        return ppuram_[MirrorAddress(mode, addr) % 2048];
    } else {
        return PaletteRead(addr % 32);
    }
}

void Mem::PPUWrite(uint16_t addr, uint8_t val) {
    addr %= 0x4000;
    if (addr < 0x2000) {
        nes_->mapper()->Write(addr, val);
    } else if (addr < 0x3F00) {
        int mode = int(nes_->cartridge()->mirror());
        ppuram_[MirrorAddress(mode, addr) % 2048] = val;
    } else {
        PaletteWrite(addr % 32, val);
    }
}

uint8_t Mem::PaletteRead(uint16_t addr) {
    if (addr >= 16 && (addr % 4) == 0)
        addr -= 16;
    return palette_[addr];
}

void Mem::PaletteWrite(uint16_t addr, uint8_t val) {
    if (addr >= 16 && (addr % 4) == 0)
        addr -= 16;
    palette_[addr] = val;
}
