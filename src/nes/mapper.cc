#include "src/nes/mapper.h"

std::map<int, std::function<Mapper*(NES*)>>* MapperRegistry::mappers() {
    static std::map<int, std::function<Mapper*(NES*)>> reg;
    return &reg;
}

MapperRegistry::MapperRegistry(int n, std::function<Mapper*(NES*)> create) {
    mappers()->insert(std::make_pair(n, create));
}

Mapper* MapperRegistry::New(NES* nes, int n) {
    const auto& mi = mappers()->find(n);
    if (mi == mappers()->end()) {
        return nullptr;
    }
    return mi->second(nes);
}
