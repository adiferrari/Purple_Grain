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
#include "grain.h"
#include "envelope.h"
#include "purple_utils.h"

c_granular_synth *c_granular_synth_new(t_word *soundfile, int soundfile_length, int grain_size_ms)
{
    c_granular_synth *x = (c_granular_synth *)malloc(sizeof(c_granular_synth));
    x->soundfile_length = soundfile_length;

    // diese vas_mem_alloc funktion hat die ganze zeit alles crashen lassen...
    //x->soundfile_table = (float *) vas_mem_alloc(x->soundfile_length * sizeof(float));
    x->soundfile_table = (float *) malloc(x->soundfile_length * sizeof(float));

    x->current_grain_index = 0;
    x->playback_position = 0;
    x->current_adsr_stage_index = 0;
    t_float SAMPLERATE = sys_getsr();
    x->grain_size_ms = grain_size_ms;
    x->adsr_env = envelope_new(20, 200, 1, 1, 200);
    
    x->grain_size_samples = get_samples_from_ms(x->grain_size_ms, SAMPLERATE);

    c_granular_synth_set_num_grains(x);
    post("C main file - new method - number of grains = %d", x->num_grains);
    
    for(int i = 0; i<soundfile_length;i++)
    {
        x->soundfile_table[i] = soundfile[i].w_float;
    }


    // Das hier noch als Funktion auslagern
    x->grains_table = (grain *) malloc(x->num_grains * sizeof(grain));
    for(int j = 0; j<x->num_grains; j++)
    {
        x->grains_table[j] = *grain_new(x->grain_size_samples, x->soundfile_length, j);
        // entweder hier mit sternchen die new method return komponente dereferenzieren...
        //oder diese umschreiben dass sie keinen grain pointer sondern einen grain zurückliefert
    }

    //x->windowing_table = (float *) vas_mem_alloc(x->grain_size_samples * sizeof(float));

    return x;
}

void c_granular_synth_process_alt(c_granular_synth *x, float *in, float *out, int vector_size)
{
    int i = vector_size;
    float output, gauss_val;
    //playback position speichern
    while(i--)
    {
        output = 0;
        if(x->playback_position >= x->soundfile_length) x->playback_position = 0;
        
        //checken an welcher position man innerhalb des Grains gerade sein müsste
        //checken ob dies die letzte position des Grain ist -- wenn ja current_grain_index++
        if(x->playback_position >= x->grains_table[x->current_grain_index].end)
        {

            x->current_grain_index++;
            if(x->current_grain_index >= x->num_grains) x->current_grain_index = 0;
             }


        output += x->soundfile_table[(int)floor(x->playback_position++)];
        
        *out++ = output;
    }
    
}

void c_granular_synth_process(c_granular_synth *x, float *in, float *out, int vector_size)
{
    int i = vector_size;
    
    while(i--)
    {
        *out++ = x->soundfile_table[(int)floor(x->current_grain_index)];
        x->current_grain_index++;
        if(x->current_grain_index >= x->soundfile_length)
        {
            x->current_grain_index -= x->soundfile_length;
        }
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

float calculate_adsr_value(c_granular_synth *x)
{
    float ret = 0;

    switch(x->adsr_env->adsr)
    {
        case ATTACK:
            ret = 1;
            x->current_adsr_stage_index++;
            //ret = x->current_adsr_stage_index++ * (1/x->adsr_env->attack_samples);
            if(x->current_adsr_stage_index >= x->adsr_env->attack_samples)
            {
                x->current_adsr_stage_index = 0;
                x->adsr_env->adsr = DECAY;
                post("switch to decay phase");
            }
            break;
        case DECAY:
            ret = 1;
            x->current_adsr_stage_index++;
            //ret = 1 + (((x->adsr_env->sustain-1)/x->adsr_env->decay_samples)*x->current_adsr_stage_index++);
            
            if(x->current_adsr_stage_index >= x->adsr_env->decay_samples)
            {
                x->current_adsr_stage_index = 0;
                x->adsr_env->adsr = RELEASE;
                post("switch to sustain phase");
            }
            break;
        case SUSTAIN:
            ret = x->adsr_env->sustain;
            break;
        case RELEASE:
            ret = 1;
            x->current_adsr_stage_index++;
            //ret = x->adsr_env->sustain - ((x->adsr_env->sustain/x->adsr_env->release_samples)*x->current_adsr_stage_index++);
            if(x->current_adsr_stage_index >= x->adsr_env->release_samples)
            {
                x->current_adsr_stage_index = 0;
                x->adsr_env->adsr = SILENT;
                post("switch to silent phase");
            }
            break;
        case SILENT:
            post("currently in silent phase");
            ret = 0;
            break;
    }
        

    return (float)ret;
}

void c_granular_synth_set_num_grains(c_granular_synth *x)
{
    x->num_grains = (int)ceilf(x->soundfile_length / x->grain_size_samples);
}

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
