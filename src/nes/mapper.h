#ifndef EMUDORE_SRC_NES_MAPPER_H
#define EMUDORE_SRC_NES_MAPPER_H
#include <functional>
#include <map>
#include <cstdint>
#include "src/nes/nes.h"

class Mapper {
  public:
    Mapper(NES* nes) : nes_(nes) {}
    virtual uint8_t Read(uint16_t addr) = 0;
    virtual void Write(uint16_t addr, uint8_t val) = 0;
    virtual void Emulate() {};
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
    MapperRegistry CONCAT(reg__, __LINE__) (n_, \
            [](NES* nes) { return new type_(nes); })

#endif // EMUDORE_SRC_NES_MAPPER_H
