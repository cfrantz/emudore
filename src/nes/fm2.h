#ifndef EMUDORE_SRC_NES_FM2_H
#define EMUDORE_SRC_NES_FM2_H
#include <string>
#include "src/nes/nes.h"

class FM2Movie {
  public:
    FM2Movie(NES* nes);
    void Load(const std::string& filename);
    void Emulate(int frame);
  private:
    void Parse(const std::string& s);
    NES* nes_;

};

#endif // EMUDORE_SRC_NES_FM2_H
