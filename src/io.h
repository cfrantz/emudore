/*
 * emudore, Commodore 64 emulator
 * Copyright (c) 2016, Mario Ballano <mballano@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef EMUDORE_IO_H
#define EMUDORE_IO_H

#include <SDL2/SDL.h>
#include <queue>
#include <chrono>
#include <thread>
#include <vector>
#include <utility>
#include <unordered_map>

#include "src/cpu.h"
#include "src/util.h"

/**
 * @brief IO devices
 *
 * This class implements Input/Output devices connected to the 
 * Commodore 64 such as the screen and keyboard.
 *
 * Current backend is SDL2.
 */
class IO
{
  private:
    Cpu *cpu_;
    SDL_Window *window_;
    SDL_Renderer *renderer_;
    SDL_Texture *texture_;
    SDL_PixelFormat *format_;

    std::function<void(uint8_t*, int)> audio_callback_;
    std::function<void(SDL_Event*)> controller_callback_;
    std::function<void(SDL_Event*)> keyboard_callback_;
    std::function<void(SDL_Renderer*)> refresh_callback_;
    uint32_t *frame_;
    size_t cols_;
    size_t rows_;
    double refresh_rate_;
    unsigned int color_palette[16];
    uint8_t keyboard_matrix_[8];
    /* keyboard mappings */
    std::unordered_map<SDL_Keycode,std::pair<int,int>> keymap_;
    std::unordered_map<char,std::vector<SDL_Keycode>> charmap_;
    enum kKeyEvent
    {
      kPress,
      kRelease,
    };
    /* key events */
    std::queue<std::pair<kKeyEvent,SDL_Keycode>> key_event_queue_;
    unsigned int next_key_event_at_;
    static const int kWait = 18000;
    /* vertical refresh sync */
    std::chrono::high_resolution_clock::time_point prev_frame_was_at_;
    void vsync();
    static void AudioCallback(void* userdata, uint8_t* stream, int len);
  public:
    IO(size_t cols, size_t rows, double refresh_rate);
    ~IO();
    bool emulate();
    void cpu(Cpu *v){cpu_=v;};
    void init_color_palette();
    void init_keyboard();
    void init_controllers(std::function<void(SDL_Event*)> callback);
    void handle_keydown(SDL_Keycode k);
    void handle_keyup(SDL_Keycode k);
    void type_character(char c);
    inline uint8_t keyboard_matrix_row(int col){return keyboard_matrix_[col];};
    void screen_update_pixel(int x, int y, int color);
    void screen_draw_rect(int x, int y, int n, int color);
    void screen_draw_border(int y, int color);
    void screen_blit(uint32_t* data);
    void screen_refresh();
    void init_audio(int freq, int chan, int bufsz, SDL_AudioFormat fmt,
                    std::function<void(uint8_t*, int)> callback);
    uint64_t clock_micros();
    void yield();
    inline void set_keyboard_callback(
            std::function<void(SDL_Event*)> callback) {
        keyboard_callback_ = callback;
    }
    inline void set_refresh_callback(
            std::function<void(SDL_Renderer*)> callback) {
        refresh_callback_ = callback;
    }
};

// inline member functions accesible from other classes /////////////////////

inline void IO::screen_update_pixel(int x, int y, int color)
{
  frame_[y * cols_  + x] = color_palette[color & 0xf];
};

#endif
