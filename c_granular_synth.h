/**
 * @file c_granular_synth.h
 * @author Nikita Kretschmar, Adrian Philipp, Micha Strobl, Tim Wennemann <br>
 * Audiocommunication Group, Technische Universität Berlin <br>
 * @brief Main file header <br>
 * <br>
 * Main file header
 */

#ifndef c_granular_synth_h
#define c_granular_synth_h

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "math.h"
#include "grain.h"
#include "envelope.h"
#include "m_pd.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NUMELEMENTS(x)  (sizeof(x) / sizeof((x)[0]))

/**
 * @struct c_granular_synth
 * @brief The Purde Data struct of the c_granular_synth~ object. <br>
 *
 */

typedef struct c_granular_synth
{
    t_word      *soundfile;
    int         soundfile_length,
                current_start_pos,          // adjustable with dedicated pd slider
                current_grain_index,
                current_adsr_stage_index,
                grain_size_ms,
                grain_size_samples,
                num_grains,
                midi_pitch,
                midi_velo;
    t_int       playback_position;    // which sample of the grain goes to the output next?
    float       *soundfile_table;     //Array containing the original soundfile
    t_float     output_buffer,          // to sum up the current samples of all active grains
                time_stretch_factor,
                sr;
    grain       *grains_table;
    envelope    *adsr_env;
    //float* windowing_table;  // smoothing window function applied to grain output
} c_granular_synth;

void c_granular_synth_free(c_granular_synth *x);
c_granular_synth *c_granular_synth_new(t_word *soundfile, int soundfile_length, int grain_size_ms, int start_pos, float time_stretch_factor, int attack, int decay, float sustain, int release);
void c_granular_synth_generate_window_function(c_granular_synth *x);

void c_granular_synth_process_alt(c_granular_synth *x, float *in, float *out, int vector_size); // Test
void c_granular_synth_process(c_granular_synth *x, float *in, float *out, int vector_size);
void c_granular_synth_noteOn(c_granular_synth *x, float frequency, float velocity);
void c_granular_synth_set_num_grains(c_granular_synth *x);
void c_granular_synth_adjust_current_grain_index(c_granular_synth *x);
void c_granular_synth_populate_grain_table(c_granular_synth *x);
void grain_internal_scheduling(grain* g, c_granular_synth* synth);
void c_granular_synth_properties_update(c_granular_synth *x, int grain_size_ms, int start_pos, float time_stretch_factor, int midi_pitch, int midi_velo, int attack, int decay, float sustain, int release);
extern t_float SAMPLERATE;

float calculate_adsr_value(c_granular_synth *x);

#ifdef __cplusplus
}
#endif

#endif
