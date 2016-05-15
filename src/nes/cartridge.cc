#include <cstdio>
#include <cstdlib>
#include <cstdint>

#include "src/nes/cartridge.h"

Cartridge::Cartridge(NES* nes)
    : nes_(nes),
    prg_(nullptr), prglen_(0),
    chr_(nullptr), chrlen_(0),
    trainer_(nullptr) {
}


Cartridge::~Cartridge() {
    delete[] prg_;
    delete[] chr_;
    delete[] trainer_;
}


void Cartridge::LoadFile(const std::string& filename) {
    FILE* fp = fopen(filename.c_str(), "rb");

    if (fp == nullptr) {
        fprintf(stderr, "Couldn't read %s.\n", filename.c_str());
        abort();
    }
    if (fread(&header_, sizeof(header_), 1, fp) != 1) {
        fprintf(stderr, "Couldn't read header.\n");
        abort();
    }
    PrintHeader();

    mirror_ = MirrorMode(header_.mirror0 | (header_.mirror1 << 1));
    prglen_ = 16384 * header_.prgsz;
    prg_ = new uint8_t[prglen_];

    if (header_.chrsz) {
        chrlen_ = 8192 * header_.chrsz;
    } else {
        // No chr rom, need and 8k buffer (ram?)
        chrlen_ = 8192;
    }
    chr_ = new uint8_t[chrlen_];

    if (header_.trainer) {
        trainer_ = new uint8_t[512];
        if (fread(trainer_, 1, 512, fp) != 512) {
            fprintf(stderr, "Couldn't read trainer.\n");
            abort();
        }
    }

    if (fread(prg_, 16384, header_.prgsz, fp) != header_.prgsz) {
        fprintf(stderr, "Couldn't read PRG.\n");
        abort();
    }

    if (fread(chr_, 8192, header_.chrsz, fp) != header_.chrsz) {
        fprintf(stderr, "Couldn't read CHR.\n");
        abort();
    }
}

void Cartridge::PrintHeader() {
    uint8_t *bytes = (uint8_t*)&header_;
    printf("NES header:\n");
    printf("  %02x %02x %02x %02x %02x %02x %02x %02x\n",
           bytes[0], bytes[1], bytes[2], bytes[3],
           bytes[4], bytes[5], bytes[6], bytes[7]);
    bytes += 8;
    printf("  %02x %02x %02x %02x %02x %02x %02x %02x\n",
           bytes[0], bytes[1], bytes[2], bytes[3],
           bytes[4], bytes[5], bytes[6], bytes[7]);

    printf("  PRG banks: %d\n", header_.prgsz);
    printf("  CHR banks: %d\n", header_.chrsz);
    printf("  Mirroring: %d\n", mirror_);
    printf("  Has SRAM:  %d\n", header_.sram);
    printf("  Has trainer: %d\n", header_.trainer);
    printf("  Mapper: %d\n", mapper());
}

uint8_t Cartridge::ReadPrg(uint32_t addr) {
    return prg_[addr];
}
uint8_t Cartridge::ReadChr(uint32_t addr) {
    return chr_[addr];
}
void Cartridge::WriteChr(uint32_t addr, uint8_t val) {
    chr_[addr] = val;
}
uint8_t Cartridge::ReadSram(uint32_t addr) {
    return sram_[addr];
}
void Cartridge::WriteSram(uint32_t addr, uint8_t val) {
    sram_[addr] = val;
}
