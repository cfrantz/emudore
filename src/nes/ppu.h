#ifndef EMUDORE_SRC_NES_PPU_H
#define EMUDORE_SRC_NES_PPU_H
#include <cstdint>
#include "src/nes/nes.h"

class PPU {
  public:
    PPU(NES* nes);
    void Reset();
    uint8_t Read(uint16_t addr);
    void Write(uint16_t addr, uint8_t val);
    void Emulate();

    inline uint64_t frame() const { return frame_; }
    inline int scanline() const { return scanline_; }
    inline int cycle() const { return cycle_; }
  private:
    void NmiChange();
    void set_control(uint8_t val);
    void set_mask(uint8_t val);
    uint8_t status();
    void set_scroll(uint8_t val);
    void set_address(uint8_t val);
    uint8_t data();
    void set_data(uint8_t val);
    void set_dma(uint8_t val);
    void IncrementX();
    void IncrementY();
    void CopyX();
    void CopyY();
    void SetVerticalBlank();
    void ClearVerticalBlank();
    void FetchNameTableByte();
    void FetchAttributeByte();
    void FetchLowTileByte();
    void FetchHighTileByte();
    void StoreTileData();
    uint8_t BackgroundPixel();
    std::tuple<uint8_t, uint8_t> SpritePixel();
    void RenderPixel();
    uint32_t FetchSpritePattern(int i, int row);
    void EvaluateSprites();
    void Tick();

    NES* nes_;

    int cycle_;
    int scanline_;
    uint64_t frame_;

    uint8_t oam_[256];

    // PPU registers
    uint16_t v_;
    uint16_t t_;
    uint8_t x_;
    uint8_t w_;
    uint8_t f_;
    uint8_t register_;

    struct {
        uint8_t occured:1;
        uint8_t previous:1;
        uint8_t output:1;
        uint8_t delay;
    } nmi_;

    uint8_t nametable_;
    uint8_t attrtable_;
    uint8_t lowtile_, hightile_;
    uint64_t tiledata_;

    struct {
        int count;
        uint32_t pattern[8];
        uint8_t position[8];
        uint8_t priority[8];
        uint8_t index[8];
    } sprite_;

    struct {
        uint8_t nametable: 2;
        uint8_t increment: 1;
        uint8_t spritetable: 1;
        uint8_t bgtable: 1;
        uint8_t spritesize: 1;
        uint8_t master: 1;
    } control_;

    struct {
        uint8_t grayscale: 1;
        uint8_t showleftbg: 1;
        uint8_t showleftsprite: 1;
        uint8_t showbg: 1;
        uint8_t showsprites: 1;
        uint8_t redtint: 1;
        uint8_t greentint: 1;
        uint8_t bluetint: 1;
    } mask_;

    struct {
        uint8_t sprite0_hit: 1;
        uint8_t sprite_overflow: 1;
    } status_;

    uint8_t oam_addr_;
    uint8_t buffered_data_;

    uint32_t picture_[256*240];
};

#endif // EMUDORE_SRC_NES_PPU_H
