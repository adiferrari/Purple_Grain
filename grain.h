/**
 * @file grain.h
 * @author Kretschmar, Nikita 
 * @author Philipp, Adrian 
 * @author Strobl, Micha 
 * @author Wennemann,Tim <br>
 * Audiocommunication Group, Technische Universität Berlin <br>
 * @brief header file to @a grain.c file
 * @version 1.0
 * @date 2021-09-27
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

/**
 * @struct grain
 * @brief pure data struct of the @a grain object
 * @details pure data struct of the @a grain object, defines all necessary variables for grain management<br>
 */
typedef struct grain
{
    struct grain        *next_grain,            ///< next grain according to the current one, passed back and forth between instances of @a granular_synth and every instantiated grain <br>
                        *previous_grain;        ///< previous grain according to the current one, passed back and forth between instances of @a granular_synth and every instantiated grain <br>
    t_int               grain_size_samples,     ///< size of the grain in samples <br>
                        grain_index,            ///< index of the current grain <br>
                        internal_step_count;    ///< count of steps <br>
    t_float             start,                  ///< starting point <br>
                        end,                    ///< ending point <br>
                        time_stretch_factor,    ///< resizes sample length within a grain, for negative values read samples in backwards direction, adjustable through slider <br>
                        current_sample_pos,     ///< position of the current sample <br>
                        next_sample_pos;        ///< position of the next sample according to the current one <br>
    bool                grain_active;           ///< current state of the grain, inactive or active <br>
        
} grain;

/**
 * @brief generates new grain
 * @details generates new grain with @a grain_index according to set @a grain_size_samples, @a start_pos, @a time_stretch_factor based on @a soundfile_size
 * @note include order forced this method to be included in c_granular_synth.h <br>
 */
grain grain_new(int grain_size_samples, int soundfile_size, float start_pos, int grain_index, float time_stretch_factor);


/**
 * @brief  frees grain
 * @details frees grain, necessary reset for further instances of grain genration <br>
 * @param  x input pointer of @a grain_free object 
 */
void grain_free(grain *x);

#ifdef __cplusplus
}
#endif

#endif
