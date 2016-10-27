#include <fstream>
#include <gflags/gflags.h>
#include "imgui.h"

#include "src/nes/mem.h"

#include "src/nes/apu.h"
#include "src/nes/cartridge.h"
#include "src/nes/controller.h"
#include "src/nes/mapper.h"
#include "src/nes/ppu.h"

DEFINE_string(memdump, "", "Custom memory dump textfile.");

Mem::Mem(NES* nes)
    : Memory(),
    nes_(nes),
    ram_{0, },
    ppuram_{0, } {
}

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
    if (addr < 0x2000) {
        return ram_[addr];
    } else if (addr >= 0x6000) {
        return nes_->mapper()->Read(addr);
    } else {
    }
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

void Mem::HexDump(int addr, int len) {
    static const char hex[] = "0123456789abcdef";
    char line[80];
    char *b = line;
    int i, c=55;
    uint8_t val;

    for(i=0; i < len; i++) {
        uint16_t a = addr + i;
        val = read_byte_no_io(a);
        if (i % 16 == 0) {
            if (i) {
                line[c] = '\0';
                ImGui::Text("%s", line);
            }
            memset(line, 32, sizeof(line));
            b = line; c = 55;
            *b++ = hex[(a>>12) & 0xf];
            *b++ = hex[(a>>8) & 0xf];
            *b++ = hex[(a>>4) & 0xf];
            *b++ = hex[(a>>0) & 0xf];
            *b++ = ':';
        }
        b++;
        *b++ = hex[(val>>4) & 0xf];
        *b++ = hex[(val>>0) & 0xf];
        line[c++] = (val>=32 && val<127) ? val : '.';
    }
    line[c] = '\0';
    ImGui::Text("%s", line);
}

bool Mem::ReadMemDump() {
    static bool once;
    if (FLAGS_memdump.empty())
        return false;
    if (once)
        return true;

    std::ifstream input(FLAGS_memdump);
    std::string line;

    while(getline(input, line)) {
        custom_memdump_.push_back(line);
    }
    once = true;
    return true;
}

void Mem::MemDump() {
    char buf[256];
    char fmt[128];

    if (!ReadMemDump())
        return;

    for(const auto& line : custom_memdump_) {
        char* b = buf;
        const char* ss = line.c_str();
        const char* s = ss;
        while(*s) {
            if (*s == '{') {
                char *end = nullptr;
                uint32_t addr = strtoul(s+1, &end, 16);
                s = end;
                if (*s == ':') {
                    char *f = fmt;
                    *f++ = '%';
                    while(*s != '}') {
                        *f++ = *s++;
                    }
                    *f++ = '\0'; s++;
                    b += sprintf(b, fmt, read_byte_no_io(addr));
                } else if (*s == '}') {
                    b += sprintf(b, "%02x", read_byte_no_io(addr));
                    s++;
                } else {

                    fprintf(stderr, "Unknown delimiter %c\n%s\n%*c\n",
                            *s, ss, int(s-ss), '^');
                    exit(1);
                }
            } else {
                *b++ = *s++;
            }
        }
        *b = '\0';
        ImGui::Text("%s", buf);
    }

}

void Mem::DebugStuff() {
    static bool display_hexdump, display_memdump;;
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Memory")) {
            ImGui::MenuItem("Hexdump", nullptr, &display_hexdump);
            ImGui::MenuItem("Memdump", nullptr, &display_memdump);
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }


    if (display_hexdump) {
        ImGui::Begin("Memory Hexdump", &display_hexdump);
        ImGui::Text("----- NES RAM -----");
        HexDump(0, 2048);
        ImGui::Text("---- Cartridge ----");
        HexDump(0x6000, 0xA000);
        ImGui::End();
    }
    if (display_memdump) {
        ImGui::Begin("Custom Memory Dump", &display_memdump);
        MemDump();
        ImGui::End();
    }
}
