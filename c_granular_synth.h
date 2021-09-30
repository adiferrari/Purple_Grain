/**
 * @file c_granular_synth.h
 * @author Kretschmar, Nikita 
 * @author Philipp, Adrian 
 * @author Strobl, Micha 
 * @author Wennemann,Tim <br>
 * Audiocommunication Group, Technische Universität Berlin <br>
 * @brief header file of @a granular_synth.c file
 * @version 1.0
 * @date 2021-07-25
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
 * @brief pure data struct of the @a c_granular_synth object
 * @details pure data struct of the @a c_granular_synth object, defines all necessary variables for synth operation<br>
 */

typedef struct c_granular_synth
{
    t_word      *soundfile;                     ///< pointer towards the soundfile
    int         soundfile_length,               ///< lenght of the soundfile in samples          
                current_grain_index,            ///< index of the current grain
                current_adsr_stage_index,       ///< index of the current ADSR stage
                current_gauss_stage_index,      ///< index of the current gauss stage
                grain_size_ms,                  ///< size of a grain in milliseconds, adjustable through slider
                grain_size_samples,             ///< size of a grain in samples
                num_grains,                     ///< number of grains
                midi_pitch,                     ///< pitch/key value given by MIDI input
                midi_velo,                      ///< velocity value given by MIDI input
                spray_input;                    ///< randomizes the start position of each grain
    float       gauss_q_factor,                 ///< used to manipulate grain envelope slope
                pitch_factor;                   ///< multiplicator of MIDI input pitch/key value
    t_int       playback_position,              ///< which sample of the grain goes to the output next
                current_start_pos,              ///< position in the soundfle, determined by slider position
                sprayed_start_pos,              ///< start position is affected by @a spray_true_offset
                playback_cycle_end,             ///< determines when to reset @a playback_pos to @a current_start_pos
                spray_true_offset;              ///< actual starting position offset (initally set to 0) calculated on the run
    bool        reverse_playback;               ///< used fo switch playback to reverse, depends on @a time_stretch_factor value negativity
    float       *soundfile_table;               ///< array containing the original soundfile
    t_float     output_buffer,                  ///< used to sum up the current samples of all active grains
                time_stretch_factor,            ///< resizes sample length within a grain, adjustable through slider
                sr;                             ///< defined samplerate
    grain       *grains_table;                  ///< array containing the grains
    envelope    *adsr_env;                      ///< ADSR envelope
} c_granular_synth;

void c_granular_synth_free(c_granular_synth *x);
c_granular_synth *c_granular_synth_new(t_word *soundfile, int soundfile_length, int grain_size_ms, int start_pos, float time_stretch_factor, int attack, int decay, float sustain, int release, float gauss_q_factor, int spray_input, float pitch_factor, int midi_pitch);
void c_granular_synth_generate_window_function(c_granular_synth *x);
void c_granular_synth_process(c_granular_synth *x, float *in, float *out, int vector_size);
void c_granular_synth_set_num_grains(c_granular_synth *x);
void c_granular_synth_adjust_current_grain_index(c_granular_synth *x);
void c_granular_synth_populate_grain_table(c_granular_synth *x);
void grain_internal_scheduling(grain* g, c_granular_synth* synth);
void c_granular_synth_reset_playback_position(c_granular_synth *x);
void c_granular_synth_properties_update(c_granular_synth *x, t_int grain_size_ms, t_int start_pos, float time_stretch_factor, t_int midi_velo, t_int midi_pitch, t_int attack, t_int decay, float sustain, t_int release, float gauss_q_factor, t_int spray_input);
extern t_float SAMPLERATE;
float calculate_adsr_value(c_granular_synth *x);
float gauss (c_granular_synth *x);

#ifdef __cplusplus
}
#endif

#endif
