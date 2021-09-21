// duration in ms and/or samples
// dur_in_ms * (samplerate/1000) = dur_in_samples

// fade in/out -> hanning fenster in main file?

// start point [in samples] relative to the sound file -> PASS IN original playback point
// endpoint = startpoint + duration
// overlap

// length of the entire sound file [in samples]

#include "grain.h"
#include "envelope.h"
#include "vas_mem.h"

static t_class *grain_class;

#define SAMPLERATE 44100   // To-Do: Set dynamically by user input

grain *grain_new(int grain_size_samples, int soundfile_size, int grain_index)
{
    //Table aus Samples -> Abschnitt aus dem Original File
    // Duration in samples und ms
    // Pointer auf previous/next Sample?
    grain *x = (grain *) vas_mem_alloc(sizeof(grain));

    // calculate numbr of samples in Grain,
    //if floating point, cast to nearest higher integer witz ceil()
    x->grain_size_samples = grain_size_samples;
    x->start = x->grain_size_samples * grain_index;
    x->end = x->start + x->grain_size_samples - 1;
    if(x->end > soundfile_size)
    {
        x->end = soundfile_size - 1;
    }
    //post("Grain with index %d starts at %d and ends at %d", grain_index, x->start, x->end);

    return x;

    // use envelope_windowing, only multiply values from attack and release phase
    // as sustain phase should always contain "1" values
}



void grain_free(grain *x)
{
    free(x);
}
