/**
 * @file envelope.h
 * @author Kretschmar, Nikita 
 * @author Philipp, Adrian 
 * @author Strobl, Micha 
 * @author Wennemann,Tim
 * Audiocommunication Group, Technische Universität Berlin <br>
 * @brief header file of @a envelope.c file <br>
 */

#ifndef envelope_h
#define envelope_h

#include "m_pd.h"
#include "grain.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


#ifdef __cplusplus
extern "C" {
#endif

enum adsr_stage {
    ATTACK,
    DECAY,
    SUSTAIN,
    RELEASE,
    SILENT
};

/**
 * @struct envelope
 * @brief pure data struct of the @a envelope object
 * @details pure data struct of the @a envelope object, defines all necessary variables for enevelope generation <b>
 */

typedef struct envelope
{
    t_object x_obj;                     ///< object used for method input/output handling <b>
    t_int attack;                       ///< attack time in the range of 0 - 4000ms, adjustable through slider <br>
    t_int decay;                        ///< decay time in the range of 0 - 4000ms, adjustable through slider <br>
    t_float sustain;                    ///< sustain time in the range of 0 - 1, adjustable through slider <br>
    t_int release;                      ///< release time in the range of 0 - 10000ms, adjustable through slider <br>
    t_int duration;                     ///< duration time from @a attack to @a release  <b>
    t_int attack_samples,               ///< attack time in samples <b>
          decay_samples,                ///< decay time in samples <b>
          release_samples;              ///< release time in samples <b>
    t_sample *envelope_samples_table;   ///< array containing the envelope samples <b>
    enum adsr_stage adsr;               ///< current ADSR stage <b>
} envelope;

int getsamples_from_ms(int ms, float sr);
/**
 * @struct window
 * @brief pure data struct of the @a window object
 * @details pure data struct of the @a window object, defines all necessary variables for windowing <b>
 */
typedef struct window
{
    t_object x_obj;                     ///< object used for method input/output handling <b>
    t_int q_factor;                     ///< q factor of the gauss distribution <b>
    t_sample *window_samples_table;     ///< array containing the window samples <b>
}window;

envelope *envelope_new(int attack, int decay, float sustain, int release);

/**
 * @brief frees envelope
 * @details frees envelope
 * @param x input pointer of @a envelope_free object
 */
void envelope_free(envelope *x);

#ifdef __cplusplus
}
#endif

#endif
