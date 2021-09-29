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

/*
    ADSR Angaben bestimmt in s oder ms?
    Konvertierung in Samples notwendig?
    Check Funktion dass Enveloe Länge nicht länger alsLänge des Soundfiles ist?
 */

enum adsr_stage {
    ATTACK,
    DECAY,
    SUSTAIN,
    RELEASE,
    SILENT
};

typedef struct envelope
{
    t_object x_obj;
    t_int attack;
    t_int decay;
    t_float peak,
            sustain;
    t_int release;
    t_int duration;
    t_int attack_samples,
            decay_samples,
            release_samples;
    t_sample *envelope_samples_table;
    enum adsr_stage adsr;
} envelope;

int getsamples_from_ms(int ms, float sr);
typedef struct window
{
    t_object x_obj;
    t_int q_factor;
    t_sample *window_samples_table;
}window;

envelope *envelope_new(int attack, int decay, float sustain, int release);

//float gauss(float q_factor, int grain_size, int sample);

void envelope_free(envelope *x);

#ifdef __cplusplus
}
#endif

#endif
