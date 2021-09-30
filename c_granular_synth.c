/**
 * @file c_granular_synth.c
 * @author Kretschmar, Nikita 
 * @author Philipp, Adrian 
 * @author Strobl, Micha 
 * @author Wennemann,Tim <br>
 * Audiocommunication Group, Technische Universität Berlin <br>
 * @brief main file of the synthesizer's implementation
 * @version 1.1
 * @date 2021-07-25
 *
 * @copyright Copyright (c) 2021
 *
 */

#include "c_granular_synth.h"
#include "envelope.h"
#include "grain.h"
#include "purple_utils.h"

/**
 * @brief initial setup of soundfile and adjustment silder related variables
 * @details initial setup of soundfile and adjustment silder related variables <br>
 * @param soundfile contains the soundfile which can be read in via inlet <br> 
 * @param soundfile_length length of the soundfile in samples <br> 
 * @param grain_size_ms size of a grain in milliseconds, adjustable through slider <br> 
 * @param start_pos position within the soundfile, adjustable through slider <br> 
 * @param time_stretch_factor resizes sample length within a grain, for negative values read samples in backwards direction, adjustable through slider <br> 
 * @param attack attack time in the range of 0 - 4000ms, adjustable through slider <br> 
 * @param decay decay time in the range of 0 - 4000ms, adjustable through slider <br> 
 * @param sustain sustain time in the range of 0 - 1, adjustable through slider <br> 
 * @param release release time in the range of 0 - 10000ms, adjustable through slider <br> 
 * @param gauss_q_factor used to manipulate grain envelope slope in the range of 0.01 - 1, adjustable through slider<br>
 * @param spray_input randomizes the start position of each grain, actual starting position offset (initally set to 0) calculated on the run <br>
 * @param pitch_factor multiplicator of MIDI input pitch/key value, adjustable through slider<br>
 * @param midi_pitch MIDI input pitch/key value, usable through virtual or external MIDI device <br>
 * @return c_granular_synth* 
 */
c_granular_synth *c_granular_synth_new(t_word *soundfile, int soundfile_length, int grain_size_ms, t_int start_pos, float time_stretch_factor, int attack, int decay, float sustain, int release, float gauss_q_factor, int spray_input, float pitch_factor, int midi_pitch)
{
    c_granular_synth *x = (c_granular_synth *)malloc(sizeof(c_granular_synth));
    x->soundfile_length = soundfile_length;
    x->sr = sys_getsr();
    x->grain_size_ms = grain_size_ms;
    x->grain_size_samples = get_samples_from_ms(x->grain_size_ms, x->sr);
    x->soundfile_table = (float *) malloc(x->soundfile_length * sizeof(float));
    x->time_stretch_factor = time_stretch_factor;
    x->midi_pitch = midi_pitch;
    x->pitch_factor =  time_stretch_factor * (float)midi_pitch/48.0;
    x->reverse_playback = (x->pitch_factor < 0);
    x->output_buffer = 0.0;
    x->current_start_pos = start_pos;
    x->sprayed_start_pos = start_pos;
    x->current_grain_index = 0;
    x->current_gauss_stage_index = 0;
    x->spray_input = spray_input;
    x->spray_true_offset = 0;
    c_granular_synth_adjust_current_grain_index(x);
    
    c_granular_synth_reset_playback_position(x);
    
    x->current_adsr_stage_index = 0;
    x->adsr_env = envelope_new(attack, decay, sustain, release);

    /**
     * @brief Construct a new c granular synth set num grains object
     * @note retriggerd when user sets different grain size <b>
     */
    c_granular_synth_set_num_grains(x);
    c_granular_synth_adjust_current_grain_index(x);
    
    for(int i = 0; i<soundfile_length;i++)
    {
        x->soundfile_table[i] = soundfile[i].w_float;
    }
    
    x->grains_table = NULL;
    c_granular_synth_populate_grain_table(x);

    return x;
}

/**
 * @brief main synthesizer process
 * @details refreshs plaback positions, starts grain scheduleing, sets gauss value, generates ADSR value according to current state <br>
 * @param x input pointer of @a c_granular_synth_process object <br>
 * @param in input pointer of @a c_granular_synth_process object <br>
 * @param out output pointer of @a c_granular_synth_process object <br>
 * @param vector_size size of the input vector <br>
 * @note adsr must be in release state <br>
 */
void c_granular_synth_process(c_granular_synth *x, float *in, float *out, int vector_size)
{
    int i = vector_size;
    float gauss_val, adsr_val;
    
     while(i--)
    {
        x->output_buffer = 0;
        
        if(x->spray_input != 0 && x->spray_true_offset == 0 && x->midi_velo != 0)
        {
            x->spray_true_offset = spray_dependant_playback_nudge(x->spray_input);
            if(x->spray_true_offset != 0)
            {     
                c_granular_synth_reset_playback_position(x);
                c_granular_synth_adjust_current_grain_index(x);
                c_granular_synth_populate_grain_table(x);
            }
        }
        else
        {
            x->playback_position++;
            if(x->playback_position >= x->soundfile_length)
            {
                x->playback_position = 0;
            }
            else if(x->playback_position < 0)
            {
                x->playback_position = x->soundfile_length - 1 + x->playback_position;
            }
            else if(x->playback_position >= x->playback_cycle_end)
            {
                x->playback_position = x->current_start_pos;
            }
        }

        grain_internal_scheduling(&x->grains_table[x->current_grain_index], x);
        
        gauss_val = gauss(x);
        x->output_buffer *= gauss_val;
        
        if(x->midi_velo > 0)
        {
            adsr_val = calculate_adsr_value(x);
        }
        else
        {
            if(x->adsr_env->adsr == SILENT)
            {
                adsr_val = 0;
            }
            else
            {
                if(x->adsr_env->adsr != RELEASE)
                {
                    x->current_adsr_stage_index = 0;
                    x->adsr_env->adsr = RELEASE;
                }
                adsr_val = calculate_adsr_value(x);
            }
        }
        x->output_buffer *= adsr_val;
        *out++ = x->output_buffer;
    }
    
}

/**
 * @brief sets number of grains
 * @details sets number of grains according to @a soundfile_length and @a grain_size_samples <br>
 * @param x input pointer of @a c_granular_synth_set_num_grains object <br>
 */
void c_granular_synth_set_num_grains(c_granular_synth *x)
{
    x->num_grains = (int)ceilf(fabsf(x->soundfile_length * x->pitch_factor) / x->grain_size_samples);
}
/**
 * @brief adjusts current grain index
 * @details adjusts current grain index according to @a currents_start_pos and @a grain_size_samples <br>
 * @param x input pointer of @a c_granular_synth_adjust_current_grain_index object <b>
 */
void c_granular_synth_adjust_current_grain_index(c_granular_synth *x)
{
    if(x->num_grains > 0)
    {
        int index = ceil((x->sprayed_start_pos * fabs(x->pitch_factor)) / x->grain_size_samples);
        x->current_grain_index = (index == 0) ? 0 : index % x->num_grains;
    }
}
/**
 * @brief generates a grain table
 * @details generates a grain table according to @a current_grain_index, for negative @a time_stretch_factor values samples are read in backwards direction <br>
 * @param x input pointer of @a c_granular_synth_populate_grain_table object <br>
 */
void c_granular_synth_populate_grain_table(c_granular_synth *x)
{
    grain *grains_table;
    grains_table = (grain *) calloc(x->num_grains, sizeof(grain));
    int j;
    float start_offset = 0;
    
    if(x->reverse_playback)
    {
        for(j = x->current_grain_index; j >= 0; j--)
        {
            
            grains_table[j] = grain_new(x->grain_size_samples,
                                        x->soundfile_length,
                                        (x->sprayed_start_pos + x->grain_size_samples + start_offset),
                                        j, x->pitch_factor);
            if(j < x->current_grain_index) grains_table[j+1].next_grain = &grains_table[j];

            start_offset += x->pitch_factor * x->grain_size_samples;
        }
        grains_table[0].next_grain = &grains_table[x->num_grains - 1];
    }
    else
    {
        for(j = x->current_grain_index; j<x->num_grains; j++)
        {
            grains_table[j] = grain_new(x->grain_size_samples,
                                        x->soundfile_length,
                                        (x->sprayed_start_pos + start_offset),
                                        j, x->pitch_factor);
            if(j > 0) grains_table[j-1].next_grain = &grains_table[j];

            start_offset += x->pitch_factor * x->grain_size_samples;
        }
        grains_table[x->num_grains - 1].next_grain = &grains_table[0];
    }
    
    c_granular_synth_reset_playback_position(x);
    
    if(x->grains_table) free(x->grains_table);
    x->grains_table = grains_table;
}
/**
 * @brief checks on current input states
 * @details checks slider positions, MIDI input and ADSR state to update correspondent values <br>
 * @param[in] x input pointer of c_granular_synth_properties_update object <br>
 * @param[in] midi_velo MIDI input velocity value, usable through virtual or external MIDI device, also used for noteon detection <br>
 * @param[in] midi_pitch MIDI input pitch/key value, usable through virtual or external MIDI device<br>
 * @param[in] grain_size_ms size of a grain in milliseconds, adjustable through slider <br>
 * @param[in] start_pos position within the soundfile, adjustable through slider <br>
 * @param[in] time_stretch_factor resizes sample length within a grain, adjustable through slider <br>
 * @param[in] attack attack time in the range of 0 - 4000ms, adjustable through slider <br>
 * @param[in] decay decay time in the range of 0 - 4000ms, adjustable through slider <br>
 * @param[in] sustain sustain time in the range of 0 - 1, adjustable through slider <br>
 * @param[in] release release time in the range of 0 - 10000ms, adjustable through slider <br>
 * @param[in] gauss_q_factor envelope manipulation value in the range of 0.01 - 1, adjustable through slider <br>
 * @param[in] spray_input randomizes the start position of each grain, adjustable through slider <br>
 */
void c_granular_synth_properties_update(c_granular_synth *x, t_int grain_size_ms, t_int start_pos, float time_stretch_factor, t_int midi_velo, t_int midi_pitch, t_int attack, t_int decay, float sustain, t_int release, float gauss_q_factor, t_int spray_input)
{
    
    if(x->midi_velo != midi_velo)
    {
        x->midi_velo = (int)midi_velo;
    }
    
    if(x->midi_pitch != midi_pitch)
    {
        x->midi_pitch = (int)midi_pitch;
        if(x->midi_velo != 0) x->pitch_factor = time_stretch_factor * x->midi_pitch / 48.0;
    }
    
    if(x->grain_size_ms != grain_size_ms ||
       x->current_start_pos != start_pos ||
       x->time_stretch_factor != time_stretch_factor ||
       !x->grains_table)
    {
        if(x->grain_size_ms != grain_size_ms)
        {
            x->grain_size_ms = (int)grain_size_ms;
            int grain_size_samples = get_samples_from_ms((int)grain_size_ms, x->sr);
            x->grain_size_samples = grain_size_samples;
        }
        if(x->current_start_pos != start_pos)
        {
            x->current_start_pos = start_pos;
        }
        
        if(x->time_stretch_factor != time_stretch_factor)
        {
            x->time_stretch_factor = time_stretch_factor;
            x->pitch_factor = time_stretch_factor * x->midi_pitch / 48.0;
            
        }
        c_granular_synth_set_num_grains(x);
        c_granular_synth_adjust_current_grain_index(x);
        c_granular_synth_populate_grain_table(x);
    }
    
    if(x->spray_input != spray_input)
    {
        x->spray_input = (int)spray_input;
    }
    
    if (x->adsr_env->attack != attack || x->adsr_env->decay != decay || x->adsr_env->sustain != sustain || x->adsr_env->release != release)
    {
        if(x->adsr_env->attack != attack)
        {
            x->adsr_env->attack = (int)attack;
        }
        if(x->adsr_env->decay != decay)
        {
            x->adsr_env->decay = (int)decay;
        }
        if(x->adsr_env->sustain != sustain)
        {
            x->adsr_env->sustain = sustain;
        }
        if(x->adsr_env->release != release)
        {
            x->adsr_env->release = (int)release;
        }
        x->adsr_env = envelope_new(x->adsr_env->attack,
                                   x->adsr_env->decay,
                                   x->adsr_env->sustain,
                                   x->adsr_env->release);
    }

    if(x->gauss_q_factor != gauss_q_factor)
    {
        x->gauss_q_factor = gauss_q_factor;
    }
}
/**
 * @related pd_granular_synth_tilde
 * @brief resets playback position
 * @details resets playback position <br>
 * @param x input pointer of @a c_granular_synth_reset_playback_position object <br>
 */
void c_granular_synth_reset_playback_position(c_granular_synth *x)
{
    x->sprayed_start_pos = x->current_start_pos + x->spray_true_offset;
    while(x->sprayed_start_pos < 0)
    {
        x->sprayed_start_pos += (x->soundfile_length - 1);
    }
    while(x->sprayed_start_pos >= x->soundfile_length)
    {
        x->sprayed_start_pos -= x->soundfile_length;
    }
    x->playback_position = x->sprayed_start_pos;
    

    x->playback_cycle_end = x->playback_position + x->grain_size_samples;
    while(x->playback_cycle_end >= x->soundfile_length)
    {
        x->playback_cycle_end -= x->soundfile_length;
    }
}

/**
 * @related pd_granular_synth_tilde
 * @brief frees @a granular_synth object
 * @details  frees @a granular_synth object <br>
 * @param x input pointer of @a c_granular_synth_free object <br>
 */
void c_granular_synth_free(c_granular_synth *x)
{
    if(x)
    {
        free(x->soundfile_table);
        free(x->grains_table);
        envelope_free(x->adsr_env);
        free(x);
    }
}
