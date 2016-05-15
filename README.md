# emudore, a Commodore 64 *and NES* emulator

![basic] (/pics/basic.gif "basic")
![zelda] (/pics/zelda.png "zelda")

# What's this?

This is a fork of [Emudore](https://github.com/marioballano/emudore)
enhanced to emulate a [Commodore 64](https://en.wikipedia.org/wiki/Commodore_64)
or a classic [NES](https://en.wikipedia.org/wiki/Nintendo_Entertainment_System).

I've restructured the code a bit and have discarded cmake in favor of bazel.

# Why

The emudore source is wonderful in its straightfoward simplicity and I wanted
to see if I could get it to emulate another 6502-based system.

I few months ago I spent some time learning Nim and ran across the
[Nimes](https://github.com/def-/nimes) NES emulator.  Similarly, Nimes is
elegantly simple and straightfoward and I wanted to see how it would work as
C++.

# Does this even work?

Well, yeah, mostly... 

So far, I've implemented:

* The NES APU and PPU.
* The MMC1 mapper.
* Game controller support (no keyboard!)

