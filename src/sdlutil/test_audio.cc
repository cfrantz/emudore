#include <gflags/gflags.h>
#include <SDL2/SDL.h>

#include "src/sdlutil/audiosample.h"
#include "src/io.h"

DEFINE_string(wav, "", "WAV file to load.");
DEFINE_string(freq, "A4", "Fundamental frequency of WAV file.");
DEFINE_string(note, "A4", "Note to play.");

using namespace sdlutil;
int main(int argc, char *argv[]) {
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    AudioSampleInstrument asi;

    asi.Load(FLAGS_wav);
    asi.set_base_frequency(FLAGS_freq);
    asi.PlayFrequency(FLAGS_note);

    IO io(256, 240, 60);
    io.init_audio(44100, 1, 2048, AUDIO_F32, [&asi](uint8_t* stream, int len) {
        len /= sizeof(float);
        float* sample = (float*)stream;
        for(int i=0; i<len; i++) {
            sample[i] = asi.Next();
        }
    });

    while(io.emulate()) ;
    return 0;
}
