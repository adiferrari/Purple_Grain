/**
 * @file c_granular_synth.c
 * @author your name (you@domain.com)
 * @brief The C Part of the synthesizer's implementation
 * @version 0.1
 * @date 2021-07-25
 *
 * @copyright Copyright (c) 2021
 *
 */

#include "c_granular_synth.h"
#include "envelope.h"
#include "grain.h"
#include "purple_utils.h"

c_granular_synth *c_granular_synth_new(t_word *soundfile, int soundfile_length, int grain_size_ms, int start_pos, int attack, int decay, float sustain, int release)
{
    c_granular_synth *x = (c_granular_synth *)malloc(sizeof(c_granular_synth));
    x->soundfile_length = soundfile_length;
    x->sr = sys_getsr();
    x->grain_size_ms = grain_size_ms;
    x->grain_size_samples = get_samples_from_ms(x->grain_size_ms, x->sr);
    // diese vas_mem_alloc funktion hat die ganze zeit alles crashen lassen...
    //x->soundfile_table = (float *) vas_mem_alloc(x->soundfile_length * sizeof(float));
    x->soundfile_table = (float *) malloc(x->soundfile_length * sizeof(float));

    x->output_buffer = 0.0;
    x->current_start_pos = start_pos;                   // Set by user in pd with slider
    x->current_grain_index = 0;
    c_granular_synth_adjust_current_grain_index(x);     // Depends on start position slider
          
    x->playback_position = 0;
    x->current_adsr_stage_index = 0;
    //t_float SAMPLERATE = sys_getsr();
    x->grain_size_ms = grain_size_ms;
    x->adsr_env = envelope_new(attack, decay, sustain, 1000, release);
    x->time_stretch_factor = 1.0f;

    // Retrigger when user sets different grain size
    c_granular_synth_set_num_grains(x);
    post("C main file - new method - number of grains = %d", x->num_grains);
    
    for(int i = 0; i<soundfile_length;i++)
    {
        x->soundfile_table[i] = soundfile[i].w_float;
    }
    
    x->grains_table = NULL;
    c_granular_synth_populate_grain_table(x);

    return x;
}
/*
void c_granular_synth_process_alt(c_granular_synth *x, float *in, float *out, int vector_size)
{
    int i = vector_size;
    float weighted, integral;
    float output, gauss_val, adsr_val;
    //playback position speichern
    while(i--)
    {
        output = 0;
        if(x->playback_position >= x->soundfile_length) x->playback_position = 0;
        
        //hier wird fälschlicherweise die playback pos nicht hochgezählt
       
        if(x->grains_table[x->current_grain_index].grain_played_through)
        {
            x->grains_table[x->current_grain_index].grain_played_through = false;
            x->current_grain_index++;
            if(x->current_grain_index >= x->num_grains) x->current_grain_index = 0;
        }
        
        float left_sample = x->soundfile_table[(int)floor(x->grains_table[x->current_grain_index].current_sample_pos)];
        float right_sample = x->soundfile_table[(int)ceil(x->grains_table[x->current_grain_index].current_sample_pos)];
        float frac = modff(x->grains_table[x->current_grain_index].current_sample_pos, &integral);

        weighted = get_interpolated_sanple_value(left_sample, right_sample,frac);

        output += weighted;
        
        gauss_val = gauss(x->grains_table[x->current_grain_index],x->grains_table[x->current_grain_index].end - x->playback_position);
        output *= gauss_val;

        adsr_val = calculate_adsr_value(x);
        //output *= adsr_val;
        
        // Original Playback
        //output += x->soundfile_table[(int)floor(x->playback_position++)];
        
        // Adjust Grain's internal playback index, check if it has run through etc.
        grain_internal_scheduling(&x->grains_table[x->current_grain_index], x->soundfile_length);
        
        *out++ = output;
    }
    
}
*/
void c_granular_synth_process(c_granular_synth *x, float *in, float *out, int vector_size)
{
    int i = vector_size;
    float gauss_val, adsr_val;
    
    while(i--)
    {
        x->output_buffer = 0;
        //x->current_grain_index = 20;
        // oder kann man playback jetzt einfach immer +1 hochgehen?
        x->playback_position = x->current_start_pos;
        //ab hier dann schauen welches grain aktiv ist
        //x->playback_position = x->grains_table[x->current_grain_index].current_sample_pos;
        
        grain_internal_scheduling(&x->grains_table[x->current_grain_index], x);
        /*
        if(x->grains_table[x->current_grain_index].next_sample_pos > x->grains_table[x->current_grain_index].end)
        {
            x->grains_table[x->current_grain_index].next_sample_pos = x->grains_table[x->current_grain_index].start;
        }
        */
        //if(x->playback_position >= x->soundfile_length) x->playback_position = 0;
        
        gauss_val = gauss(x->grains_table[x->current_grain_index],x->grains_table[x->current_grain_index].end - x->playback_position);
        x->output_buffer *= gauss_val;
        
        adsr_val = calculate_adsr_value(x);
        weighted *= adsr_val;
        
        *out++ = x->output_buffer;
    }
    
}

void c_granular_synth_noteOn(c_granular_synth *x, float frequency, float velocity)
{
        //Create Voice, map Midi Key Number to frequency?
    // Apply Pitch to Signal

    // Use envelope, multiply values between 0-1 with sample volume value -> result = output volume value for voice
    // if (velocity == 0) -> go into release phase of envelope
    // -> velocity = 0 means NoteOff-Event

    if(velocity == 0) x->adsr_env->adsr = RELEASE;

    return;
}

void c_granular_synth_set_num_grains(c_granular_synth *x)
{
    x->num_grains = (int)ceilf(x->soundfile_length / x->grain_size_samples);
}

void c_granular_synth_adjust_current_grain_index(c_granular_synth *x)
{
    int index = x->current_start_pos / x->grain_size_samples;
    x->current_grain_index = index;
}

void c_granular_synth_populate_grain_table(c_granular_synth *x)
{
    grain *grains_table;
    grains_table = (grain *) calloc(x->num_grains, sizeof(grain));
    for(int j = 0; j<x->num_grains; j++)
    {
        grains_table[j] = grain_new(x->grain_size_samples, x->soundfile_length, j, x->time_stretch_factor);
        if(j > 0) grains_table[j-1].next_grain = &grains_table[j];
        if(grains_table[j].start <= x->playback_position &&  grains_table[j].end >= x->playback_position)
        {
            grains_table[j].grain_active = true;
        }
    }
    grains_table[x->num_grains - 1].next_grain = &grains_table[0];
    
    if(x->grains_table) free(x->grains_table);
    x->grains_table = grains_table;
}

void c_granular_synth_properties_update(c_granular_synth *x, int grain_size_ms, int start_pos, int midi_velo, int midi_pitch, int attack, int decay, float sustain, int release)
{
    if(!x->grains_table)
    {
        c_granular_synth_populate_grain_table(x);
    }
    if(x->current_start_pos != start_pos)
    {
        x->current_start_pos = start_pos;
        c_granular_synth_adjust_current_grain_index(x);
    }
    
    if(x->grain_size_ms != grain_size_ms)
    {
        x->grain_size_ms = grain_size_ms;
        int grain_size_samples = get_samples_from_ms(grain_size_ms, x->sr);
        x->grain_size_samples = grain_size_samples;
        c_granular_synth_set_num_grains(x);
        //GrainTable neu schreiben
        c_granular_synth_populate_grain_table(x);
    }
    if(x->midi_velo != midi_velo) x->midi_velo = midi_velo;
    if(x->midi_pitch != midi_pitch) x->midi_pitch = midi_pitch;
    if(x->attack != attack) x->attack = attack;
    if(x->decay != decay) x->decay = decay;
    if(x->sustain != sustain) x->sustain = sustain;
    if(x->release != release) x->release = release;
}
/*
void c_granular_synth_midi_update()
{
    
}
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
