#include "stdint.h"

#include "sin.h"
#include "exp2.h"
#include "controllers.h"
#include "dx7note.h"
#include "EngineMkI.h"
#include "PluginData.h"
#include "freqlut.h"

void engine_init(void) {
  Exp2::init();
  Tanh::init();
  Sin::init();
  Freqlut::init(44100);
  PitchEnv::init(44100);
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

int main(int argc, char **argv) {
  engine_init();
  if (argc < 4) return 1;

  Controllers controllers;
  controller_prepare(&controllers);

  Cartridge cart;

  uint8_t bufer[4104];
  FILE *syx = fopen(argv[1], "rb");
  int size = fread(bufer, 1, 4104, syx);
  fprintf(stderr, "siz: %d\n", size);
  fprintf(stderr, "lesa: %d\n", cart.load(bufer, size));

  char dest[32][11];
  cart.getProgramNames(dest);
  for (int i = 0; i <32; i++) {
    // fprintf(stderr, "prog %d: %s\n", i, dest[i]);
  }

  Dx7Note note {};

  uint8_t data[161];
  int num = 0;
  sscanf(argv[2], "%d", &num);
  cart.unpackProgram(data, num);

  int velo = 100;
  int midinote = 60;
  sscanf(argv[3], "%d", &midinote);
  int pitch = standardNoteToFreq(midinote);
  fprintf(stderr, "pitchat: %d\n", pitch);
  // memcpy(data, init_voice, 155);

  note.init(data, midinote, pitch, velo);
  if (data[136] ) {
    note.oscSync();
  }
  // note.update(data, midinote, pitch, velo);

  const int numSamples = N*4410;
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
