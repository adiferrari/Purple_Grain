/**
 * @file grain.h
 * @author Nikita Kretschmar, Adrian Philipp, Micha Strobl, Tim Wennemann <br>
 * Audiocommunication Group, Technische Universität Berlin <br>
 * @brief Grain file header <br>
 * <br>
 * Grain file header
 */

#ifndef grain_h
#define grain_h

#include "m_pd.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

//import SAMPLERATE from granular_synth.h

typedef struct grain
{
    struct grain        *next_grain;
    t_int               grain_size_samples,   // Grain size in samples
                        grain_index;
                        
    t_float             start,
                        end,
                        time_stretch_factor,
                        current_sample_pos,
                        next_sample_pos;
    bool                grain_active;
    
    // statt start nehme source_read_position
    // dann laufe über so viele Schritte wie grain_size_samples groß ist
    // Schrittweite modulieren, hochzählen und nach außen zurückgeben
    

    //grain *next_grain;          // next and previous pointers have to be passed back and forth
    //grain *previous_grain;      // between instance of granular_synth and every instantiated grain
} grain;


grain grain_new(int grain_size_samples, int soundfile_size, int grain_index, float time_stretch_factor);

// Include order forced this method to be included in c_granular_synth.h
//void grain_internal_scheduling(grain* g, c_granular_synth* synth);

void grain_free(grain *x);

#ifdef __cplusplus
}
#endif

#endif
