#include <cstdint>
#include "src/nes/controller.h"

Controller::Controller(NES* nes, int cnum) :
    nes_(nes),
    buttons_(0),
    index_(0),
    strobe_(0),
    movie_(0),
    got_read_(false),
    cnum_(cnum)
{
}

uint8_t Controller::Read() {
    uint8_t ret = 0;
    got_read_ = true;
    if (index_ < 8)
        ret = (buttons_ >> index_) & 1;
    index_++;
    if (strobe_ & 1)
        index_ = 0;
    return ret;
}

void Controller::Write(uint8_t val) {
    strobe_ = val;
    if (strobe_ & 1)
        index_ = 0;
}

void Controller::set_buttons(SDL_Event* event) {
    if (event->type == SDL_CONTROLLERBUTTONDOWN) {
        switch(event->cbutton.button) {
            case SDL_CONTROLLER_BUTTON_DPAD_UP:
                buttons_ |= BUTTON_UP; break;
            case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
                buttons_ |= BUTTON_DOWN; break;
            case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
                buttons_ |= BUTTON_LEFT; break;
            case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
                buttons_ |= BUTTON_RIGHT; break;
            case SDL_CONTROLLER_BUTTON_A:
                // The xbox button names for A and B are opposite their
                // classic NES A and B button positions.
                buttons_ |= BUTTON_B; break;
            case SDL_CONTROLLER_BUTTON_B:
                // The xbox button names for A and B are opposite their
                // classic NES A and B button positions.
                buttons_ |= BUTTON_A; break;
            case SDL_CONTROLLER_BUTTON_BACK:
                buttons_ |= BUTTON_SELECT; break;
            case SDL_CONTROLLER_BUTTON_START:
                buttons_ |= BUTTON_START; break;
        }
    }
    else if (event->type == SDL_CONTROLLERBUTTONUP) {
        switch(event->cbutton.button) {
            case SDL_CONTROLLER_BUTTON_DPAD_UP:
                buttons_ &= ~BUTTON_UP; break;
            case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
                buttons_ &= ~BUTTON_DOWN; break;
            case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
                buttons_ &= ~BUTTON_LEFT; break;
            case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
                buttons_ &= ~BUTTON_RIGHT; break;
            case SDL_CONTROLLER_BUTTON_A:
                // The xbox button names for A and B are opposite their
                // classic NES A and B button positions.
                buttons_ &= ~BUTTON_B; break;
            case SDL_CONTROLLER_BUTTON_B:
                // The xbox button names for A and B are opposite their
                // classic NES A and B button positions.
                buttons_ &= ~BUTTON_A; break;
            case SDL_CONTROLLER_BUTTON_BACK:
                buttons_ &= ~BUTTON_SELECT; break;
            case SDL_CONTROLLER_BUTTON_START:
                buttons_ &= ~BUTTON_START; break;
        }
    } else if (event->type == SDL_CONTROLLERAXISMOTION) {
        if (event->caxis.axis == 0) {
            if (event->caxis.value < -3000) {
                buttons_ |= BUTTON_LEFT;
                buttons_ &= ~BUTTON_RIGHT;
            } else if (event->caxis.value > 3000) {
                buttons_ &= ~BUTTON_LEFT;
                buttons_ |= BUTTON_RIGHT;
            } else {
                buttons_ &= ~BUTTON_LEFT;
                buttons_ &= ~BUTTON_RIGHT;
            }
        } else if (event->caxis.axis == 1) {
            if (event->caxis.value < -3000) {
                buttons_ |= BUTTON_UP;
                buttons_ &= ~BUTTON_DOWN;
            } else if (event->caxis.value > 3000) {
                buttons_ &= ~BUTTON_UP;
                buttons_ |= BUTTON_DOWN;
            } else {
                buttons_ &= ~BUTTON_UP;
                buttons_ &= ~BUTTON_DOWN;
            }
        }
    }
}

void Controller::AppendButtons(uint8_t b) {
    movie_.push_back(b);
}

void Controller::Emulate(int frame) {
    if (frame < int(movie_.size())) {
        buttons_ = movie_.at(frame);
        if (!got_read_ && cnum_ == 0) {
            printf("Missed controller read @ %d\n", frame-1);
        }
//        if (buttons_) {
//            printf("Press %02x at %d\n", buttons_, frame);
//        }
    }

}
