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


envelope *envelope_new(int attack, int decay, float sustain, int release)

{
    envelope *x = (envelope *) vas_mem_alloc(sizeof(envelope));
    t_float SAMPLERATE = sys_getsr();
    
    //ACHTUNG diese muss bei Note on wieder raus -> start mit silent
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
/*
float gauss(float q_factor, int grain_size, int grainindex)
{
    //t_int grain_size = x.grain_size_samples;
    if (grain_size == 0)
        return 0;
    float numerator = pow(grainindex-(grain_size/2), 2);
    float denominatior = q_factor * pow(grain_size, 2);
    float gauss_value = expf(-numerator/denominatior);
    return gauss_value;
}
*/


void envelope_free(envelope *x)
{
    free(x);
}
