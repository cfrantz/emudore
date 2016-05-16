#include "src/sdlutil/audiosample.h"

#include <cstdint>
#include <string.h>
#include <SDL2/SDL.h>

namespace sdlutil {

const char *kNoteNames[] = {
    "C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B",
};

AudioSample::AudioSample() : length_(0) {}

void AudioSample::Load(const std::string& filename) {
    SDL_AudioSpec spec;
    SDL_AudioCVT cvt;
    uint8_t *buf;
    uint32_t len;

    if (SDL_LoadWAV(filename.c_str(), &spec, &buf, &len) == nullptr) {
        fprintf(stderr, "Could not load %s: %s\n", filename.c_str(),
                SDL_GetError());
        abort();
    }

    printf("Loaded %d bytes @ %p\n", len, buf);
    SDL_BuildAudioCVT(&cvt, spec.format, spec.channels, spec.freq,
                            AUDIO_F32, 1, 44100);
    cvt.len = len;
    samples_.reset(new float[cvt.len * cvt.len_mult]());
    printf("samples_ = %d bytes @ %p\n", cvt.len * cvt.len_mult, samples_.get());
    cvt.buf = (uint8_t*)samples_.get();
    memcpy(cvt.buf, buf, len);
    //SDL_FreeWAV(buf);
    SDL_ConvertAudio(&cvt);
    length_ = cvt.len_cvt / sizeof(float);
}

float AudioSample::at(double n) {
    int a = int(n), b = int(n+1);
    double f = n - floor(n);
    float* samples = samples_.get();

    if (b < length_) {
        float sa = samples[a], sb = samples[b];
        return sa * (1.0-f) + sb * f;
    } else if (a < length_) {
        return samples[a];
    } else {
        return 0;
    }
}

AudioSampleInstrument::AudioSampleInstrument() :
    AudioSample(),
    base_frequency_(440), pos_(0), inc_(1) {}

    
void AudioSampleInstrument::PlayFrequency(double f) {
    pos_ = 0;
    inc_ = f / base_frequency_;
}

float AudioSampleInstrument::Next() {
    float v = at(pos_);
    pos_ += inc_;
    return v;
}

void AudioSampleInstrument::set_base_frequency(const std::string& notename) {
    const auto& fi = notes().find(notename);
    if (fi != notes().end()) {
        set_base_frequency(fi->second);
    } else {
        fprintf(stderr, "Can not find note name '%s'\n", notename.c_str());
        abort();
    }
}

void AudioSampleInstrument::PlayFrequency(const std::string& notename) {
    const auto& fi = notes().find(notename);
    if (fi != notes().end()) {
        PlayFrequency(fi->second);
    } else {
        fprintf(stderr, "Can not find note name '%s'\n", notename.c_str());
        abort();
    }
}

const std::map<std::string, double>& AudioSampleInstrument::notes() {
    static int once;
    static std::map<std::string, double> notemap;

    if (!once) {
        once = 1;
        char buf[8];
        double tr2 = pow(2, 1.0/12.0);
        for(int n=1; n<=88; n++) {
            double f = 440.0 * pow(tr2, n-49);
            int i = (n+8) % 12;
            int octave = (n+8) / 12;
            sprintf(buf, "%s%d", kNoteNames[i], octave);
            notemap.insert(std::make_pair(std::string(buf), f));
        }
    }
    return notemap;
}
}  // namespace sdlutil
