// duration in ms and/or samples
// dur_in_ms * (samplerate/1000) = dur_in_samples

// fade in/out -> hanning fenster in main file?

// start point [in samples] relative to the sound file -> PASS IN original playback point
// endpoint = startpoint + duration
// overlap

// length of the entire sound file [in samples]
/**
 * @file grain.c
 * @author Nikita Kretschmar
 * @author Adrian Philipp
 * @author Micha Strobl
 * @author Tim Wennemann
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
/**
 * @brief set samplerate to 44100
 * @todo make samplerate adjustable
 */
#define SAMPLERATE 44100

/**
 * @brief generates new grain
 * @details generates new grain depending on @a grain_size_samples, @a soundfile_size and @a grain_index
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
    x.grain_size_samples = grain_size_samples;
    x.grain_index = grain_index;
    x.time_stretch_factor = time_stretch_factor;
    
    x.start = fabsf(x.grain_size_samples * grain_index * x.time_stretch_factor);
    x.end = x.start + ((x.grain_size_samples - 1) * x.time_stretch_factor);
    if(x.end < 0) x.end = soundfile_size - 1 - x.end;
    if(x.end > soundfile_size - 1) x.end = soundfile_size - 1;
    
    if(time_stretch_factor < 0.0)
    {
        switch_float_values(&x.start, &x.end);
    }
    
    x.current_sample_pos = x.start;
    x.next_sample_pos = x.current_sample_pos + x.time_stretch_factor;
    if(x.next_sample_pos < 0) x.next_sample_pos = soundfile_size - 1 - x.next_sample_pos;
    if(x.next_sample_pos >= x.end) x.next_sample_pos = x.end;
    
    // If the endpoint exceeds the soundfile length in positive or negative direction
    // clamp the grain length to a point the size of the file
    // maybe just use fabsf(x.end) < soundfile_size

    return x;
}
/**
 * @brief scheduling of grain playback
 * @details sheduling of grain playback
 * @param g grain
 * @param synth synthesized output of c_granular_synth object
 */
void grain_internal_scheduling(grain* g, c_granular_synth* synth)
{
    if(synth->time_stretch_factor <= -1.0)
    {
        //
    }
    if(synth->reverse_playback)
    {
        g->grain_active = (g->start >= synth->playback_position) && (g->end <= synth->playback_position);
    }
    else
    {
        // is abs necessary?
        g->grain_active = (g->start <= synth->playback_position) && (g->end >= synth->playback_position);
    }
    
    if(g->grain_active)
    {
        float   left_sample, ///<
                right_sample, ///<
                frac, ///<
                integral, ///<
                weighted;///<
        
        // For negative time_stretch_factor values read samples in backwards direction
        left_sample = synth->soundfile_table[(int)floorf(g->current_sample_pos)];
        right_sample = synth->soundfile_table[(int)ceilf(g->current_sample_pos)];
        frac = modff(g->current_sample_pos, &integral);
        weighted = get_interpolated_sample_value(left_sample, right_sample,frac);
        synth->output_buffer += weighted;
        g->current_sample_pos = g->next_sample_pos;
        g->next_sample_pos += g->time_stretch_factor;
        
        if((!synth->reverse_playback && g->current_sample_pos >= g->end)
           || (synth->reverse_playback && g->current_sample_pos <= g->end)
           || g->next_sample_pos > synth->soundfile_length - 1
           || g->next_sample_pos < 0.0)
        {
            //g->grain_active = false;
            // Grain wieder auf seinen Startpunkt setzen, wie bei Initialisierung in new-methode
            g->current_sample_pos = g->start;
            g->next_sample_pos = g->current_sample_pos + g->time_stretch_factor;
        }
        
        // checken ob nächstes grain aktiv ist
        grain_internal_scheduling(g->next_grain, synth);
    }
    else {
        // Grain nicht oder nicht mehr aktiv
        // seine current pos auf seinen start zurücksetzen
        g->current_sample_pos = g->start;
        g->next_sample_pos = g->current_sample_pos + g->time_stretch_factor;
        /*
        g->current_sample_pos = g->grain_size_samples * g->grain_index * g->time_stretch_factor;
        g->next_sample_pos = g->current_sample_pos + g->time_stretch_factor;
         */
        return;
        
    }
}
/**
 * @brief frees grain
 * @details frees grain
 * @param x input pointer of grain_fre object
 */
void grain_free(grain *x)
{
    free(x);
}
