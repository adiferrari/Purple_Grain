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

#include "envelope.h"
#include "grain.h"
#include "vas_mem.h"
#include "purple_utils.h"
#include "m_pd.h"
#include "c_granular_synth.h"

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
            adsr_val = 1.0 + ((x->adsr_env->sustain-1.0)/x->adsr_env->decay_samples*x->current_adsr_stage_index++);
            if(x->current_adsr_stage_index >= x->adsr_env->decay_samples)
            {
                x->current_adsr_stage_index = 0;
                x->adsr_env->adsr = SUSTAIN;
            }
            break;
        case SUSTAIN:
            adsr_val = x->adsr_env->sustain;
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
    envelope *x = (envelope *) malloc(sizeof(envelope));
    t_float SAMPLERATE = sys_getsr();
    
    x->adsr = SILENT;
    x->attack = attack;
    x->decay = decay;
    x->sustain = sustain;
    x->release = release;
    
    x->attack_samples = get_samples_from_ms(attack, SAMPLERATE);
    x->decay_samples = get_samples_from_ms(decay, SAMPLERATE);
    x->release_samples = get_samples_from_ms(release, SAMPLERATE);
    return x;
}

/**
 * @brief calculates gauss value
 * calculates gauss value according to @param grainindex
 * @param x reference to the actual synthesizer
 * @return float gauss value
 */
float gauss(c_granular_synth *x)
{
    //t_int grain_size = x.grain_size_samples;
    if (x->grain_size_samples == 0)
        return 0;
    if (x->current_gauss_stage_index >= x->grain_size_samples)
    {
        x->current_gauss_stage_index = 0;
    }
    float numerator = pow(x->current_gauss_stage_index++ -(x->grain_size_samples/2), 2);
    float denominatior = x->gauss_q_factor * pow(x->grain_size_samples, 2);
    float gauss_value = expf(-numerator/denominatior);
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
