#ifndef EMUDORE_SRC_NES_CARTRIDGE_H
#define EMUDORE_SRC_NES_CARTRIDGE_H
#include <string>
#include <cstdint>

#include "src/nes/nes.h"

class Cartridge {
  public:
    struct iNESHeader {
        uint32_t signature;
        uint8_t prgsz;
        uint8_t chrsz;
        uint8_t mirror0: 1;
        uint8_t sram: 1;
        uint8_t trainer: 1;
        uint8_t mirror1: 1;
        uint8_t mapperl: 4;
        uint8_t vs_unisystem: 1;
        uint8_t playchoice_10: 1;
        uint8_t version: 2;
        uint8_t mapperh: 4;
        uint8_t unused[8];
    };
    enum MirrorMode {
        HORIZONTAL,
        VERTICAL,
        SINGLE0,
        SINGLE1,
        FOUR,
    };
    Cartridge(NES* nes);
    ~Cartridge();

    void LoadFile(const std::string& filename);
    void PrintHeader();
    inline uint8_t mirror() const { return mirror_; }
    inline void set_mirror(MirrorMode m) { mirror_ = m; }
    inline bool battery() const {
        return header_.sram;
    }
    inline uint8_t mapper() const {
        return header_.mapperl | header_.mapperh << 4;
    }
    inline uint32_t prglen() const { return prglen_; }
    inline uint32_t chrlen() const { return chrlen_; }

    uint8_t ReadPrg(uint32_t addr);
    uint8_t ReadChr(uint32_t addr);
    void WriteChr(uint32_t addr, uint8_t val);
    uint8_t ReadSram(uint32_t addr);
    void WriteSram(uint32_t addr, uint8_t val);

  private:
    NES* nes_;
    struct iNESHeader header_;
    uint8_t *prg_;
    uint32_t prglen_;
    uint8_t *chr_;
    uint32_t chrlen_;
    uint8_t *trainer_;
    MirrorMode mirror_;
    uint8_t sram_[0x2000];
};

#endif // EMUDORE_SRC_NES_CARTRIDGE_H
