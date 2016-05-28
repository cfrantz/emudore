#include <cstdio>
#include "src/cpu2.h"

// Information about each instruction is encoded into the info_ table.
// Each 4 bits means (from lowest to highest):
//    AddressingMode
//    Instruction Size (in bytes)
//    Cyles
//    Extra cycles when crossing a page boundary
const Cpu::InstructionInfo Cpu::info_[256] = {
    // 0x00      1       2       3       4       5       6       7
    0x0715, 0x0626, 0x0205, 0x0806, 0x032a, 0x032a, 0x052a, 0x050a, 
    0x0315, 0x0224, 0x0213, 0x0204, 0x0430, 0x0430, 0x0630, 0x0600, 
    // 0x10
    0x1229, 0x1528, 0x0205, 0x0808, 0x042b, 0x042b, 0x062b, 0x060b, 
    0x0215, 0x1432, 0x0215, 0x0702, 0x1431, 0x1431, 0x0731, 0x0701, 
    // 0x20
    0x0630, 0x0626, 0x0205, 0x0806, 0x032a, 0x032a, 0x052a, 0x050a, 
    0x0415, 0x0224, 0x0213, 0x0204, 0x0430, 0x0430, 0x0630, 0x0600, 
    // 0x30
    0x1229, 0x1528, 0x0205, 0x0808, 0x042b, 0x042b, 0x062b, 0x060b, 
    0x0215, 0x1432, 0x0215, 0x0702, 0x1431, 0x1431, 0x0731, 0x0701, 
    // 0x40
    0x0615, 0x0626, 0x0205, 0x0806, 0x032a, 0x032a, 0x052a, 0x050a, 
    0x0315, 0x0224, 0x0213, 0x0204, 0x0330, 0x0430, 0x0630, 0x0600, 
    // 0x50
    0x1229, 0x1528, 0x0205, 0x0808, 0x042b, 0x042b, 0x062b, 0x060b, 
    0x0215, 0x1432, 0x0215, 0x0702, 0x1431, 0x1431, 0x0731, 0x0701, 
    // 0x60
    0x0615, 0x0626, 0x0205, 0x0806, 0x032a, 0x032a, 0x052a, 0x050a, 
    0x0415, 0x0224, 0x0213, 0x0204, 0x0537, 0x0430, 0x0630, 0x0600, 
    // 0x70
    0x1229, 0x1528, 0x0205, 0x0808, 0x042b, 0x042b, 0x062b, 0x060b, 
    0x0215, 0x1432, 0x0215, 0x0702, 0x1431, 0x1431, 0x0731, 0x0701, 
    // 0x80
    0x0224, 0x0626, 0x0204, 0x0606, 0x032a, 0x032a, 0x032a, 0x030a, 
    0x0215, 0x0204, 0x0215, 0x0204, 0x0430, 0x0430, 0x0430, 0x0400, 
    // 0x90
    0x1229, 0x0628, 0x0205, 0x0608, 0x042b, 0x042b, 0x042c, 0x040c, 
    0x0215, 0x0532, 0x0215, 0x0502, 0x0501, 0x0531, 0x0502, 0x0502, 
    // 0xA0
    0x0224, 0x0626, 0x0224, 0x0606, 0x032a, 0x032a, 0x032a, 0x030a, 
    0x0215, 0x0224, 0x0215, 0x0204, 0x0430, 0x0430, 0x0430, 0x0400, 
    // 0xB0
    0x1229, 0x1528, 0x0205, 0x1508, 0x042b, 0x042b, 0x042c, 0x040c, 
    0x0215, 0x1432, 0x0215, 0x1402, 0x1431, 0x1431, 0x1432, 0x1402, 
    // 0xC0
    0x0224, 0x0626, 0x0204, 0x0806, 0x032a, 0x032a, 0x052a, 0x050a, 
    0x0215, 0x0224, 0x0215, 0x0204, 0x0430, 0x0430, 0x0630, 0x0600, 
    // 0xD0
    0x1229, 0x1528, 0x0205, 0x0808, 0x042b, 0x042b, 0x062b, 0x060b, 
    0x0215, 0x1432, 0x0215, 0x0702, 0x1431, 0x1431, 0x0731, 0x0701, 
    // 0xE0
    0x0224, 0x0626, 0x0204, 0x0806, 0x032a, 0x032a, 0x052a, 0x050a, 
    0x0215, 0x0224, 0x0215, 0x0204, 0x0430, 0x0430, 0x0630, 0x0600, 
    // 0xF0
    0x1229, 0x1528, 0x0205, 0x0808, 0x042b, 0x042b, 0x062b, 0x060b, 
    0x0215, 0x1432, 0x0215, 0x0702, 0x1431, 0x1431, 0x0731, 0x0701, 
};

void Cpu::Branch(uint16_t addr) {
    static uint16_t last_pc, last_addr;
    if (pc_ == last_pc && addr == last_addr && addr == pc_ - 2)
        abort();
    last_pc = pc_; last_addr = addr;
    if (PagesDiffer(pc_, addr))
        cycles_++;
    pc_ = addr;
    cycles_++;
}


Cpu::Cpu(Memory* mem) :
    mem_(mem),
    flags_{0x24},
    pc_(0),
    sp_(0xFD),
    a_(0), x_(0), y_(0),
    cycles_(0),
    stall_(0),
    nmi_pending_(false),
    irq_pending_(false) {}

void Cpu::Reset() {
    pc_ = Read16(0xFFFC);
    sp_ = 0xFD;
    flags_.value = 0x24;
    a_ = x_ = y_ = 0;
    nmi_pending_ = false;
    irq_pending_ = false;
    cycles_ = 0;
    stall_ = 0;
}

int Cpu::Emulate(void) {
    if (stall_ > 0) {
      stall_--;
      return 1;
    }
    int cycles = cycles_;

    // Interrupt?
    if (nmi_pending_) {
//        printf("NMI @ %d\n", cycles_);
        nmi_pending_ = false;
        Push16(pc_);
        Push(flags_.value | 0x10);
        pc_ = Read16(0xFFFA);
        flags_.i = true;
        cycles_ += 7;
    } else if (irq_pending_) {
        irq_pending_ = false;
        Push16(pc_);
        Push(flags_.value | 0x10);
        pc_ = Read16(0xFFFE);
        flags_.i = true;
        cycles_ += 7;
    }

    // Scratch values
    uint8_t val, a, b;
    int16_t r;

    uint16_t fetchpc = pc_;
    uint16_t addr = 0;
    uint8_t opcode = Read(pc_);
    InstructionInfo info = info_[opcode];

#undef TESTCPU
#ifdef TESTCPU
    printf("      A=%02x X=%02x Y=%02x SP=1%02x %c%c%c%c%c%c%c%c\n",
            a_, x_, y_, sp_,
            flags_.n ? 'N' : 'n',
            flags_.v ? 'V' : 'v',
            flags_.u ? 'U' : 'u',
            flags_.b ? 'B' : 'b',
            flags_.d ? 'D' : 'd',
            flags_.i ? 'I' : 'i',
            flags_.z ? 'Z' : 'z',
            flags_.c ? 'C' : 'c');
    switch(info.size) {
    case 1:
        printf("%02x: %02x\n", pc_, opcode);
        break;
    case 2:
        printf("%02x: %02x%02x\n", pc_, opcode, Read(pc_+1));
        break;
    case 3:
        printf("%02x: %02x%02x%02x\n", pc_, opcode, Read(pc_+1), Read(pc_+2));
        break;
    }
#endif
    
    //printf("Decoding instruction at %04x -> %02x\n", pc_, opcode);
    // Based on the AddressingMode of the instruction, compute the address
    // target to be used by the instruction.
    switch(AddressingMode(info.mode)) {
    case Absolute:
        addr = Read16(pc_+1);
        break;
    case AbsoluteX:
        addr = Read16(pc_+1) + x_;
        if (PagesDiffer(addr - x_, addr))
            cycles_ += info.page;
        break;
    case AbsoluteY:
        addr = Read16(pc_+1) + y_;
        if (PagesDiffer(addr - y_, addr))
            cycles_ += info.page;
        break;
    case IndexedIndirect:
        //addr = Read16Bug(Read(pc_+1) + x_);
        addr = Read16((Read(pc_ + 1) + x()) & 0xff);
        break;
    case Indirect:
        addr = Read16Bug(Read16(pc_+1));
        break;
    case IndirectIndexed:
        //addr = Read16Bug(Read(pc_+1)) + y_;
        // Fixed?
        addr = Read16(Read(pc_ + 1)) + y();
        if (PagesDiffer(addr - y_, addr))
            cycles_ += info.page;
        break;
    case ZeroPage:
        addr = Read(pc_ + 1);
        break;
    case ZeroPageX:
        addr = (Read(pc_ + 1) + x_) & 0xFF;
        break;
    case ZeroPageY:
        addr = (Read(pc_ + 1) + y_) & 0xFF;
        break;
    case Immediate:
        addr = pc_ + 1;
        break;
    case Accumulator:
    case Implied:
        addr = 0;
        break;
    case Relative:
        addr = pc_ + 2 + int8_t(Read(pc_ + 1));;
        break;
    }

#ifdef TESTCPU
    printf("Computed address %04x via mode %d (%02x %02x)\n",
            addr, info.mode, Read(addr), Read(addr+1));
#endif
    pc_ += info.size;
    cycles_ += info.cycles;


    switch(opcode) {
    /* BRK */
    case 0x0:
        Push16(pc_+1);
        Push(flags_.value | 0x10);
        flags_.i = 1;
        pc_ = Read16(0xFFFE);
        break;
    /* ORA (nn,X) */
    case 0x1:
    /* ORA nn */
    case 0x5:
    /* ORA #nn */
    case 0x9:
    /* ORA nnnn */
    case 0xD:
    /* ORA (nn,Y) */
    case 0x11:
    /* ORA nn,X */
    case 0x15:
    /* ORA nnnn,Y */
    case 0x19:
    /* ORA nnnn,X */
    case 0x1D:
        a_ = a_ | Read(addr);
        SetZN(a_);
        break;
    /* ASL nn */
    case 0x6:
    /* ASL nnnn */
    case 0xE:
    /* ASL nn,X */
    case 0x16:
    /* ASL nnnn,X */
    case 0x1E:
        val = Read(addr);
        flags_.c = val >> 7;
        val <<= 1;
        Write(addr, val);
        SetZN(val);
        break;
    /* ASL A */
    case 0xA:
        flags_.c = a_ >> 7;
        a_ <<= 1;
        SetZN(a_);
        break;
    /* PHP */
    case 0x8:
        Push(flags_.value | 0x10);
        break;
    /* BPL nn */
    case 0x10:
        if (!flags_.n)
            Branch(addr);
        break;
    /* CLC */
    case 0x18:
        flags_.c = 0;
        break;
    /* JSR */
    case 0x20:
        Push16(pc_ - 1);
        pc_ = addr;
        break;
    /* AND (nn,X) */
    case 0x21:
    /* AND nn */
    case 0x25:
    /* AND #nn */
    case 0x29:
    /* AND nnnn */
    case 0x2D:
    /* AND (nn,Y) */
    case 0x31:
    /* AND nn,X */
    case 0x35:
    /* AND nnnn,Y */
    case 0x39:
    /* AND nnnn,X */
    case 0x3D:
        a_ = a_ & Read(addr);
        SetZN(a_);
        break;
    /* BIT nn */
    case 0x24:
    /* BIT nnnn */
    case 0x2C:
        val = Read(addr);
        flags_.v = val >> 6;
        SetZ(val & a_);
        SetN(val);
        break;
    /* ROL nn */
    case 0x26:
    /* ROL nnnn */
    case 0x2E:
    /* ROL nn,X */
    case 0x36:
    /* ROL nnnn,X */
    case 0x3E:
        r = Read(addr);
        r = (r << 1) | flags_.c;
        flags_.c = r >> 8;
        Write(addr, r);
        SetZN(r);
        break;
    /* PLP */
    case 0x28:
        flags_.value = (Pull() & 0xEF) | 0x20;
        break;
    /* ROL A */
    case 0x2A:
        a = (a_ << 1) | flags_.c;
        flags_.c = a_ >> 7;
        a_ = a;
        SetZN(a_);
        break;
    /* BMI nn */
    case 0x30:
        if (flags_.n)
            Branch(addr);
        break;
    /* SEC */
    case 0x38:
        flags_.c = 1;
        break;
    /* RTI */
    case 0x40:
        flags_.value = (Pull() & 0xEF) | 0x20;
        pc_ = Pull16();
        break;
    /* EOR (nn,X) */
    case 0x41:
    /* EOR nn */
    case 0x45:
    /* EOR #nn */
    case 0x49:
    /* EOR nnnn */
    case 0x4D:
    /* EOR (nn,Y) */
    case 0x51:
    /* EOR nn,X */
    case 0x55:
    /* EOR nnnn,Y */
    case 0x59:
    /* EOR nnnn,X */
    case 0x5D:
        a_ = a_ ^ Read(addr);
        SetZN(a_);
        break;
    /* LSR nn */
    case 0x46:
    /* LSR nnnn */
    case 0x4E:
    /* LSR nn,X */
    case 0x56:
    /* LSR nnnn,X */
    case 0x5E:
        val = Read(addr);
        flags_.c = val & 1;
        val >>= 1;
        Write(addr, val);
        SetZN(val);
        break;
    /* PHA */
    case 0x48:
        Push(a_);
        break;
    /* BVC */
    case 0x50:
        if (!flags_.v)
            Branch(addr);
        break;
    /* JMP nnnn */
    case 0x4C:
    /* JMP (nnnn) */
    case 0x6C:
        pc_ = addr;
        break;
    /* LSR A */
    case 0x4A:
        flags_.c = a_ & 1;
        a_ >>= 1;
        SetZN(a_);
        break;
    /* CLI */
    case 0x58:
        flags_.i = 0;
        break;
    /* RTS */
    case 0x60:
        pc_ = Pull16() + 1;
        break;
    /* ADC (nn,X) */
    case 0x61:
    /* ADC nn */
    case 0x65:
    /* ADC #nn */
    case 0x69:
    /* ADC nnnn */
    case 0x6D:
    /* ADC (nn,Y) */
    case 0x71:
    /* ADC nn,X */
    case 0x75:
    /* ADC nnnn,Y */
    case 0x79:
    /* ADC nnnn,X */
    case 0x7D:
        a = a_;
        b = Read(addr);
        r = a + b + flags_.c;
        a_ = r;
        flags_.c = (r > 0xff);
        flags_.v = ((a ^ b) & 0x80) == 0 && ((a ^ a_) & 0x80) != 0;
        SetZN(a_);
        break;
    /* ROR nn */
    case 0x66:
    /* ROR nnnn */
    case 0x6E:
    /* ROR nn,X */
    case 0x76:
    /* ROR nnnn,X */
    case 0x7E:
        val = Read(addr);
        a = (val >> 1) | (flags_.c << 7);
        flags_.c = val & 1;
        Write(addr, a);
        SetZN(a);
        break;
    /* PLA */
    case 0x68:
        a_ = Pull();
        SetZN(a_);
        break;
    /* ROR A */
    case 0x6A:
        a = (a_ >> 1) | (flags_.c << 7);
        flags_.c = a_ & 1;
        a_ = a;
        SetZN(a_);
        break;
    /* BVS */
    case 0x70:
        if (flags_.v)
            Branch(addr);
        break;
    /* SEI */
    case 0x78:
        flags_.i = 1;
        break;
    /* STA (nn,X) */
    case 0x81:
    /* STA nn */
    case 0x85:
    /* STA nnnn */
    case 0x8D:
    /* STA (nn,Y) */
    case 0x91:
    /* STA nn,X */
    case 0x95:
    /* STA nnnn,Y */
    case 0x99:
    /* STA nnnn,X */
    case 0x9D:
        Write(addr, a_);
        break;
    /* STY nn */
    case 0x84:
    /* STY nnnn */
    case 0x8C:
    /* STY nn,X */
    case 0x94:
        Write(addr, y_);
        break;
    /* STX nn */
    case 0x86:
    /* STX nnnn */
    case 0x8E:
    /* STX nn,Y */
    case 0x96:
        Write(addr, x_);
        break;
    /* DEY */
    case 0x88:
        SetZN(--y_);
        break;
    /* TXA */
    case 0x8A:
        a_ = x_;
        SetZN(a_);
        break;
    /* BCC nn */
    case 0x90:
        if (!flags_.c)
            Branch(addr);
        break;
    /* TYA */
    case 0x98:
        a_ = y_;
        SetZN(a_);
        break;
    /* TXS */
    case 0x9A:
        sp_ = x_;
        break;
    /* LDY #nn */
    case 0xA0:
    /* LDY nn */
    case 0xA4:
    /* LDY nnnn */
    case 0xAC:
    /* LDY nn,X */
    case 0xB4:
    /* LDY nnnn,X */
    case 0xBC:
        y_ = Read(addr);
        SetZN(y_);
        break;
    /* LDA (nn,X) */
    case 0xA1:
    /* LDA nn */
    case 0xA5:
    /* LDA #nn */
    case 0xA9:
    /* LDA nnnn */
    case 0xAD:
    /* LDA (nn,Y) */
    case 0xB1:
    /* LDA nn,X */
    case 0xB5:
    /* LDA nnnn,Y */
    case 0xB9:
    /* LDA nnnn,X */
    case 0xBD:
        a_ = Read(addr);
        SetZN(a_);
        break;
    /* LDX #nn */
    case 0xA2:
    /* LDX nn */
    case 0xA6:
    /* LDX nnnn */
    case 0xAE:
    /* LDX nn,Y */
    case 0xB6:
    /* LDX nnnn,Y */
    case 0xBE:
        x_ = Read(addr);
        SetZN(x_);
        break;
    /* TAY */
    case 0xA8:
        y_ = a_;
        SetZN(y_);
        break;
    /* TAX */
    case 0xAA:
        x_ = a_;
        SetZN(x_);
        break;
    /* BCS nn */
    case 0xB0:
        if (flags_.c)
            Branch(addr);
        break;
    /* CLV */
    case 0xB8:
        flags_.v = 0;
        break;
    /* TSX */
    case 0xBA:
        x_ = sp_;
        SetZN(x_);
        break;
    /* CPY #nn */
    case 0xC0:
    /* CPY nn */
    case 0xC4:
    /* CPY nnnn */
    case 0xCC:
        Compare(y_, Read(addr));
        break;
    /* CMP (nn,X) */
    case 0xC1:
    /* CMP nn */
    case 0xC5:
    /* CMP #nn */
    case 0xC9:
    /* CMP nnnn */
    case 0xCD:
    /* CMP (nn,Y) */
    case 0xD1:
    /* CMP nn,X */
    case 0xD5:
    /* CMP nnnn,Y */
    case 0xD9:
    /* CMP nnnn,X */
    case 0xDD:
        Compare(a_, Read(addr));
        break;
    /* DEC nn */
    case 0xC6:
    /* DEC nnnn */
    case 0xCE:
    /* DEC nn,X */
    case 0xD6:
    /* DEC nnnn,X */
    case 0xDE:
        val = Read(addr) - 1;
        Write(addr, val);
        SetZN(val);
        break;
    /* INY */
    case 0xC8:
        SetZN(++y_);
        break;
    /* DEX */
    case 0xCA:
        SetZN(--x_);
        break;
    /* BNE nn */
    case 0xD0:
        if (!flags_.z)
            Branch(addr);
        break;
    /* CLD */
    case 0xD8:
        flags_.d = 0;
        break;
    /* CPX #nn */
    case 0xE0:
    /* CPX nn */
    case 0xE4:
    /* CPX nnnn */
    case 0xEC:
        Compare(x_, Read(addr));
        break;
    /* SBC (nn,X) */
    case 0xE1:
    /* SBC nn */
    case 0xE5:
    /* SBC #nn */
    case 0xE9:
    /* SBC nnnn */
    case 0xED:
    /* SBC (nn,Y) */
    case 0xF1:
    /* SBC nn,X */
    case 0xF5:
    /* SBC nnnn,Y */
    case 0xF9:
    /* SBC nnnn,X */
    case 0xFD:
        a = a_;
        b = Read(addr);
        r = a - b - (1- flags_.c);
        a_ = r;
        flags_.c = (r >= 0);
        flags_.v = ((a ^ r) & 0x80) != 0 && ((a ^ b) & 0x80) != 0;
        SetZN(a_);
        break;
    /* INC nn */
    case 0xE6:
    /* INC nnnn */
    case 0xEE:
    /* INC nn,X */
    case 0xF6:
    /* INC nnnn,X */
    case 0xFE:
        val = Read(addr) + 1;
        Write(addr, val);
        SetZN(val);
        break;
    /* INX */
    case 0xE8:
        SetZN(++x_);
        break;
    /* NOP */
    case 0xEA:
        break;
    /* BEQ nn */
    case 0xF0:
        if (flags_.z)
            Branch(addr);
        break;
    /* SED */
    case 0xF8:
        flags_.d = true;
        break;
    /* Unknown or illegal instruction */
    default:
        printf("Illegal opcode %02x at %04x\n", opcode, fetchpc);
    }
    return cycles_ - cycles;
}

