/**
 * @file envelope.c
 * @author Nikita Kretschmar
 * @author Adrian Philipp
 * @author Micha Strobl
 * @author Tim Wennemann
 * @brief handles envelope generation
 * generates ADSR envelope according to adjustable attack, decay, sustain and release parameters
 * @version 0.1
 * @date 2021-09-27
 * 
 * @copyright Copyright (c) 2021
 * 
 */

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


//static t_class *envelope_class;
/**
 * @brief calculates ADSR value
 * calculates single atm ADSR value according to current state
 * @param x input pointer of calculate_adsr_value object
 * @return float ADSR value
 */
float calculate_adsr_value(c_granular_synth *x)
{
    float adsr_val = 0;
    float attack_val = 0;
    switch(x->adsr_env->adsr)
    {
        case ATTACK:
            attack_val = (1.0/x->adsr_env->attack_samples);
            adsr_val = x->current_adsr_stage_index++ * attack_val;
            if(x->current_adsr_stage_index >= x->adsr_env->attack_samples)
            {
                x->current_adsr_stage_index = 0;
                x->adsr_env->adsr = DECAY;
            }
            break;
        case DECAY:
            //decay_val = (x->adsr_env->sustain-1.0)/x->adsr_env->decay_samples;
            adsr_val = 1.0 + ((x->adsr_env->sustain-1.0)/x->adsr_env->decay_samples*x->current_adsr_stage_index++);
            //adsr_val = 1.0 + ((x->adsr_env->sustain-1.0)*(x->current_adsr_stage_index++/x->adsr_env->decay_samples));
            
            if(x->current_adsr_stage_index >= x->adsr_env->decay_samples)
            {
                x->current_adsr_stage_index = 0;
                x->adsr_env->adsr = SUSTAIN;
            }
            break;
        case SUSTAIN:
            adsr_val = x->adsr_env->sustain;
            //x->current_adsr_stage_index++;
            //if(x->current_adsr_stage_index >= x->adsr_env->key_pressed_samples)
            /*
            if(x->midi_velo == 0)
            {
                x->current_adsr_stage_index = 0;
                x->adsr_env->adsr = RELEASE;
            }
    */
            break;
        case RELEASE:
            if(x->midi_velo > 0)
            {
                x->adsr_env->adsr = ATTACK;
                x->current_adsr_stage_index = 0;
                break;
            }
            adsr_val = x->adsr_env->sustain - ((x->adsr_env->sustain/x->adsr_env->release_samples)*x->current_adsr_stage_index++);
            if(x->current_adsr_stage_index >= x->adsr_env->release_samples)
            {
                x->current_adsr_stage_index = 0;
                x->adsr_env->adsr = SILENT;
            }
            break;
        case SILENT:
            if(x->midi_velo>0)
            {
                x->adsr_env->adsr = ATTACK;
                x->current_adsr_stage_index = 0;
                break;
            }
            adsr_val = 0;
            break;
    }
    return adsr_val;
}

/**
 * @brief generates new ADSR envelope
 * 
 * @param attack attack time in the range of 0 - 4000ms, adjustable through slider
 * @param decay decay time in the range of 0 - 4000ms, adjustable through slider
 * @param sustain sustain time in the range of 0 - 1, adjustable through slider
 * @param release release time in the range of 0 - 10000ms, adjustable through slider
 * @return envelope* 
 */
envelope *envelope_new(int attack, int decay, float sustain, int release)

{
    envelope *x = (envelope *) vas_mem_alloc(sizeof(envelope));
    t_float SAMPLERATE = sys_getsr();
    
    //ACHTUNG diese muss bei Note on wieder raus -> start mit silent
    x->adsr = SILENT;

    x->attack = attack;
    x->decay = decay;
    x->sustain = sustain;
    //x->key_pressed = key_pressed;
    x->release = release;
    //x->duration = x->attack + x->decay + x->key_pressed+ x->release;

    //x->envelope_samples_table = (t_sample *) vas_mem_alloc(x->duration * sizeof(t_sample));
    
    x->attack_samples = get_samples_from_ms(attack, SAMPLERATE);
    x->decay_samples = get_samples_from_ms(decay, SAMPLERATE);
    x->release_samples = get_samples_from_ms(release, SAMPLERATE);
    return x;
}

    //int new_coordinate_decay = 0;
    //int new_coordinate_release = 0;
/*
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
 */


/*
    Create windowing for all Grains by using envelope.h
    using only A,S,R parameters (3 stages: Fade-In, Full Volume, Fade-Out)
    Consider Grain Duration (as Input parameter) and maybe take 1/10 of the duration at start for Fade-In
    1/10 at the end fo Fade-Out and the other 8/10s for full output stage
*/

/**
 * @brief calculates gauss value
 * calculates gauss value according to @param grainindex
 * @param x input pointer of gauss object
 * @param grainindex index of grain
 * @return float gauss value
 */
float gauss(grain x, int grainindex)
{
    t_int grain_size = x.grain_size_samples;
    if (grain_size == 0)
        return 0;
    float numerator = pow(grainindex-(grain_size/2), 2);
    float denominatior = 0.2*pow(grain_size, 2);
    float gauss_value = expf(-numerator/denominatior);
    //float gauss_value = expf(-(pow(grainindex-(grain_size/2), 2) / 0.2* pow(grain_size, 2)));
    return gauss_value;
}


/**
 * @brief frees envelope
 * 
 * @param x input pointer of envelope_free object
 */
void envelope_free(envelope *x)
{
    free(x);
}
