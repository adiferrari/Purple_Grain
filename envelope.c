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
            x->current_adsr_stage_index++;
            if(x->current_adsr_stage_index >= x->adsr_env->key_pressed_samples)
            {
                x->current_adsr_stage_index = 0;
                x->adsr_env->adsr = RELEASE;
            }
    
            break;
        case RELEASE:
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


envelope *envelope_new(int attack, int decay, float sustain, int key_pressed, int release)

{
    envelope *x = (envelope *) vas_mem_alloc(sizeof(envelope));
    t_float SAMPLERATE = sys_getsr();
    
    //ACHTUNG diese muss bei Note on wieder raus -> start mit silent
    x->adsr = ATTACK;

    x->attack = attack;
    x->decay = decay;
    x->sustain = sustain;
    x->key_pressed = key_pressed;
    x->release = release;
    x->duration = x->attack + x->decay + x->key_pressed+ x->release;

    x->envelope_samples_table = (t_sample *) vas_mem_alloc(x->duration * sizeof(t_sample));
    
    x->attack_samples = get_samples_from_ms(attack, SAMPLERATE);
    x->decay_samples = get_samples_from_ms(decay, SAMPLERATE);
    x->key_pressed_samples = get_samples_from_ms(key_pressed, SAMPLERATE);
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



void envelope_free(envelope *x)
{
    free(x);
}
