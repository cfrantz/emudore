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
 
#ifndef EMUDORE_MEMORY_H
#define EMUDORE_MEMORY_H 

#include <iostream>
#include <cstdint>

class Memory
{
  public:
    Memory() {}
    virtual ~Memory() {}
    enum kBankCfg
    {
      kROM,
      kRAM,
      kIO
    };
    static const uint16_t kBaseAddrStack = 0x0100;
    static const uint16_t kAddrZeroPage  = 0x0000;
    static const uint16_t kAddrResetVector = 0xfffc;
    static const uint16_t kAddrIRQVector = 0xfffe;
    static const uint16_t kAddrNMIVector = 0xfffa;

    /* read/write memory */
    virtual uint8_t read_byte(uint16_t addr) = 0;
    virtual uint8_t read_byte_no_io(uint16_t addr) = 0;
    virtual void write_byte(uint16_t addr, uint8_t v) = 0;
    virtual void write_byte_no_io(uint16_t addr, uint8_t v) = 0;
    virtual uint16_t read_word(uint16_t addr) = 0;
    virtual uint16_t read_word_no_io(uint16_t) = 0;
    virtual void write_word(uint16_t addr, uint16_t v) = 0;
    virtual void write_word_no_io(uint16_t addr, uint16_t v) = 0;
};

#endif
