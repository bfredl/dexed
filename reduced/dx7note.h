/*
 * Copyright 2016-2017 Pascal Gauthier.
 * Copyright 2012 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SYNTH_DX7NOTE_H_
#define SYNTH_DX7NOTE_H_

// This is the logic to put together a note from the MIDI description
// and run the low-level modules.

// It will continue to evolve a bit, as note-stealing logic, scaling,
// and real-time control of parameters live here.

#include "env.h"
#include "pitchenv.h"
#include "fm_core.h"
#include "controllers.h"
#include <memory>

struct VoiceStatus {
    uint32_t amp[6];
    char ampStep[6];
    char pitchStep;
};

class Dx7Patch {
  public:
    const uint8_t *currentPatch;
    EnvParams env_p[6];
    EnvParams pitchenv_p;
  
    int pitchmoddepth;
    int pitchmodsens;
    int ampmoddepth;

    int32_t ampmodsens[6];
    uint8_t opMode[6];
    uint8_t algorithm;
    int32_t fb_shift;

    void update(const uint8_t patch[156]);
};

class Dx7Note {
public:
    Dx7Note();
    void init(Dx7Patch &p, int midinote, int logfreq, int velocity);
    // Note: this _adds_ to the buffer. Interesting question whether it's
    // worth it...
    void compute(int32_t *buf, int32_t lfo_val, int32_t lfo_delay,
                 int pb, const Controllers *ctrls);
    
    void keyup();
    
    // TODO: some way of indicating end-of-note. Maybe should be a return
    // value from the compute method? (Having a count return from keyup
    // is also tempting, but if there's a dynamic parameter change after
    // keyup, that won't work.
    
    // PG:add the update
    void update(Dx7Patch &p, int midinote, int logFreq, int velocity);
    void updateBasePitches(int logFreq);
    void transferState(Dx7Note& src);
    void transferSignal(Dx7Note &src);
    void oscSync();

    int32_t osc_freq(int log_freq, int mode, int coarse, int fine, int detune);
    
private:
    Env env_[6];
    PitchEnv pitchenv_;
    int32_t phase[6];
    int32_t gain_out[6];
    int32_t basepitch_[6];
    int32_t fb_buf_[2];
    
    const Dx7Patch *p;
    
};

#endif  // SYNTH_DX7NOTE_H_
