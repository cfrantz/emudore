#include <tuple>
#include "src/nes/fm2.h"
#include "src/nes/ppu.h"
#include "src/nes/mem.h"
#include "src/io.h"

PPU::PPU(NES* nes)
    : nes_(nes),
    cycle_(0), scanline_(0), frame_(0),
    oam_{0, },
    v_(0), t_(0), x_(0), w_(0), f_(0), register_(0),
    nmi_{0,},
    nametable_(0), attrtable_(0), lowtile_(0), hightile_(0), tiledata_(0),
    sprite_{0,},
    control_{0,},
    mask_{0,},
    status_{0,},
    oam_addr_(0), buffered_data_(0),
    picture_{0,} {}

void PPU::Reset() {
    cycle_ = 340;
    scanline_ = 240;
    frame_ = 0;
    set_control(0);
    set_mask(0);
    oam_addr_ = 0;
}

void PPU::NmiChange() {
    uint8_t nmi = nmi_.output & nmi_.occured;
    if (nmi && !nmi_.previous)
        nmi_.delay = 15;
    nmi_.previous = nmi;
}

void PPU::set_control(uint8_t val) {
    control_.nametable =   val & 3;
    control_.increment =   val >> 2;
    control_.spritetable = val >> 3;
    control_.bgtable =     val >> 4;
    control_.spritesize =  val >> 5;
    control_.master =      val >> 6;
    nmi_.output = val >> 7;
    NmiChange();
    t_ = (t_ & 0xF3FF) | (uint16_t(val & 3) << 10);
}

void PPU::set_mask(uint8_t val) {
    uint8_t *mask = (uint8_t*)&mask_;
    *mask = val;
}

uint8_t PPU::status() {
    uint8_t result;
    result = register_ & 0x1F;
    result |= uint8_t(status_.sprite_overflow) << 5;
    result |= uint8_t(status_.sprite0_hit) << 6;
    result |= uint8_t(nmi_.occured) << 7;
    nmi_.occured = false;
    NmiChange();
    w_ = 0;
    return result;
}

void PPU::set_scroll(uint8_t val) {
    if (w_ == 0) {
        t_ = (t_ & 0xFFE0) | (val >> 3);
        x_ = val & 7;
        w_ = 1;
    } else {
        t_ = (t_ & 0x8FFF) | (uint16_t(val & 0x07) << 12);
        t_ = (t_ & 0xFC1F) | (uint16_t(val & 0xF8) << 2);
        w_ = 0;
    }
}

void PPU::set_address(uint8_t val) {
    if (w_ == 0) {
        t_ = (t_ & 0x80FF) | (uint16_t(val & 0x3F) << 8);
        w_ = 1;
    } else {
        t_ = (t_ & 0xFF00) | val;
        v_ = t_;
        w_ = 0;
    }
}

uint8_t PPU::data() {
    uint8_t result = nes_->memory()->PPURead(v_);

    if (v_ % 0x4000 < 0x3F00) {
        std::swap(buffered_data_, result);
    } else {
        buffered_data_ = nes_->memory()->PPURead(v_ - 0x1000);
    }
    v_ += control_.increment ? 32 : 1;
    return result;
}

void PPU::set_data(uint8_t val) {
    nes_->memory()->PPUWrite(v_, val);
    v_ += control_.increment ? 32 : 1;
}

void PPU::set_dma(uint8_t val) {
    uint16_t addr = uint16_t(val) << 8;
    for(int i=0; i<256; i++) {
        oam_[oam_addr_++] = nes_->memory()->Read(addr++);
    }
    // TODO(cfrantz):
    // stall cpu for 513 + (cpu.cycles % 2) cycles.
    nes_->Stall(513 + nes_->cpu_cycles() % 2);
}

uint8_t PPU::Read(uint16_t addr) {
    switch(addr) {
        case 0x2002: return status();
        case 0x2004: return oam_[oam_addr_];
        case 0x2007: return data();
    }
    return 0;
}

void PPU::Write(uint16_t addr, uint8_t val) {
    register_ = val;
    switch(addr) {
        case 0x2000: set_control(val); break;
        case 0x2001: set_mask(val); break;
        case 0x2003: oam_addr_ = val; break;
        case 0x2004: oam_[oam_addr_++] = val; break;
        case 0x2005: set_scroll(val); break;
        case 0x2006: set_address(val); break;
        case 0x2007: set_data(val); break;
        case 0x4014: set_dma(val); break;
    }
}

void PPU::IncrementX() {
    if ((v_ & 0x001f) == 0x1f) {
        v_ = (v_ & 0xFFE0) ^ 0x0400;
    } else {
        v_++;
    }
}

void PPU::IncrementY() {
    if ((v_ & 0x7000) != 0x7000) {
        v_ += 0x1000;
    } else {
        v_ = v_ & 0x8FFF;
        uint16_t y = (v_ & 0x03E0) >> 5;
        if (y == 29) {
            y = 0;
            v_ = v_ ^ 0x800;
        } else if (y == 31) {
            y = 0;
        } else {
            y++;
        }
        v_ = (v_ & 0xFC1F) | (y << 5);
    }
}

void PPU::CopyX() {
    v_ = (v_ & 0xFBE0) | (t_ & 0x041F);
}

void PPU::CopyY() {
    v_ = (v_ & 0x841F) | (t_ & 0x7BE0);
}

void PPU::SetVerticalBlank() {
    //static uint64_t last;
    nmi_.occured = true;
    NmiChange();
    nes_->io()->screen_blit(picture_);
    nes_->io()->screen_refresh();
    //printf("frametime = %" PRIu64 "us\n", uint64_t(nes_->io()->clock_micros() - last));
    //last = nes_->io()->clock_micros();
}

void PPU::ClearVerticalBlank() {
    nmi_.occured = false;
    NmiChange();
}

void PPU::FetchNameTableByte() {
    nametable_ = nes_->memory()->PPURead(0x2000 | (v_ & 0x0FFF));
}

void PPU::FetchAttributeByte() {
    uint16_t a = 0x23C0 | (v_ & 0x0C00) | ((v_ >> 4) & 0x38) | ((v_ >> 2) & 7);
    uint8_t shift = ((v_ >> 4) & 4) | (v_ & 2);
    attrtable_ = ((nes_->memory()->PPURead(a) >> shift) & 3) << 2;
}

void PPU::FetchLowTileByte() {
    uint16_t a = (0x1000 * control_.bgtable) + (16 * nametable_) +
                 ((v_ >> 12) & 7);
    lowtile_ = nes_->memory()->PPURead(a);
}

void PPU::FetchHighTileByte() {
    uint16_t a = (0x1000 * control_.bgtable) + (16 * nametable_) +
                 ((v_ >> 12) & 7); hightile_ = nes_->memory()->PPURead(a + 8);
}

void PPU::StoreTileData() {
    uint64_t data = 0;
    for(int i=0; i<8; i++) {
        uint8_t a = attrtable_;
        uint8_t p1 = (lowtile_ & 0x80) >> 7;
        uint8_t p2 = (hightile_ & 0x80) >> 6;
        lowtile_ <<= 1;
        hightile_ <<= 1;
        data = (data << 4) | a | p1 | p2;
    }
    tiledata_ |= data;
}

uint8_t PPU::BackgroundPixel() {
    if (!mask_.showbg)
        return 0;
    uint32_t data = (tiledata_ >> 32) >> ((7-x_) * 4);
    return uint8_t(data & 0x0F);
}

std::tuple<uint8_t, uint8_t> PPU::SpritePixel() {
    if (mask_.showsprites) {
        for(int i=0; i < sprite_.count; i++) {
            uint32_t offset = cycle_ - 1 - sprite_.position[i];
            if (offset > 7)
                continue;
            offset = 7 - offset;
            uint8_t color = (sprite_.pattern[i] >> (offset*4)) & 0x0F;
            if (color % 4 == 0)
                continue;

            return std::make_tuple(i, color);
        }
    }
    return std::make_tuple(0, 0);
}


void PPU::RenderPixel() {
    int x = cycle_ - 1;
    int y = scanline_;
    uint8_t background = BackgroundPixel();
    auto sp = SpritePixel();
    uint8_t i = std::get<0>(sp), sprite = std::get<1>(sp);
    uint8_t color;

    if (x < 8) {
        if (!mask_.showleftbg) background = 0;
        if (!mask_.showleftsprite) sprite = 0;
    }

    bool b = (background % 4) != 0;
    bool s = (sprite % 4) != 0;

    if (!b) {
        color = s ? (sprite | 0x10) : 0;
    } else if (!s) {
        color = background;
    } else {
        if (sprite_.index[i] == 0 && x < 255)
            status_.sprite0_hit = 1;

        if (sprite_.priority[i] == 0) {
            color = sprite | 0x10;
        } else {
            color = background;
        }
    }
    picture_[y * 256 + x] = nes_->palette(nes_->memory()->PaletteRead(color));
}

uint32_t PPU::FetchSpritePattern(int i, int row) {
    uint8_t tile = oam_[i*4 + 1];
    uint8_t attr = oam_[i*4 + 2];
    uint16_t addr;
    uint16_t table;

    if (!control_.spritesize) {
        if (attr & 0x80)
            row = 7-row;
        table = control_.spritetable;
    } else {
        if (attr & 0x80)
            row = 15-row;
        table = tile & 1;
        tile = tile & 0xFE;
        if (row > 7) {
            tile++; row -= 8;
        }
    }
    
    addr = 0x1000 * table + tile * 16 + row;
    uint8_t a = (attr & 3) << 2;
    uint8_t lo = nes_->memory()->PPURead(addr);
    uint8_t hi = nes_->memory()->PPURead(addr + 8);
    uint32_t result = 0;

    for(i=0; i<8; i++) {
        uint8_t p1, p2;
        if (attr & 0x40) {
            p1 = lo & 1;
            p2 = (hi & 1) << 1;
            lo >>= 1; hi >>= 1;
        } else {
            p1 = (lo & 0x80) >> 7;
            p2 = (hi & 0x80) >> 6;
            lo <<= 1; hi <<= 1;
        }
        result = (result << 4) | a | p1 | p2;
    }
    return result;
}

void PPU::EvaluateSprites() {
    int h = (control_.spritesize) ? 16 : 8;
    int count = 0;

    for(int i=0; i<64; i++) {
        uint8_t y = oam_[i*4 + 0];
        uint8_t a = oam_[i*4 + 2];
        uint8_t x = oam_[i*4 + 3];
        int row = scanline_ - y;

        if (!(row >= 0 && row < h))
            continue;

        if (count < 8) {
            sprite_.pattern[count] = FetchSpritePattern(i, row);
            sprite_.position[count] = x;
            sprite_.priority[count] = (a >> 5) & 1;
            sprite_.index[count] = i;
        }
        ++count;
    }
    if (count > 8) {
        count = 8;
        status_.sprite_overflow = 1;
    }
    sprite_.count = count;
}

void PPU::Tick() {
    if (nmi_.delay) {
        nmi_.delay--;
        if (nmi_.delay == 0 && nmi_.output && nmi_.occured)
            nes_->NMI();
    }
    if (mask_.showbg || mask_.showsprites) {
        if (f_ == 1 && scanline_ == 261 && cycle_ == 339) {
            cycle_ = 0;
            scanline_ = 0;
            frame_++;
            nes_->movie()->Emulate(frame_);
            f_ = f_ ^ 1;
            return;
        }
    }

    cycle_++;
    if (cycle_ > 340) {
        cycle_ = 0;
        scanline_++;
        if (scanline_ > 261) {
            scanline_ = 0;
            frame_++;
            nes_->movie()->Emulate(frame_);
            f_ = f_ ^ 1;
        }
    }
}

void PPU::Emulate() {
    Tick();

    const bool pre_line = scanline_ == 261;
    const bool visible_line = scanline_ < 240;
    const bool render_line = pre_line || visible_line;
    const bool prefetch_cycle = (cycle_ >= 321 && cycle_ <= 336);
    const bool visible_cycle = (cycle_ > 0 && cycle_ <= 256);
    const bool fetch_cycle = prefetch_cycle || visible_cycle;

    if (mask_.showbg || mask_.showsprites) {
        if (visible_line && visible_cycle)
            RenderPixel();

        if (render_line && fetch_cycle) {
            tiledata_ <<= 4;
            switch(cycle_ % 8) {
            case 0: StoreTileData(); break;
            case 1: FetchNameTableByte(); break;
            case 3: FetchAttributeByte(); break;
            case 5: FetchLowTileByte(); break;
            case 7: FetchHighTileByte(); break;
            }
        }

        if (pre_line && cycle_ >= 280 && cycle_ <= 304)
            CopyY();

        if (render_line) {
            if (fetch_cycle && (cycle_ % 8) == 0)
                IncrementX();
            if (cycle_ == 256)
                IncrementY();
            if (cycle_ == 257)
                CopyX();
        }
        if (cycle_ == 257) {
            if (visible_line) {
                EvaluateSprites();
            } else {
                sprite_.count = 0;
            }
        }
    }

    if (scanline_ == 241 && cycle_ == 1)
        SetVerticalBlank();
    if (pre_line && cycle_ == 1) {
        ClearVerticalBlank();
        status_.sprite0_hit = 0;
        status_.sprite_overflow = 0;
    }
}

