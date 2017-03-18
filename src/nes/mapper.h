#ifndef EMUDORE_SRC_NES_MAPPER_H
#define EMUDORE_SRC_NES_MAPPER_H
#include <functional>
#include <map>
#include <cstdint>
#include "src/nes/nes.h"
#include "proto/mappers.pb.h"

class Mapper {
  public:
    Mapper(NES* nes) : nes_(nes) {}
    virtual uint8_t Read(uint16_t addr) = 0;
    virtual void ReadChr2(uint16_t addr, uint8_t* a, uint8_t* b) {
        *a = Read(addr);
        *b = Read(addr + 8);
    }
    virtual void Write(uint16_t addr, uint8_t val) = 0;
    virtual void Emulate() {}
    virtual void DebugStuff() {}
    virtual void LoadState(proto::Mapper *state) {}
    virtual void SaveState(proto::Mapper *state) {}
  protected:
    NES* nes_;
};

class MapperRegistry {
  public:
    MapperRegistry(int n, std::function<Mapper*(NES*)> create);
    static Mapper* New(NES* nes, int n);
  private:
    static std::map<int, std::function<Mapper*(NES*)>>* mappers();
};

#define CONCAT_(x, y) x ## y
#define CONCAT(x, y) CONCAT_(x, y)

#define REGISTER_MAPPER(n_, type_) \
    MapperRegistry CONCAT(CONCAT(CONCAT(reg_, type_), __), __LINE__) (n_, \
            [](NES* nes) { return new type_(nes); })

#endif // EMUDORE_SRC_NES_MAPPER_H
