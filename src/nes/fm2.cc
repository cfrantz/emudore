#include <gflags/gflags.h>

#include "src/nes/fm2.h"
#include "src/nes/controller.h"

DEFINE_int32(fm2_predelay, 0, "Number of frames of pre-delay on fm2 inputs.");

FM2Movie::FM2Movie(NES* nes) :
    nes_(nes) {}

void FM2Movie::Emulate(int frame) {
    for(int i=0; i<nes_->controller_size(); i++) {
        nes_->controller(i)->Emulate(frame);
    }
}

void FM2Movie::Load(const std::string& filename) {
    FILE *fp;
    char buf[256];
    int n = 0;
    int predelay = FLAGS_fm2_predelay;

    fp = fopen(filename.c_str(), "r");
    if (fp == nullptr) {
        fprintf(stderr, "Could not open %s\n", filename.c_str());
        abort();
    }

    for(int j=0; j<predelay; j++) {
        for(int i=0; i<nes_->controller_size(); i++) {
            nes_->controller(i)->AppendButtons(0);
        }
    }

    printf("FM2 Info:\n");
    while(fgets(buf, sizeof(buf), fp) != nullptr) {
        std::string line(buf);
        while(line.back() == '\r' || line.back() == '\n')
            line.pop_back();

        if (line[0] != '|') {
            printf("    %s\n", line.c_str());
            continue;
        }
        if (predelay < 0) {
            predelay++;
            continue;
        }
        Parse(line);
        n++;
    }
    printf("Parse %d records.\n", n);
}

void FM2Movie::Parse(const std::string& s) {
    size_t pos = 0;

    if (s.at(pos++) != '|') {
        fprintf(stderr, "Expected '|' at position %lu\n", pos);
        fprintf(stderr, "%s\n%*s\n", s.c_str(), int(pos-1), "^");
        return;
    }

    while(isdigit(s.at(pos)))
        pos++;

    if (s.at(pos++) != '|') {
        fprintf(stderr, "Expected '|' at position %lu\n", pos);
        fprintf(stderr, "%s\n%*s\n", s.c_str(), int(pos-1), "^");
        return;
    }

    int ctrl = 0;
    while(pos != s.size()) {
        int b = 0;
        while(s.at(pos) != '|') {
            b <<= 1;
            b |= (s.at(pos) == '.' || s.at(pos) == ' ') ? 0 : 1;
            pos++;
        }
        nes_->controller(ctrl)->AppendButtons(b);
        pos++;
        ctrl++;
    }
}
