#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <gflags/gflags.h>

#include "src/nes/cartridge.h"

DEFINE_bool(sram_on_disk, true, "Save SRAM to disk.");

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
    fclose(fp);

    sram_filename_ = filename + ".sram";
    if (FLAGS_sram_on_disk && header_.sram) {
        if ((fp = fopen(sram_filename_.c_str(), "rb")) != nullptr) {;
            if (fread(sram_, sizeof(sram_), 1, fp) == 1) {
                fprintf(stderr, "Couldn't read SRAM.\n");
            }
            fclose(fp);
        }
    }
}

void Cartridge::Emulate() {
    static uint64_t last_frame;

    if (last_frame != nes_->frame()) {
        SaveSram();
    }
    last_frame = nes_->frame();
}

void Cartridge::SaveSram() {
    if (!FLAGS_sram_on_disk)
        return;
    if (!header_.sram)
        return;

    FILE *fp = fopen(sram_filename_.c_str(), "wb");
    if (!fp) {
        fprintf(stderr, "Can't open %s for writing.\n", sram_filename_.c_str());
        return;
    }
    fwrite(sram_, 1, sizeof(sram_), fp);
    fclose(fp);
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
