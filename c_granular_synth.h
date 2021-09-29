/**
 * @file c_granular_synth.h
 * @author Kretschmar, Nikita 
 * @author Philipp, Adrian 
 * @author Strobl, Micha 
 * @author Wennemann,Tim 
 * Audiocommunication Group, Technische Universität Berlin <br>
 * @brief header file of @a granular_synth.c file<br>
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
 * @brief The Pure Data struct of the c_granular_synth~ object. <br>
 *
 */

typedef struct c_granular_synth
{
    t_word      *soundfile;
    int         soundfile_length,
                current_grain_index,
                current_adsr_stage_index,
                current_gauss_stage_index,
                grain_size_ms,
                grain_size_samples,
                num_grains,
                midi_pitch,
                midi_velo,
                spray_input;
    float       gauss_q_factor;
    t_int       playback_position,    // which sample of the grain goes to the output next?
                playback_cycle_end,         // determines when to reset playback_pos to 
                current_start_pos,          // adjustable with dedicated pd slider
                sprayed_start_pos,          // start_pos affected by spray offset
                spray_true_offset;
    bool        reverse_playback;
    float       *soundfile_table;     //Array containing the original soundfile
    t_float     output_buffer,          // to sum up the current samples of all active grains
                time_stretch_factor,
                sr;
    grain       *grains_table;
    envelope    *adsr_env;
    //float* windowing_table;  // smoothing window function applied to grain output
} c_granular_synth;

void c_granular_synth_free(c_granular_synth *x);
c_granular_synth *c_granular_synth_new(t_word *soundfile, int soundfile_length, t_int grain_size_ms, t_int start_pos, float time_stretch_factor, t_int attack, t_int decay, float sustain, t_int release, float gauss_q_factor, t_int spray_input);
void c_granular_synth_generate_window_function(c_granular_synth *x);

void c_granular_synth_process_alt(c_granular_synth *x, float *in, float *out, int vector_size); // Test
void c_granular_synth_process(c_granular_synth *x, float *in, float *out, int vector_size);
void c_granular_synth_noteOn(c_granular_synth *x, float frequency, float velocity);
void c_granular_synth_set_num_grains(c_granular_synth *x);
void c_granular_synth_adjust_current_grain_index(c_granular_synth *x);
void c_granular_synth_populate_grain_table(c_granular_synth *x);
void grain_internal_scheduling(grain* g, c_granular_synth* synth);
void c_granular_synth_reset_playback_position(c_granular_synth *x);
void c_granular_synth_properties_update(c_granular_synth *x, t_int grain_size_ms, t_int start_pos, float time_stretch_factor, t_int midi_pitch, t_int midi_velo, t_int attack, t_int decay, float sustain, t_int release, float gauss_q_factor, t_int spray_input);
extern t_float SAMPLERATE;
float calculate_adsr_value(c_granular_synth *x);
float gauss (c_granular_synth *x);

#ifdef __cplusplus
}
#endif

#endif
