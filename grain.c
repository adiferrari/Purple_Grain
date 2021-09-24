// duration in ms and/or samples
// dur_in_ms * (samplerate/1000) = dur_in_samples

// fade in/out -> hanning fenster in main file?

// start point [in samples] relative to the sound file -> PASS IN original playback point
// endpoint = startpoint + duration
// overlap

// length of the entire sound file [in samples]

#include "grain.h"

static t_class *grain_class;

#define SAMPLERATE 44100   // To-Do: Set dynamically by user input

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
    x.current_sample_pos = (float)x.start;
    x.next_sample_pos = x.current_sample_pos + x.time_stretch_factor;
    
    x.end = x.start + ((x.grain_size_samples - 1) * x.time_stretch_factor);
    if(x.end > soundfile_size)
    {
        x.end = soundfile_size - 1;
    }
    //post("Grain with index %d starts at %d and ends at %d", grain_index, x.start, x.end);

    return x;
}

void grain_internal_scheduling(grain* g, c_granular_synth* synth)
{
    g->grain_active = (g->start <= synth->playback_position && g->next_sample_pos >= synth->playback_position);
    if(g->grain_active)
    {
        float left_sample, right_sample, frac, integral, weighted;
        g->current_sample_pos = g->next_sample_pos;
        g->next_sample_pos += g->time_stretch_factor;
        if(g->next_sample_pos > synth->soundfile_length || g->current_sample_pos >= g->end)
        {
            //g->grain_active = false;
            // Grain wieder auf seinen Startpunkt setzen, wie bei Initialisierung in new-methode
            g->current_sample_pos = g->grain_size_samples * g->grain_index;
            g->next_sample_pos = g->current_sample_pos + g->time_stretch_factor;
        }
        // soundfile[g->current_sample_pos] in output-buffer (oder weighted) schreiben
        // interpol methode hier schon verwenden?
        // rückgabewert hier also float? oder float pointer?
        left_sample = synth->soundfile_table[(int)floor(g->current_sample_pos)];
        right_sample = synth->soundfile_table[(int)ceil(g->current_sample_pos)];
        frac = modff(g->current_sample_pos, &integral);
        weighted = get_interpolated_sanple_value(left_sample, right_sample,frac);
        synth->output_buffer += weighted;
        
        // checken ob nächstes grain aktiv ist
        grain_internal_scheduling(g->next_grain, synth);
    }
    else {
        // Grain nicht oder nicht mehr aktiv
        // seine current pos auf seinen start zurücksetzen
        return;
        
    }
}

void grain_free(grain *x)
{
    free(x);
}
