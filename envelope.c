/*
    ADSR durchläuft Zeitachse (x-Achse) auf y-Achse Werte von 0-1
    y-Werte werden an granular_synth übergeben (an NoteOn Methode?) und dort auf Output Level multipliziert
*/

#include "envelope.h"
#include "grain.h"
#include "vas_mem.h"
#include "purple_utils.h"
#include "m_pd.h"
#include "c_granular_synth.h"


static t_class *envelope_class;

int getsamples_from_ms(int ms, float sr)
{
    if(sr)
    {
        return ceil((sr / 1000) * ms);
    }
    else{
        post("envelope.c - could not convert from ms to samples");
        return 0;
    }
}

float calculate_adsr_value(c_granular_synth *x)
{
    float adsr_val = 0;

    switch(x->adsr_env->adsr)
    {
        case ATTACK:
            x->current_adsr_stage_index++;
            adsr_val = x->current_adsr_stage_index++ * (1/x->adsr_env->attack_samples);
            if(x->current_adsr_stage_index >= x->adsr_env->attack_samples)
            {
                x->current_adsr_stage_index = 0;
                x->adsr_env->adsr = DECAY;
            }
            break;
        case DECAY:
            x->current_adsr_stage_index++;
            adsr_val = 1 + (((x->adsr_env->sustain-1)/x->adsr_env->decay_samples)*x->current_adsr_stage_index++);
            
            if(x->current_adsr_stage_index >= x->adsr_env->decay_samples)
            {
                x->current_adsr_stage_index = 0;
                x->adsr_env->adsr = RELEASE;
            }
            break;
        case SUSTAIN:
            adsr_val = x->adsr_env->sustain;
            break;
        case RELEASE:
            x->current_adsr_stage_index++;
            adsr_val = x->adsr_env->sustain - ((x->adsr_env->sustain/x->adsr_env->release_samples)*x->current_adsr_stage_index++);
            if(x->current_adsr_stage_index >= x->adsr_env->release_samples)
            {
                x->current_adsr_stage_index = 0;
                x->adsr_env->adsr = SILENT;
            }
            break;
        case SILENT:
            adsr_val = 0;
            break;
    }
    return adsr_val;
}

envelope *envelope_new(int attack, int decay, int sustain, int key_pressed, int release)
{
    envelope *x = (envelope *) vas_mem_alloc(sizeof(envelope));
    t_float SAMPLERATE = sys_getsr();
    x->adsr = ATTACK;

    x->attack = attack;
    x->decay = decay;
    x->sustain = sustain;
    x->key_pressed = key_pressed;
    x->release = release;
    x->duration = x->attack + x->decay + x->key_pressed+ x->release;

    x->envelope_samples_table = (t_sample *) vas_mem_alloc(x->duration * sizeof(t_sample));
    //fill envelope_samples_table
    
    x->attack_samples = get_samples_from_ms(attack, SAMPLERATE);
    x->decay_samples = get_samples_from_ms(decay, SAMPLERATE);
    x->key_pressed_samples = get_samples_from_ms(key_pressed, SAMPLERATE);
    x->release_samples = get_samples_from_ms(release, SAMPLERATE);
    int new_coordinate_decay = 0;
    int new_coordinate_release = 0;

    for(int i =0; i<x->duration;i++)
    {
        if(i<attack)
        {
            x->envelope_samples_table[i] = ((1*i)/x->attack_samples);
        }
        else if (i<attack+decay)
        {
            x->envelope_samples_table[i] = 1 + (((x->sustain-1)/x->decay_samples)*new_coordinate_decay);
            new_coordinate_decay++;
        }
        else if (i<attack+decay+key_pressed)
        {
            x->envelope_samples_table[i] = 1 * x->sustain;
        }
        else
        {
            // Release Stage
            x->envelope_samples_table[i] = sustain - ((sustain/x->release_samples)*new_coordinate_release);
            new_coordinate_release++;
        }
    }
    return x;
}



float gauss(grain x, int grainindex)
{
    t_int grain_size = x.grain_size_samples;
    if (grain_size == 0)
        return 0;
    float numerator = pow(grainindex-(grain_size/2), 2);
    float denominatior = 0.05*pow(grain_size, 2);
    float gauss_value = expf(-numerator/denominatior);
    //float gauss_value = expf(-(pow(grainindex-(grain_size/2), 2) / 0.2* pow(grain_size, 2)));
    return gauss_value;
}



void envelope_free(envelope *x)
{
    free(x);
}
