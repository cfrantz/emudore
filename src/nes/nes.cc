
#include <gflags/gflags.h>
#include <unistd.h>
#include "imgui.h"
#include "google/protobuf/text_format.h"

#include "src/nes/nes.h"

#include "src/cpu2.h"
#include "src/io.h"
#include "src/nes/apu.h"
#include "src/nes/cartridge.h"
#include "src/nes/controller.h"
#include "src/nes/debug_console.h"
#include "src/nes/fm2.h"
#include "src/nes/mapper.h"
#include "src/nes/mem.h"
#include "src/nes/ppu.h"
#include "src/sdlutil/gfx.h"

DEFINE_string(fm2, "", "FM2 Movie file.");
DEFINE_double(fps, 60.0988, "Desired NES fps.");

using namespace std::placeholders;

const uint32_t standard_palette[] = {
    0xFF666666, 0xFF002A88, 0xFF1412A7, 0xFF3B00A4,
    0xFF5C007E, 0xFF6E0040, 0xFF6C0600, 0xFF561D00,
    0xFF333500, 0xFF0B4800, 0xFF005200, 0xFF004F08,
    0xFF00404D, 0xFF000000, 0xFF000000, 0xFF000000,
    0xFFADADAD, 0xFF155FD9, 0xFF4240FF, 0xFF7527FE,
    0xFFA01ACC, 0xFFB71E7B, 0xFFB53120, 0xFF994E00,
    0xFF6B6D00, 0xFF388700, 0xFF0C9300, 0xFF008F32,
    0xFF007C8D, 0xFF000000, 0xFF000000, 0xFF000000,
    0xFFFFFEFF, 0xFF64B0FF, 0xFF9290FF, 0xFFC676FF,
    0xFFF36AFF, 0xFFFE6ECC, 0xFFFE8170, 0xFFEA9E22,
    0xFFBCBE00, 0xFF88D800, 0xFF5CE430, 0xFF45E082,
    0xFF48CDDE, 0xFF4F4F4F, 0xFF000000, 0xFF000000,
    0xFFFFFEFF, 0xFFC0DFFF, 0xFFD3D2FF, 0xFFE8C8FF,
    0xFFFBC2FF, 0xFFFEC4EA, 0xFFFECCC5, 0xFFF7D8A5,
    0xFFE4E594, 0xFFCFEF96, 0xFFBDF4AB, 0xFFB3F3CC,
    0xFFB5EBF2, 0xFFB8B8B8, 0xFF000000, 0xFF000000,
};

NES::NES() :
    pause_(false),
    step_(false),
    debug_(false),
    reset_(false),
    stall_(0),
    frame_(0)
{
    cpu_ = new Cpu();
    cart_ = new Cartridge(this);
    controller_[0] = new Controller(this, 0);
    controller_[1] = new Controller(this, 1);
    controller_[2] = new Controller(this, 2);
    controller_[3] = new Controller(this, 3);
    apu_ = new APU(this);
    mapper_ = nullptr;
    mem_ = new Mem(this);
    movie_ = new FM2Movie(this);
    ppu_ = new PPU(this);
    io_ = new IO(256, 240, FLAGS_fps);

    io_->init_audio(44100, 1, APU::BUFFERLEN/2, AUDIO_F32,
            [this](uint8_t* stream, int len) {
                apu_->PlayBuffer(stream, len); });
    io_->init_controllers(
            [this](SDL_Event* event) { controller_[0]->set_buttons(event); });
    io_->set_refresh_callback([this](SDL_Renderer* r) { DebugStuff(r); });
    io_->set_keyboard_callback(
            [this](SDL_Event* event) { HandleKeyboard(event); });
#if 0
    debugger_ = new Debugger();
    debugger_->cpu(cpu_);
    debugger_->memory(mem_);
#endif
    cpu_->memory(mem_);
    for(size_t i=0; i<sizeof(palette_)/sizeof(palette_[0]); i++) {
        palette_[i] = (standard_palette[i] & 0xFF00FF00) |
                      ((standard_palette[i] >> 16 ) & 0xFF) |
                      ((standard_palette[i] & 0xFF) << 16);
    }
    cpu_->set_write_cb(std::bind(&NES::Watcher, this,
        "PC=%04x wrote %02x to %04x.  A=%02x X=%02x Y=%02x SP=%04x[%04x]",
        &watches_, _1, _2, _3));

    cpu_->set_exec_cb(std::bind(&NES::Watcher, this,
        "PC=%04x Exec %02x, inst=%02x.  A=%02x X=%02x Y=%02x SP=%04x[%04x]",
        &xwatches_, _1, _2, _3));
    cpu_->set_read_cb(std::bind(&NES::Watcher, this,
        "PC=%04x Exec %02x, inst=%02x.  A=%02x X=%02x Y=%02x SP=%04x[%04x]",
        &rwatches_, _1, _2, _3));

    console_.RegisterCommand("db", "Hexdump bytes", [=](int argc, char **argv){
        this->HexdumpBytes(argc, argv);
    });
    console_.RegisterCommand("dc", "Hexdump CHR bytes", [=](int argc, char **argv){
        this->HexdumpBytes(argc, argv);
    });
    console_.RegisterCommand("dp", "Hexdump PRG bytes", [=](int argc, char **argv){
        this->HexdumpBytes(argc, argv);
    });
    console_.RegisterCommand("wb", "Write bytes", [=](int argc, char **argv){
        this->WriteBytes(argc, argv);
    });
    console_.RegisterCommand("wc", "Write CHR memory", [=](int argc, char **argv){
        this->WriteBytes(argc, argv);
    });
    console_.RegisterCommand("wp", "Write PRG memory", [=](int argc, char **argv){
        this->WriteBytes(argc, argv);
    });
    console_.RegisterCommand("wi", "Write Bytes Incrementing", [=](int argc, char **argv){
        this->WriteBytesInc(argc, argv);
    });
    console_.RegisterCommand("dw", "Hexdump words", [=](int argc, char **argv){
        this->HexdumpWords(argc, argv);
    });
    console_.RegisterCommand("ww", "Write words", [=](int argc, char **argv){
        this->WriteBytes(argc, argv);
    });
    console_.RegisterCommand("nail", "Nail byte value", [=](int argc, char **argv){
        this->NailByte(argc, argv);
    });
    console_.RegisterCommand("unnail", "Cancel a nailed byte", [=](int argc, char **argv){
        this->UnnailByte(argc, argv);
    });
    console_.RegisterCommand("mm", "Print mirror mode", [=](int argc, char **argv){
        console_.AddLog("mirror: %d", this->cart_->mirror());
    });
    console_.RegisterCommand("reset", "Reset console", [=](int argc, char **argv){
        this->Reset();
    });
    console_.RegisterCommand("u", "Unassemble", [=](int argc, char **argv){
        this->Unassemble(argc, argv);
    });
    console_.RegisterCommand("abort", "Quit Immediatelyt", [=](int argc, char **argv){
        abort();
    });
    console_.RegisterCommand("z2cheat", "Zelda2 Cheat", [=](int argc, char **argv){
        Z2Cheat(argc, argv);
    });
    console_.RegisterCommand("find", "Find byte in RAM", [=](int argc, char **argv){
        this->Find(argc, argv);
    });
    console_.RegisterCommand("ls", "Load State", [=](int argc, char **argv){
        this->CmdLoadState(argc, argv);
    });
    console_.RegisterCommand("ss", "Save State", [=](int argc, char **argv){
        this->CmdSaveState(argc, argv);
    });
    console_.RegisterCommand("setw", "Set a write watch", std::bind(&NES::SetWatch, this, _1, _2));
    console_.RegisterCommand("delw", "Del a write watch", std::bind(&NES::DelWatch, this, _1, _2));

    console_.RegisterCommand("setwx", "Set an exec watch", std::bind(&NES::SetWatch, this, _1, _2));
    console_.RegisterCommand("delwx", "Del an exec watch", std::bind(&NES::DelWatch, this, _1, _2));
    console_.RegisterCommand("setwr", "Set an read watch", std::bind(&NES::SetWatch, this, _1, _2));
    console_.RegisterCommand("delwr", "Del an read watch", std::bind(&NES::DelWatch, this, _1, _2));
}

void NES::LoadFile(const std::string& filename) {
    cart_->LoadFile(filename);
    mapper_ = MapperRegistry::New(this, cart_->mapper());
    if (!FLAGS_fm2.empty()) {
        movie_->Load(FLAGS_fm2);
    }
}

void NES::CmdLoadState(int argc, char **argv) {
    if (argc < 2) {
        console_.AddLog("[error] %s: Wrong number of arguments.", argv[0]);
        console_.AddLog("[error] %s <filename>", argv[0]);
    }
    std::string file(argv[1]);
    LoadState(file);
}

void NES::CmdSaveState(int argc, char **argv) {
    if (argc < 2) {
        console_.AddLog("[error] %s: Wrong number of arguments.", argv[0]);
        console_.AddLog("[error] %s <filename> [text]", argv[0]);
    }
    std::string file(argv[1]);
    bool text = false;
    if (argc == 3 && argv[2][0] == 't')
        text = true;

    SaveState(file, text);
}

void NES::LoadState(const std::string& filename) {
    FILE* fp;
    std::string data;
    if ((fp = fopen(filename.c_str(), "rb")) != nullptr) {
        fseek(fp, 0, SEEK_END);
        data.resize(ftell(fp));
        fseek(fp, 0, SEEK_SET);
        fread(&data.front(), 1, data.size(), fp);
        fclose(fp);
    } else {
        console_.AddLog("[error] Could not LoadState from %s",
                        filename.c_str());
    }

    state_.Clear();
    if (!state_.ParseFromString(data)) {
        if (!google::protobuf::TextFormat::ParseFromString(data, &state_)) {
            console_.AddLog("[error] Could not parse data from %s",
                            filename.c_str());
            return;
        }
    }

    apu_->LoadState(state_.mutable_apu());
    cpu_->LoadState(state_.mutable_cpu());
    mem_->LoadState(&state_);
    ppu_->LoadState(state_.mutable_ppu());
    mapper_->LoadState(state_.mutable_mapper());
    cart_->LoadState(state_.mutable_mapper());
}

void NES::SaveState(const std::string& filename, bool text) {
    apu_->SaveState(state_.mutable_apu());
    cpu_->SaveState(state_.mutable_cpu());
    mem_->SaveState(&state_);
    ppu_->SaveState(state_.mutable_ppu());
    mapper_->SaveState(state_.mutable_mapper());
    cart_->SaveState(state_.mutable_mapper());

    std::string data;
    if (text) {
        google::protobuf::TextFormat::PrintToString(state_, &data);
    } else {
        state_.SerializeToString(&data);
    }
    FILE* fp;
    if ((fp = fopen(filename.c_str(), "wb")) != nullptr) {
        fwrite(data.data(), 1, data.size(), fp);
        fclose(fp);
    } else {
        console_.AddLog("[error] Could not SaveState to %s", filename.c_str());
    }
}

void NES::DebugPalette(bool* active) {
    int i, x, y;;
    static ImVec4 pal[64];
    static char label[64][16];
    static bool once;

    if (!*active)
        return;

    if (!once) {
        for(i=0; i<64; i++) {
            uint8_t b = palette_[i] >> 16;
            uint8_t g = palette_[i] >> 8;
            uint8_t r = palette_[i] >> 0;
            pal[i] = ImColor(r, g, b);
            sprintf(label[i], "Edit Color %02x", i);
        }
        once = true;
    }

    ImGui::Begin("Hardware Palette", active);
    ImGui::Text("Right click to edit colors");
    for(x=0; x<16; x++) {
        if (x) {
            ImGui::SameLine();
        }
        ImGui::BeginGroup();
        ImGui::Text(x==0 ? "   %02x" : "%02x", x);
        for(y=0; y<4; y++) {
            if (x == 0) {
                ImGui::Text("%x0", y);
                ImGui::SameLine();
            }
            i = y*16+x;
            ImGui::ColorButton(pal[i]);
            if (ImGui::BeginPopupContextItem(label[i])) {
                ImGui::Text("Edit color");
                ImGui::ColorEdit3("##edit", (float*)&pal[i]);
                if (ImGui::Button("Close"))
                    ImGui::CloseCurrentPopup();
                ImGui::EndPopup();
                palette_[i] = 0xFF000000 |
                    int(255*pal[i].z)<<16 |
                    int(255*pal[i].y)<<8 |
                    int(255*pal[i].x) ;
            }

        }
        ImGui::EndGroup();
    }
    ImGui::End();
}

void NES::DebugStuff(SDL_Renderer* r) {
    static bool palette_editor, debug_console;

    ImGui::Text("Frame: %d", int(ppu_->frame()));
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Console")) {
            ImGui::MenuItem("Palette Editor", nullptr, &palette_editor);
            ImGui::MenuItem("Debug Console", nullptr, &debug_console);
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    DebugPalette(&palette_editor);
    if (debug_console) {
        console_.Draw("Debug Console", &debug_console);
    }
    mem_->DebugStuff();
    apu_->DebugStuff();
    ppu_->DebugStuff();
    controller_[0]->DebugStuff();
    ImGui::SameLine();
    controller_[1]->DebugStuff();
}

void NES::HandleKeyboard(SDL_Event *event) {
    if (event->type == SDL_KEYUP) {
        switch(event->key.keysym.scancode) {
        case SDL_SCANCODE_PAUSE:
            pause_ = !pause_;
            break;
        case SDL_SCANCODE_BACKSLASH:
            pause_ = true; step_ = true;
            break;
        case SDL_SCANCODE_PERIOD:
            debug_ = !debug_;
            break;
        case SDL_SCANCODE_F11:
            Reset();
            break;
        default:
            ;
        }
    }
}

int NES::cpu_cycles() {
    return int(cpu_->cycles());
}

void NES::Reset() {
    cpu_->reset();
    ppu_->Reset();
}

bool NES::Emulate() {
#if 0
    if (!debugger_->emulate())
        return false;
#endif

    const int n = cpu_->Emulate();
    for(int i=0; i<n*3; i++) {
        // The PPU is clocked at 3 dots per CPU clock
        ppu_->Emulate();
        mapper_->Emulate();
        cart_->Emulate();
    }
    for(int i=0; i<n; i++) {
        apu_->Emulate();
    }
    return true;
}

bool NES::EmulateFrame() {
    frame_ = ppu_->frame();

    movie_->Emulate(frame_);
    while(frame_ == ppu_->frame()) {
        for(const auto& n : nailed_)
            mem_->Write(n.first, n.second);

        if (!Emulate())
            return false;
    }
    return true;
}

void NES::Run() {
#if 0
    uint64_t t0, t1;
    int64_t ns;

    t0 = io_->clock_nanos();
#endif

    Reset();
    for(;;) {
        io_->screen_refresh();

#if 0
        // This is ifdef'd out in favor of sleeping in the audio loop
        // to lock to the right frame rate.
        t1 = io_->clock_nanos();
        ns = int64_t(1e9 / FLAGS_fps) - (t1 - t0);
        if (ns > 200) {
            sleep_nanos(ns - 100);
        }
        t0 = io_->clock_nanos();
#endif

        if (!io_->emulate())
            break;

        if (pause_) {
            if (!step_)
                continue;
            step_ = false;
        }
        EmulateFrame();
    }
}

void NES::IRQ() {
    cpu_->irq();
}

void NES::NMI() {
    cpu_->nmi();
}

void NES::HexdumpBytes(int argc, char **argv) {
    if (argc < 2) {
        console_.AddLog("[error] %s: Wrong number of arguments.", argv[0]);
        console_.AddLog("[error] %s <addr> <length>", argv[0]);
        return;
    }

    uint16_t addr = strtoul(argv[1], 0, 0);
    int len = (argc == 3) ? strtoul(argv[2], 0, 0) : 64;

    char line[128], chr[17];
    int i, n;
    uint8_t val;

    for(i=n=0; i < len; i++) {
        if (argv[0][1] == 'c')
            val = cart_->ReadChr(addr+i);
        else if (argv[0][1] == 'p')
            val = cart_->ReadPrg(addr+i);
        else
            val = mem_->read_byte_no_io(addr+i);

        if (i % 16 == 0) {
            if (i) {
                n += sprintf(line+n, "  %s", chr);
                console_.AddLog("%s", line);
            }
            n = sprintf(line, "%04x: ", addr+i);
            memset(chr, 0, sizeof(chr));
        }
        n += sprintf(line+n, " %02x", val);
        chr[i%16] = (val>=32 && val<127) ? val : '.';
    }
    if (i % 16) {
        i = 3*(16 - i%16);
    } else {
        i = 0;
    }
    n += sprintf(line+n, " %*c%s", i, ' ', chr);
    console_.AddLog("%s", line);
}

void NES::WriteBytes(int argc, char **argv) {
    if (argc < 3) {
        console_.AddLog("[error] %s: Wrong number of arguments.", argv[0]);
        console_.AddLog("[error] %s <addr> <val> ...", argv[0]);
        return;
    }

    uint16_t addr = strtoul(argv[1], 0, 0);
    for(int i=2; i<argc; i++) {
        uint8_t val = strtoul(argv[i], 0, 0);
        if (argv[0][1] == 'b')
            mem_->Write(addr++, val);
        else if (argv[0][1] == 'c')
            cart_->WriteChr(addr++, val);
        else if (argv[0][1] == 'p')
            cart_->WritePrg(addr++, val);
    }
}

void NES::WriteBytesInc(int argc, char **argv) {
    if (argc < 4) {
        console_.AddLog("[error] %s: Wrong number of arguments.", argv[0]);
        console_.AddLog("[error] %s <addr> <sval> <len>", argv[0]);
        return;
    }
    uint16_t addr = strtoul(argv[1], 0, 0);
    uint8_t val = strtoul(argv[2], 0, 0);
    uint16_t len = strtoul(argv[3], 0, 0);
    for(unsigned i=0; i<len; i++) {
        mem_->Write(addr+i, val+i);
    }
}


void NES::HexdumpWords(int argc, char **argv) {
    if (argc < 2) {
        console_.AddLog("[error] %s: Wrong number of arguments.", argv[0]);
        console_.AddLog("[error] %s <addr> <length>", argv[0]);
        return;
    }

    uint16_t addr = strtoul(argv[1], 0, 0);
    int len = (argc == 3) ? strtoul(argv[2], 0, 0) : 64;

    char line[128], chr[17];
    int i, n;
    uint8_t val;

    for(i=n=0; i < len; i+=2) {
        if (i % 16 == 0) {
            if (i) {
                n += sprintf(line+n, "  %s", chr);
                console_.AddLog("%s", line);
            }
            n = sprintf(line, "%04x: ", addr+i);
            memset(chr, 0, sizeof(chr));
        }
        val = mem_->read_byte_no_io(addr+i+1);
        n += sprintf(line+n, " %02x", val);
        chr[i%16] = (val>=32 && val<127) ? val : '.';

        val = mem_->read_byte_no_io(addr+i);
        n += sprintf(line+n, "%02x", val);
        chr[(i+1)%16] = (val>=32 && val<127) ? val : '.';
    }
    if (i % 16) {
        i = 3*(16 - i%16);
    } else {
        i = 0;
    }
    n += sprintf(line+n, " %*c%s", i, ' ', chr);
    console_.AddLog("%s", line);
}

void NES::WriteWords(int argc, char **argv) {
    if (argc < 3) {
        console_.AddLog("[error] %s: Wrong number of arguments.", argv[0]);
        console_.AddLog("[error] %s <addr> <val> ...", argv[0]);
        return;
    }

    uint16_t addr = strtoul(argv[1], 0, 0);
    for(int i=2; i<argc; i++) {
        uint16_t val = strtoul(argv[i], 0, 0);
        mem_->Write(addr++, val);
        mem_->Write(addr++, val>>8);
    }
}

void NES::NailByte(int argc, char **argv) {
    if (argc < 3) {
        console_.AddLog("[error] %s: Wrong number of arguments.", argv[0]);
        console_.AddLog("[error] %s <addr> <val> ...", argv[0]);
        for(const auto& n : nailed_)
            console_.AddLog("  %04x: %02x", n.first, n.second);
        return;
    }

    uint16_t addr = strtoul(argv[1], 0, 0);
    for(int i=2; i<argc; i++) {
        uint8_t val = strtoul(argv[i], 0, 0);
        nailed_[addr++] = val;
    }
}

void NES::Z2Cheat(int argc, char **argv) {
    console_.ExecCommand("wb 0x777 8 8 8");
    console_.ExecCommand("wb 0x77b 1 1 1 1 1 1 1 1 8 8 1 1 1 1 1 1 1 1");
    console_.ExecCommand("wb 0x796 0x14");
    console_.ExecCommand("nail 0x773 255 255");
}

void NES::UnnailByte(int argc, char **argv) {
    if (argc < 2) {
        console_.AddLog("[error] %s: Wrong number of arguments.", argv[0]);
        console_.AddLog("[error] %s <addr> ...", argv[0]);
        for(const auto& n : nailed_)
            console_.AddLog("  %04x: %02x", n.first, n.second);
        return;
    }

    for(int i=1; i<argc; i++) {
        uint16_t addr = strtoul(argv[i], 0, 0);
        nailed_.erase(addr);
    }
}

void NES::Unassemble(int argc, char **argv) {
    static uint16_t addr = 0;

    if (addr == 0) {
        addr = mem_->read_word(0xFFFC);
    }
    if (argc >= 2) {
        addr = strtoul(argv[1], 0, 0);
    }

    int len = (argc == 3) ? strtol(argv[2], 0, 0) : 10;
    for(int i=0; i<len; i++) {
        std::string s = cpu_->Disassemble(&addr);
        console_.AddLog("%s", s.c_str());
    }
}

void NES::Find(int argc, char **argv) {
    if (argc < 1) {
        console_.AddLog("[error] %s: Wrong number of arguments.", argv[0]);
        console_.AddLog("[error] %s <byte>", argv[0]);
        return;
    }

    uint8_t byte = strtoul(argv[1], 0, 0);
    for(int i=0; i<2048; i++) {
        uint8_t val = mem_->read_byte_no_io(i);
        if (val == byte) {
            console_.AddLog("%04x: %02x", i, byte);
        }
    }
    for(int i=0x6000; i<0x8000; i++) {
        uint8_t val = mem_->read_byte_no_io(i);
        if (val == byte) {
            console_.AddLog("%04x: %02x", i, byte);
        }
    }
}

void NES::SetWatch(int argc, char **argv) {
    if (argc < 2) {
        console_.AddLog("[error] %s: Wrong number of arguments.", argv[0]);
        console_.AddLog("[error] %s <addr>", argv[0]);
        return;
    }
    char type = argv[0][4] ? argv[0][4] : 'w';
    auto* watch = &watches_;
    if (type == 'x') watch = &xwatches_;
    if (type == 'r') watch = &rwatches_;

    Watch w = {0, -1};
    w.addr = strtoul(argv[1], 0, 0);
    if (argc == 3)
        w.val = strtoul(argv[2], 0, 0);

    watch->push_back(w);
    console_.AddLog("Watches (type=%c):", type);
    for(unsigned int i=0; i<watch->size(); i++) {
        w = watch->at(i);
        console_.AddLog("%u: %04x = %x", i, w.addr, w.val);
    }
}

void NES::DelWatch(int argc, char **argv) {
    if (argc < 1) {
        console_.AddLog("[error] %s: Wrong number of arguments.", argv[0]);
        console_.AddLog("[error] %s <addr>", argv[0]);
        return;
    }
    char type = argv[0][4] ? argv[0][4] : 'w';
    auto* watch = &watches_;
    if (type == 'x') watch = &xwatches_;
    if (type == 'r') watch = &rwatches_;

    uint16_t n = strtoul(argv[1], 0, 0);
    auto it = watch->begin() + n;
    if (it < watch->end()) watch->erase(it);
    console_.AddLog("Watches (type=%c):", type);
    for(unsigned int i=0; i<watch->size(); i++) {
        auto w = watch->at(i);
        console_.AddLog("%u: %04x = %x", i, w.addr, w.val);
    }
}

void NES::Watcher(const char *msg, const std::vector<Watch>* watch,
                  Cpu* cpu, uint16_t addr, uint8_t val) {
    for(const auto w : *watch) {
        if (w.addr == addr && (w.val == -1 || w.val == val)) {
            uint16_t sp = 0x100 | cpu->sp();
            uint16_t tos = mem_->Read(sp+1) | mem_->Read(sp+2) << 8;
            console_.AddLog(msg,
                    cpu->pc(), val, addr, cpu->a(), cpu->x(), cpu->y(), sp, tos);
        }
    }
}
