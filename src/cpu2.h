#ifndef EMUDORE_SRC_CPU2_H
#define EMUDORE_SRC_CPU2_H
#include <functional>
#include <cstdint>
#include <string>
#include "src/memory.h"
#include "proto/cpu6502.pb.h"

class Cpu {
  public:
    Cpu() : Cpu(nullptr) {}
    Cpu(Memory* mem);

    void SaveState(proto::CPU6502 *state);
    void LoadState(proto::CPU6502 *state);
    void Reset();
    int Emulate();
    std::string Disassemble(uint16_t *nexti=nullptr, bool tracemode=false);
    std::string CpuState();
    inline void NMI() {
        nmi_pending_ = true;
        Emit("NMI");
    }
    inline void IRQ() {
        irq_pending_ = true;
        Emit("IRQ");
    }

    inline void reset() { Reset(); }
    inline int emulate() { return Emulate(); }
    inline void nmi() { NMI(); }
    inline void irq() { IRQ(); }
    inline void memory(Memory* mem) { mem_ = mem; }

    inline int cycles() { return cycles_; }

    inline uint8_t a() { return a_; }
    inline uint8_t x() { return x_; }
    inline uint8_t y() { return y_; }
    inline uint8_t sp() { return sp_; }
    inline uint16_t pc() { return pc_; }
    inline void set_pc(uint16_t pc) { pc_ = pc; }

    inline bool cf() { return flags_.c; }
    inline bool zf() { return flags_.z; }
    inline bool idf() { return flags_.i; }
    inline bool dmf() { return flags_.d; }
    inline bool bcf() { return flags_.b; }
    inline bool of() { return flags_.v; }
    inline bool nf() { return flags_.n; }

    union CpuFlags {
        uint8_t value;
        struct {
            uint8_t c:1;
            uint8_t z:1;
            uint8_t i:1;
            uint8_t d:1;
            uint8_t b:1;
            uint8_t u:1;
            uint8_t v:1;
            uint8_t n:1;
        };
    };

    union InstructionInfo {
        uint16_t value;
        struct {
            uint16_t mode:4;
            uint16_t size:4;
            uint16_t cycles:4;
            uint16_t page:4;
        };
    };

    enum AddressingMode {
        Absolute,
        AbsoluteX,
        AbsoluteY,
        Accumulator,
        Immediate,
        Implied,
        IndexedIndirect,
        Indirect,
        IndirectIndexed,
        Relative,
        ZeroPage,
        ZeroPageX,
        ZeroPageY,
    };

    inline void set_write_cb(std::function<void(Cpu*, uint16_t, uint8_t)> cb) {
        write_cb_ = cb;
    }
    inline void set_exec_cb(std::function<void(Cpu*, uint16_t, uint8_t)> cb) {
        exec_cb_ = cb;
    }
    inline void set_read_cb(std::function<void(Cpu*, uint16_t, uint8_t)> cb) {
        read_cb_ = cb;
    }
  private:
    uint8_t inline Read(uint16_t addr) {
        uint8_t val = mem_->read_byte(addr);
        if (read_cb_) read_cb_(this, addr, val);
        return val;
    }
    void inline Write(uint16_t addr, uint8_t val) {
        if (write_cb_) write_cb_(this, addr, val);
        mem_->write_byte(addr, val);
    }
    uint16_t inline Read16(uint16_t addr) {
        return Read(addr) | Read(addr+1) << 8;
    }
    uint16_t inline Read16Bug(uint16_t addr) {
        // When reading the high byte of the word, the address
        // increments, but doesn't carry from the low address byte to the
        // high address byte.
        uint16_t ret = Read(addr);
        ret |= Read((addr & 0xFF00) | ((addr+1) & 0x00FF)) << 8;
        return ret;
    }

    inline void Push(uint8_t val) { Write(sp_-- | 0x100, val); }
    inline uint8_t Pull() { return Read(++sp_ | 0x100); }

    inline void Push16(uint16_t val) { Push(val>>8); Push(val); }
    inline uint16_t Pull16() { return Pull() | Pull() << 8; }

    inline void SetZ(uint8_t val) { flags_.z = (val == 0); }
    inline void SetN(uint8_t val) { flags_.n = !!(val & 0x80); }
    inline void SetZN(uint8_t val) { SetZ(val); SetN(val); }
    inline void Compare(uint8_t a, uint8_t b) {
        SetZN(a - b);
        flags_.c = (a >= b);
    }
    inline bool PagesDiffer(uint16_t a, uint16_t b) {
        return (a & 0xFF00) != (b & 0xFF00);
    }
    void Branch(uint16_t addr);
    /*
    inline void Branch(uint16_t addr) {
        if (PagesDiffer(pc_, addr))
            cycles_++;
        pc_ = addr;
        cycles_++;
    }
    */

    Memory* mem_;
    CpuFlags flags_;
    uint16_t pc_;
    uint8_t sp_;
    uint8_t a_, x_, y_;
    
    uint64_t cycles_;
    int stall_;
    bool nmi_pending_;
    bool irq_pending_;

    static const InstructionInfo info_[256];
    static const char* instruction_names_[256];

    void Flush();
    void Emit(const char *buf, int how=0);
    void Trace();

    static const int TRACEBUFSZ = 1000000;
    static const int SLOP = 1000;
    char tracebuf_[TRACEBUFSZ][80];
    int tbptr_;
    bool halted_;
    std::function<void(Cpu*, uint16_t, uint8_t)> write_cb_;
    std::function<void(Cpu*, uint16_t, uint8_t)> exec_cb_;
    std::function<void(Cpu*, uint16_t, uint8_t)> read_cb_;;
};

#endif // EMUDORE_SRC_CPU2_H
