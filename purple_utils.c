/**
 * @file purple_utils.c
 * @author Kretschmar, Nikita 
 * @author Philipp, Adrian 
 * @author Strobl, Micha 
 * @author Wennemann,Tim <br>
 * Audiocommunication Group, Technische Universität Berlin <br>
 * @brief useful utilities for value conversion and manipulation <br>
 * @details useful utilities for value conversion and manipulation, outsourced into own .c file for better code readability <br>
 * @version 1.1
 * @date 2021-09-27
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "m_pd.h"
#include "purple_utils.h"
/**
 * @brief calculates number of samples
 * @details calculates number of samples from @a ms according to defined @a sr <br>
 * @param ms sample time in ms <br>
 * @param sr defined sample rate <br>
 * @return int number of samples <br>
 */
int get_samples_from_ms(int ms, float sr)
{
    if(sr)
    {
        return ceil((sr / 1000) * ms);
    }
    else{
        return 0;
    }
}
/**
 * @brief calculates sample time in ms
 * @details calculates sample time from @a num_samples according to defined @a sr <br>
 * @param num_samples number of samples <br>
 * @param sr defined samplerate <br>
 * @return float sample time
 */
float get_ms_from_samples(int num_samples, float sr)
{
    if(sr)
    {
        return (num_samples * 1000) / sr;
    }
    else{
        return 0;
    }
}
/**
 * @brief calculates interpolated sample value
 * @details calculates interpolated sample value between @a sample_left and @a sample_right <br>
 * @param sample_left value at the beginning of sample <br>
 * @param sample_right value at the end of sample <br>
 * @param frac position after decimal point <br>
 * @return float interpolated sample value <br>
 */
float get_interpolated_sample_value(float sample_left, float sample_right, float frac)
{
    float weighted_a = sample_left * (1 - frac);
    float weighted_b = sample_right * frac;
    return (weighted_a + weighted_b);
}
/**
 * @brief swaps to values
 * @details swaps to values @a a with @a b using a temporary third pointer <br>
 * @param a first value to swapped with second <br>
 * @param b second value to be swappend with first <br>
 */
void switch_float_values(float *a, float *b)
{
    float *temp_ptr = a;
    a = b;
    b = temp_ptr;
    return;
}
/**
 * @brief randomizes spray input value
 * @details randomizes spray input value for randomized start position of each grain <br>
 * @param spray_input spray input value
 * @return int randomized value
 */
int spray_dependant_playback_nudge(int spray_input)
{
    if(spray_input == 0) return 0;
    int off = rand() % (2 * spray_input);
    return off - spray_input;
}
