#ifndef EMUDORE_SRC_NES_CONTROLLER_H
#define EMUDORE_SRC_NES_CONTROLLER_H
#include <cstdint>
#include <vector>
#include <SDL2/SDL.h>
#include "src/nes/nes.h"

class Controller {
  public:
    Controller(NES* nes);
    uint8_t Read();
    void Write(uint8_t val);
    inline uint8_t buttons() { return buttons_; }
    inline void set_buttons(uint8_t b) { buttons_ = b; }
    void set_buttons(SDL_Event* event);
    void AppendButtons(uint8_t b);
    void Emulate(int frame);

    static const int BUTTON_A      = 0x01;
    static const int BUTTON_B      = 0x02;
    static const int BUTTON_SELECT = 0x04;
    static const int BUTTON_START  = 0x08;
    static const int BUTTON_UP     = 0x10;
    static const int BUTTON_DOWN   = 0x20;
    static const int BUTTON_LEFT   = 0x40;
    static const int BUTTON_RIGHT  = 0x80;
  private:
    NES* nes_;
    uint8_t buttons_;
    int index_, strobe_;
    std::vector<uint8_t> movie_;
};


#endif // EMUDORE_SRC_NES_CONTROLLER_H
