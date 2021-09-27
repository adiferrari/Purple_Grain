// duration in ms and/or samples
// dur_in_ms * (samplerate/1000) = dur_in_samples

// fade in/out -> hanning fenster in main file?

// start point [in samples] relative to the sound file -> PASS IN original playback point
// endpoint = startpoint + duration
// overlap

// length of the entire sound file [in samples]
/**
 * @file grain.c
 * @author Nikita Kretschmar, Adrian Philipp, Micha Strobl, Tim Wennemann <br>
 * Audiocommunication Group, Technische Universität Berlin <br>
 * @brief handles grain creation
 * @version 0.1
 * @date 2021-09-27
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "grain.h"
#include "c_granular_synth.h"
#include "envelope.h"
#include "purple_utils.h"
#include "vas_mem.h"

static t_class *grain_class;

#define SAMPLERATE 44100   ///< To-Do: Set dynamically by user input
/**
 * @brief generates new grain depending on @param grain_size_samples, @param soundfile_size and @param grain_index
 * 
 * @param grain_size_samples size of samples contained in a grain
 * @param soundfile_size size of the soundfile which can be read in via inlet
 * @param grain_index corresponding index of a grain
 * @param time_stretch_factor resizes sample length within a grain, adjustable through slider 
 * @return grain 
 */
grain grain_new(int grain_size_samples, int soundfile_size, int grain_index, float time_stretch_factor)
{
    grain x;
    grain *next_grain = NULL;
    x.grain_active = false;
    // calculate numbr of samples in Grain,
    //if floating point, cast to nearest higher integer witz ceil()
    x.grain_size_samples = grain_size_samples;
    x.grain_index = grain_index;
    x.time_stretch_factor = time_stretch_factor;
    
    x.start = x.grain_size_samples * grain_index * x.time_stretch_factor;
    // For negative time_stretch_factor values read samples in backwards direction
    if(x.start < 0)
    {
        // ???
    }
    x.current_sample_pos = (float)x.start;
    x.next_sample_pos = x.current_sample_pos + x.time_stretch_factor;
    
    x.end = x.start + ((x.grain_size_samples - 1) * x.time_stretch_factor);
    // If the endpoint exceeds the soundfile length in positive or negative direction
    // clamp the grain length to a point the size of the file
    if(abs((int)floor(x.end))  > soundfile_size)
    {
        x.end = soundfile_size - 1;
        if(x.start < 0) x.end *= (-1);
    }
    
    //post("Grain with index %d starts at %d and ends at %d", grain_index, x.start, x.end);

    return x;
}
/**
 * @brief scheduling of grain playback
 * sheduling of grain playback
 * @param g grain
 * @param synth synthesized output of c_granular_synth object
 */
void grain_internal_scheduling(grain* g, c_granular_synth* synth)
{
    g->grain_active = (g->start <= synth->playback_position && g->end >= synth->playback_position);
    if(g->grain_active)
    {
        float   left_sample, ///<
                right_sample, ///<
                frac, ///<
                integral, ///<
                weighted;///<
        
        // For negative time_stretch_factor values read samples in backwards direction
        
        left_sample = synth->soundfile_table[(int)floor(g->current_sample_pos)];
        right_sample = synth->soundfile_table[(int)ceil(g->current_sample_pos)];
        frac = modff(g->current_sample_pos, &integral);
        weighted = get_interpolated_sanple_value(left_sample, right_sample,frac);
        synth->output_buffer += weighted;
        g->current_sample_pos = g->next_sample_pos;
        g->next_sample_pos += g->time_stretch_factor;
        if(g->next_sample_pos > synth->soundfile_length || g->current_sample_pos >= g->end)
        {
            //g->grain_active = false;
            g->current_sample_pos = g->grain_size_samples * g->grain_index; ///< sets grain position to start
            g->next_sample_pos = g->current_sample_pos + g->time_stretch_factor;
        }
        
        
        grain_internal_scheduling(g->next_grain, synth); ///< checks active state of grain
    }
    else {
        // Grain nicht oder nicht mehr aktiv
        g->current_sample_pos = g->grain_size_samples * g->grain_index; ///< sets current position to start
        g->next_sample_pos = g->current_sample_pos + g->time_stretch_factor;
        return;
        
    }
}
/**
 * @brief frees grain
 * frees grain
 * @param x input pointer of grain_fre object
 */
void grain_free(grain *x)
{
    free(x);
}
