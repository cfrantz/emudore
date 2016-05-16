#ifndef EMUDORE_SRC_SDLUTIL_AUDIOSAMPLE_H
#define EMUDORE_SRC_SDLUTIL_AUDIOSAMPLE_H

#include <string>
#include <memory>
#include <map>

namespace sdlutil {

class AudioSample {
  public:
    AudioSample();
    void Load(const std::string& filename);
    float at(double n);

  private:
    std::unique_ptr<float> samples_;
    int length_;
};

class AudioSampleInstrument: public AudioSample {
  public:
    AudioSampleInstrument();
    inline void set_base_frequency(double f) { base_frequency_ = f; }
    void set_base_frequency(const std::string& notename);
    void PlayFrequency(double f);
    void PlayFrequency(const std::string& notename);
    float Next();

  private:
    static const std::map<std::string, double>& notes();
    double base_frequency_;
    double pos_, inc_;
};

}  // namespace sdlutil
#endif // EMUDORE_SRC_SDLUTIL_AUDIOSAMPLE_H
