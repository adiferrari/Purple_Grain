/**
 * @file grain.c
 * @author Kretschmar, Nikita 
 * @author Philipp, Adrian 
 * @author Strobl, Micha 
 * @author Wennemann,Tim <br>
 * Audiocommunication Group, Technische Universität Berlin <br>
 * @brief handles grain creation
 * @details handles grain creation and basic scheduling according to input parameters set by the synthesizer<br>
 * @version 1.1
 * @date 2021-09-27
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "grain.h"
#include "c_granular_synth.h"
#include "envelope.h"
#include "purple_utils.h"

/**
 * @brief generates new grain
 * @details generates new grain with @a grain_index according to set @a grain_size_samples, @a start_pos, @a time_stretch_factor based on @a soundfile_size <br>
 * @param grain_size_samples size of a grain as amount of contained samples <br>
 * @param soundfile_size size of the soundfile in samples <br>
 * @param start_pos starting position within the soundfile, adjustable through slider <br>
 * @param grain_index corresponding index of a grain <br>
 * @param time_stretch_factor resizes sample length within a grain, adjustable through slider <br>
 * @return grain 
 */
grain grain_new(int grain_size_samples, int soundfile_size, float start_pos, int grain_index, float time_stretch_factor)
{
    grain x;
    x.grain_active = false;
    x.grain_size_samples = grain_size_samples;
    x.grain_index = grain_index;
    x.internal_step_count = 0;
    x.time_stretch_factor = time_stretch_factor;
    bool reverse_playback = x.time_stretch_factor < 0.0;
    
    x.start = start_pos;
    if(x.start < 0) x.start += (soundfile_size - 1);
    x.end = x.start + ((x.grain_size_samples - 1) * x.time_stretch_factor);
    
    if(x.end < 0) x.end += soundfile_size - 1;
    if(x.end > soundfile_size - 1) x.end -= (soundfile_size - 1);

    x.current_sample_pos = x.start;
    x.next_sample_pos = x.current_sample_pos + x.time_stretch_factor;
    
    if(reverse_playback)
    {
        if(x.next_sample_pos < 0) x.next_sample_pos += (soundfile_size - 1);
        if(x.next_sample_pos < x.end && x.start > x.end) x.next_sample_pos = x.end;
 
    }
    else
    {
        if(x.next_sample_pos > (soundfile_size - 1)) x.next_sample_pos -= (soundfile_size - 1);
        if(x.next_sample_pos >= x.end && x.start < x.end) x.next_sample_pos = x.end;
    }

    return x;
}
/**
 * @author Strobl, Micha <br>
 * @brief scheduling of grain playback
 * @details recursive scheduling of successive grain playback with time and/or start position shifts <br>
 * @param g grain <br>
 * @param synth pointer to c_granular_synth object that schedules the grain <br>
 */
void grain_internal_scheduling(grain* g, c_granular_synth* synth)
{
    if(synth->reverse_playback)
    {
        g->grain_active = g->grain_index == synth->current_grain_index ||
        ((((synth->soundfile_length - 1 - synth->playback_position) <= g->start) &&
          ((synth->soundfile_length - 1 - synth->playback_position) >= g->end)));
    }
    else
    {
        g->grain_active = g->grain_index == synth->current_grain_index ||
        ((g->start <= synth->playback_position) &&
         (g->end >= synth->playback_position));
    }
    
    if(g->grain_active)
    {
        float   left_sample, 
                right_sample, 
                frac, 
                integral, 
                weighted;
        
        left_sample = synth->soundfile_table[(int)floorf(g->current_sample_pos)];
        right_sample = synth->soundfile_table[(int)ceilf(g->current_sample_pos)];
        frac = modff(g->current_sample_pos, &integral);
        weighted = get_interpolated_sample_value(left_sample, right_sample,frac);
        synth->output_buffer += weighted;
        g->current_sample_pos = g->next_sample_pos;
        g->next_sample_pos += synth->pitch_factor;

        if(g->next_sample_pos > (synth->soundfile_length - 1))
        {
            g->next_sample_pos -= (synth->soundfile_length - 1);
        }
        
        if(g->next_sample_pos < 0.0)
        {
            g->next_sample_pos += (synth->soundfile_length - 1);
        }
        g->internal_step_count++;
        
        if(g->internal_step_count >= g->grain_size_samples)
        {
            g->current_sample_pos = g->start;
            g->next_sample_pos = g->current_sample_pos + synth->pitch_factor;
            g->internal_step_count = 0;
            synth->spray_true_offset = 0;
            c_granular_synth_reset_playback_position(synth);
        }
        
        if(g->next_grain)
        {
            grain_internal_scheduling(g->next_grain, synth);
        }
        
    }
    else {
        g->current_sample_pos = g->start;
        g->next_sample_pos = g->current_sample_pos + synth->pitch_factor;
        g->internal_step_count = 0;
        return;
        
    }
}
/**
 * @brief frees grain
 * @details frees grain <br>
 * @param x input pointer of grain object <br>
 */
void grain_free(grain *x)
{
    free(x);
}
