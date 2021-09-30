/**
 * @file purple_utils.h
 * @author Kretschmar, Nikita 
 * @author Philipp, Adrian 
 * @author Strobl, Micha 
 * @author Wennemann,Tim <br>
 * Audiocommunication Group, Technische Universität Berlin <br>
 * @brief header file to @a purple_utils.c file
 * @version 1.1
 * @date 2021-09-27
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef purple_utils_h
#define purple_utils_h

int get_samples_from_ms(int ms, float sr);
float get_ms_from_samples(int num_samples, float sr);
float get_interpolated_sample_value(float sample_left, float sample_right, float frac);
void switch_float_values(float *a, float *b);
int spray_dependant_playback_nudge(int spray_input);

#endif
