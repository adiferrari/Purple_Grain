/**
 * @file purple_utils.c
 * @author Nikita Kretschmar
 * @author Adrian Philipp
 * @author Micha Strobl
 * @author Tim Wennemann
 * @brief useful utilities for value conversion and manipulation
 * useful utilities for value conversion and manipulation
 * outsourced into own .c file for better code readability
 * @version 0.1
 * @date 2021-09-27
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <stdio.h>
#include <math.h>
#include "m_pd.h"
#include "purple_utils.h"
/**
 * @brief calculates number of samples from @a ms and @a sr
 * 
 * @param ms sample time in ms
 * @param sr defined sample rate
 * @return int number of samples
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
 * @brief calculates sample time in ms from @a num_samples and @a sr
 * 
 * @param num_samples number of samples
 * @param sr defined samplerate
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
 * calculates interpolated sample value between @a sample_left and @a sample_right
 * @param sample_left value at the beginning of sample
 * @param sample_right value at the end of sample
 * @param frac position after decimal point
 * @return float interpolated sample value
 */
float get_interpolated_sample_value(float sample_left, float sample_right, float frac)
{
    float weighted_a = sample_left * (1 - frac);
    float weighted_b = sample_right * frac;
    return (weighted_a + weighted_b);
}
/**
 * @brief swaps to values
 * swaps to values of float type
 * @param a first value to swapped with second
 * @param b second value to be swappend with first
 */
void switch_float_values(float *a, float *b)
{
    float *temp_ptr = a;
    a = b;
    b = temp_ptr;
    return;
}
