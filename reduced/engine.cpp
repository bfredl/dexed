#include "stdint.h"

#include "sin.h"
#include "exp2.h"
#include "controllers.h"
#include "dx7note.h"
#include "EngineMkI.h"

void engine_init(void) {
  Exp2::init();
  Tanh::init();
  Sin::init();

}


EngineMkI engineMkI;
void controller_prepare(Controllers * controllers) {
    controllers->values_[kControllerPitchRangeUp] = 3;
    controllers->values_[kControllerPitchRangeDn] = 3;
    controllers->values_[kControllerPitchStep] = 0;
    controllers->masterTune = 0;

    controllers->values_[kControllerPitch] = 0x2000;
    controllers->modwheel_cc = 0;
    controllers->foot_cc = 0;
    controllers->breath_cc = 0;
    controllers->aftertouch_cc = 0;
    controllers->refresh(); 

    controllers->core = &engineMkI;
}

int standardNoteToFreq(int note) {
    const int base = 50857777;  // (1 << 24) * (log(440) / log(2) - 69/12) 
    const int step = (1 << 24) / 12;
    return base + step * note;
}

int main() {
  engine_init();

  Controllers controllers;
  controller_prepare(&controllers);


  Dx7Note note {};

  uint8_t data[161];

  int velo = 127;
  int midinote = 64;
  int pitch = standardNoteToFreq(midinote);

  note.init(data, midinote, pitch, velo);
  if (data[136] ) {
    note.oscSync();
  }

  const int numSamples = N*1000;
  int16_t audiobuf[numSamples];
  for (int i = 0; i < numSamples; i += N) {
    int32_t computebuf[N];
    note.compute(computebuf, 0, 0, &controllers);
    for (int j = 0; j < N; j++) {
        int val = computebuf[j] >> 4;
        int clip_val = val < -(1 << 24) ? -0x8000 : val >= (1 << 24) ? 0x7fff : val >> 9;
        audiobuf[i+j] = clip_val;
    }
  }

  fwrite(audiobuf, 1, numSamples*2, stdout);
}
