/**
 * @file pd_granular_synth_tilde.c
 * @author Kretschmar, Nikita 
 * @author Philipp, Adrian 
 * @author Strobl, Micha 
 * @author Wennemann,Tim <br>
 * Audiocommunication Group, Technische Universität Berlin <br>
 * @brief Main file of the pure data external
 * @details Main file of the pure data external, generates in- and outlets of the pure data granular synth object, updates values corresponding to input slider states <br>
 * @todo Incorporate pointers to previous grains <br>
 * @todo Define maximum grain scheduling as grain density <br>
 * @todo Smoothen output buffer values when grains overlap <br>
 * @todo Incorporate more windowing functions apart from Gauss <br>
 * @todo Pitch detection of samples <br>
 */

#include "c_granular_synth.h"
#include "purple_utils.h"

static t_class *pd_granular_synth_tilde_class;

/**
 * @struct c_granular_synth_tilde_
 * @brief pure data struct of the @a c_granular_synth_tilde object
 * @details pure data struct of the @a c_granular_synth_tilde object, sets all necessary in- and outlets and defines corresponding variables for synth operation <br>
 */
typedef struct pd_granular_synth_tilde
{
    t_object  x_obj;                                    ///< object used for method input/output handling <b>
    t_float f;                                          ///< of type float, used for various input handling <b>
    t_float sr;                                         ///< defined samplerate <b>
    c_granular_synth *synth;                            ///< pure data granular synth object <b>
    t_int               start_pos,                      ///< position within the soundfile, adjustable through slider <br>
                        midi_pitch,                     ///< pitch/key value given by MIDI input <br>
                        midi_velo,                      ///< velocity value given by MIDI input <br>
                        attack,                         ///< attack time in the range of 0 - 4000ms, adjustable through slider <br>
                        decay,                          ///< decay time in the range of 0 - 4000ms, adjustable through slider <br>
                        release,                        ///< release time in the range of 0 - 10000ms, adjustable through slider <br>
                        spray_input;                    ///< randomizes the start position of each grain in the range of 0 - 75, adjustable through slider <br>
    t_float             sustain,                        ///< sustain time in the range of 0 - 1, adjustable through slider <br>
                        time_stretch_factor,            ///< resizes sample length within a grain, for negative values read samples in backwards direction, adjustable through slider <br>
                        gauss_q_factor;                 ///< used to manipulate grain envelope slope in the range of 0.01 - 1, adjustable through slider <br>
    t_word              *soundfile;                     ///< Pointer to the soundfile Array <br>
    t_symbol            *soundfile_arrayname;           ///< String used in pd to identify array that holds the soundfile <br>
    int                 grain_size,                     ///< size of a grain in milliseconds, adjustable through slider <br>          
                        soundfile_length;               ///< lenght of the soundfile in samples <b>
    float               pitch_factor,                   ///< scaled by pitch/key value given by MIDI input <br>
                        soundfile_length_ms;            ///< lenght of the soundfile in milliseconds <b>

    t_inlet             *in_midi_pitch,                 ///< inlet for MIDI input pitch/key value <br>
                        *in_midi_velo,                  ///< inlet for MIDI input velocity value <br>
                        *in_start_pos,                  ///< inlet for start position slider <br>
                        *in_grain_size,                 ///< inlet for grain size slider <br> 
                        *in_time_stretch_factor,        ///< inlet for time stretch factor slider <br>
                        *in_gauss_q_factor,             ///< inlet for gauss q factor slider <br>
                        *in_spray,                      ///< inlet for spray slider <br>
                        *in_attack,                     ///< inlet attack slider <br>
                        *in_decay,                      ///< inlet for decay slider <br>
                        *in_sustain,                    ///< inlet for sustain slider <br>
                        *in_release;                    ///< inlet for release slider <br>;
    t_outlet            *out;                           ///< main outlet <br>
} t_pd_granular_synth_tilde;

/** 
 * @related pd_granular_synth_tilde
 * @brief Creates a new pd_granular_synth_tilde object.<br>
 * @details 
 */

void *pd_granular_synth_tilde_new(t_symbol *soundfile_arrayname)
{
    t_pd_granular_synth_tilde *x = (t_pd_granular_synth_tilde *)pd_new(pd_granular_synth_tilde_class);
    x->f = 0;
    x->sr  = sys_getsr();
    x->soundfile = 0;
    x->soundfile_arrayname = soundfile_arrayname;

    x->soundfile_length = 0;                            ///< default value for soundfile length in samples <b>
    x->soundfile_length_ms = 0;                         ///< default value for soundfile length in ms <b>
    x->grain_size = 50;                                 ///< default value for grain size, before adjustment through slider <b>
    x->start_pos = 0;                                   ///< default value for starting position, before adjustment through slider <b>
    x->time_stretch_factor = 1.0,                       ///< default value for time stretch factor, before adjustment through slider <b>
    x->pitch_factor = 1;                                ///< default value for pitch factor, before adjustment through slider <b>
    x->midi_velo = 0;                                   ///< default value for MIDI input velocity, equals noteoff event <b>
    x->midi_pitch = 48;                                 ///< default value for MIDI input pitch/key, equals note C3 <b>
    x->attack = 500;                                    ///< default value for attack time, before adjustment through slider <b>
    x->decay = 500;                                     ///< default value for decay time, before adjustment through slider <b>
    x->sustain = 0.7;                                   ///< default value for sustain time, before adjustment through slider <b>
    x->release = 1000;                                  ///< default value for release time, before adjustment through slider <b>
    x->gauss_q_factor = 0.2;                            ///< default value for gauss q factor, before adjustment through slider <b>
    x->spray_input = 0;                                 ///< default value for spray randomizer, before adjustment through slider <b>
    
    /// @note The main inlet is created automatically
    x->in_midi_pitch = inlet_new(&x->x_obj,  &x->x_obj.ob_pd, &s_float, gensym("midi_pitch"));
    x->in_midi_velo = inlet_new(&x->x_obj,  &x->x_obj.ob_pd, &s_float, gensym("midi_velo"));
    x->in_start_pos = inlet_new(&x->x_obj,  &x->x_obj.ob_pd, &s_float, gensym("start_pos"));
    x->in_grain_size = inlet_new(&x->x_obj,  &x->x_obj.ob_pd, &s_float, gensym("grain_size"));
    x->in_time_stretch_factor = inlet_new(&x->x_obj,  &x->x_obj.ob_pd, &s_float, gensym("time_stretch_factor"));
    x->in_gauss_q_factor = inlet_new(&x->x_obj,  &x->x_obj.ob_pd, &s_float, gensym("gauss_q_factor"));
    x->in_spray = inlet_new(&x->x_obj,  &x->x_obj.ob_pd, &s_float, gensym("spray"));
    x->in_attack = inlet_new(&x->x_obj,  &x->x_obj.ob_pd, &s_float, gensym("attack"));
    x->in_decay = inlet_new(&x->x_obj,  &x->x_obj.ob_pd, &s_float, gensym("decay"));
    x->in_sustain = inlet_new(&x->x_obj,  &x->x_obj.ob_pd, &s_float, gensym("sustain"));
    x->in_release = inlet_new(&x->x_obj,  &x->x_obj.ob_pd, &s_float, gensym("release"));
    
    x->out = outlet_new(&x->x_obj, &s_signal);
    return (void *)x;
}


/**
 * @related pd_granular_synth_tilde
 * @brief performs @a pd_granular_synth_tilde
 * @param w main input for performing pd_granular_synth_tilde
 */

t_int *pd_granular_synth_tilde_perform(t_int *w)
{
    t_pd_granular_synth_tilde *x = (t_pd_granular_synth_tilde *)(w[1]);
    t_sample  *in = (t_sample *)(w[2]);
    t_sample  *out =  (t_sample *)(w[3]);
    int n =  (int)(w[4]);

    if(x->grain_size < 1) x->grain_size = 1;
    if(x->grain_size >  (int)x->soundfile_length) x->grain_size = x->soundfile_length;
    if(x->start_pos < 0) x->start_pos = 0;
    if(x->start_pos > (int)x->soundfile_length) x->start_pos = x->soundfile_length - 1;

    c_granular_synth_properties_update(x->synth, x->grain_size, x->start_pos, x->time_stretch_factor, x->midi_velo, x->midi_pitch, x->attack, x->decay, x->sustain, x->release, x->gauss_q_factor, x->spray_input); ///< passes all (slider) changes to synth

    c_granular_synth_process(x->synth, in, out, n); ///< returns pointer to dataspace for the next dsp-object

    return (w+5); ///< returns argument equal to argument of the perform-routine plus the number of pointer variables +1
}

/**
 * @related pd_granular_synth_tilde
 * @brief frees inlets
 * @details frees inlets of @a pd_granular_synth_tilde <b>
 * @param x input pointer of @a pd_granular_synth_tilde object <br>
 */

void pd_granular_synth_tilde_free(t_pd_granular_synth_tilde *x)
{
    if(x){
        inlet_free(x->in_midi_velo);
        inlet_free(x->in_midi_pitch);
        inlet_free(x->in_start_pos);
        inlet_free(x->in_grain_size);
        inlet_free(x->in_time_stretch_factor);
        inlet_free(x->in_gauss_q_factor);
        inlet_free(x->in_spray);
        inlet_free(x->in_attack);
        inlet_free(x->in_decay);
        inlet_free(x->in_sustain);
        inlet_free(x->in_release);
        outlet_free(x->out);
        c_granular_synth_free(x->synth);
        free(x);
    }
}

/**
 * @brief reads the array containing the loaded soundfile
 * @details reads the array containing the loaded soundfile, modified version of a method in the course's repository <br>
 * @param x granular synth object that uses the soundfile's sample-data <br>
 */
static void pd_granular_synth_tilde_getArray(t_pd_granular_synth_tilde *x, t_symbol *s)
{
    t_garray *a;
    x->soundfile_arrayname = s;
    
    if (!(a = (t_garray *)pd_findbyclass(x->soundfile_arrayname, garray_class)))
    {
        if (*s->s_name)
        {
        post("Inner if-condition reached");
        x->soundfile = 0;
        }
        post("Get Array method if block reached");
    }
    else if (!garray_getfloatwords(a, &x->soundfile_length, &x->soundfile))
    {
        post("Get Array method else if block reached"); 
    }
    else {
        garray_usedindsp(a);

        x->soundfile_length = garray_npoints(a);
        x->soundfile_length_ms = get_ms_from_samples(x->soundfile_length, x->sr);
        x->synth = c_granular_synth_new(x->soundfile, x->soundfile_length, x->grain_size, x->start_pos, x->time_stretch_factor, x->attack, x->decay, x->sustain, x->release, x->gauss_q_factor, x->spray_input, x->pitch_factor, x->midi_pitch);
    }
    return;
}

/**
 * @related pd_granular_synth_tilde
 * @brief adds @a pd_granular_synth_tilde to the signal processing chain
 * @details adds @a pd_granular_synth_tilde to the signal processing chain, activate in pd window by checking the mark at 'DSP' option<br>
 */
void pd_granular_synth_tilde_dsp(t_pd_granular_synth_tilde *x, t_signal **sp)
{
    pd_granular_synth_tilde_getArray(x, x->soundfile_arrayname);
    dsp_add(pd_granular_synth_tilde_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}
/**
 * @related t_pd_granular_synth_tilde
 * @brief sets grain size
 * @details sets size of a grain in samples, adjustable through slider <br>
 * @param x input pointer of the @a pd_granular_synth_set_grain_size object <br>
 * @param f argument of type float for handling grain size input <br>
 */
static void pd_granular_synth_set_grain_size(t_pd_granular_synth_tilde *x, t_floatarg f)
{
    int new_grain_size = (int)f;
    if(new_grain_size < 1) new_grain_size = 1;
    if(x->soundfile_length && new_grain_size > x->soundfile_length) {
        new_grain_size = x->soundfile_length;
        }
    x->grain_size = (int)new_grain_size;
}
/**
 * @related t_pd_granular_synth_tilde
 * @brief sets starting position
 * @details sets the starting position within the soundfile, adjustable through slider <br>
 * @param x input pointer of the @a pd_granular_synth_set_start_pos object <br>
 * @param f argument of type float for handling starting position input <br>
 */
static void pd_granular_synth_set_start_pos(t_pd_granular_synth_tilde *x, t_floatarg f)
{
    t_int new_start_pos = (t_int)f;
    if(new_start_pos < 0) new_start_pos = 0;
    if(x->soundfile_length && new_start_pos > x->soundfile_length) {
        new_start_pos = x->soundfile_length;
        }
    x->start_pos = new_start_pos;
}
/**
 * @related t_pd_granular_synth_tilde
 * @brief sets time stretch factor
 * @details sets time stretch factor used to resize sample length within a grain, adjustable through slider <br>
 * @param x input pointer of the @a pd_granular_synth_set_time_stretch_factor object <br>
 * @param f argument of type float for handling time stretch factor input <br>
 */
static void pd_granular_synth_set_time_stretch_factor(t_pd_granular_synth_tilde *x, t_floatarg f)
{
    float new_time_stretch_factor = f;
    if(x->synth)
    {
        if((!x->synth->reverse_playback && new_time_stretch_factor < 0)
           || (x->synth->reverse_playback && new_time_stretch_factor > 0)
           || fabsf(new_time_stretch_factor) < 0.1)
        { 
            x->time_stretch_factor = (x->time_stretch_factor > 0) ? -0.1 : 0.1;
            x->synth->reverse_playback = !x->synth->reverse_playback; ///< inverts reverse playback state
            return;
        }
    }
    x->time_stretch_factor = new_time_stretch_factor;
}
/**
 * @related t_pd_granular_synth_tilde
 * @brief sets MIDI pitch/key
 * @details MIDI input pitch/key value, usable through virtual or external MIDI device <br>
 * @param x input pointer of the @a pd_granular_synth_set_midi_pitch
 * @param f argument of type float for handling MIDI pitch/key input
 */
static void pd_granular_synth_set_midi_pitch(t_pd_granular_synth_tilde *x, t_floatarg f)
{
    int new_midi_pitch = (int)f;
    if(new_midi_pitch < 0) new_midi_pitch = 0;
    x->midi_pitch = (int)new_midi_pitch;
}
/**
 * @related t_pd_granular_synth_tilde
 * @brief sets MIDI velocity
 * @details MIDI input velocity value, usable through virtual or external MIDI device, also used for noteon detection <br>
 * @param x input pointer of the @a pd_granular_synth_set_midi_velo <br>
 * @param f argument of type float for handling MIDI velocity input <br>
 */
static void pd_granular_synth_set_midi_velo(t_pd_granular_synth_tilde *x, t_floatarg f)
{
    int new_midi_velo = (int)f;
    if(new_midi_velo < 0) new_midi_velo = 0;
    x->midi_velo = (int)new_midi_velo;
}
/**
 * @related t_pd_granular_synth_tilde
 * @brief sets attack
 * @details sets the attack time component of the ADSR, adjustable through slider <br>
 * @param x input pointer of the @a pd_granular_synth_set_attack object <br>
 * @param f argument of type float for handling attack slider input <br>
 */
static void pd_granular_synth_set_attack(t_pd_granular_synth_tilde *x, t_floatarg f)
{
    int new_attack = (int)f;
    if(new_attack < 0) new_attack = 0;
    x->attack = (int)new_attack;
}
/**
 * @related t_pd_granular_synth_tilde
 * @brief sets decay
 * @details sets the decay time component of the ADSR, adjustable through slider <br>
 * @param x input pointer of the @a pd_granular_synth_set_decay object <br>
 * @param f argument of type float for handling decay slider input <br>
 */
static void pd_granular_synth_set_decay(t_pd_granular_synth_tilde *x, t_floatarg f)
{
    int new_decay = (int)f;
    if(new_decay < 0) new_decay = 0;
    x->decay = (int)new_decay;
}
/**
 * @related t_pd_granular_synth_tilde
 * @brief sets sustain time
 * @details sets the sustain time component of the ADSR, adjustable through slider <br>
 * @param x input pointer of the @a pd_granular_synth_set_sustain object <br>
 * @param f argument of type float for handling sustain slider input <br>
 */
static void pd_granular_synth_set_sustain(t_pd_granular_synth_tilde *x, t_floatarg f)
{
    float new_sustain = (float)f;
    if(new_sustain < 0) new_sustain = 0;
    x->sustain = (float)new_sustain;
}
/**
 * @related t_pd_granular_synth_tilde
 * @brief sets release time
 * @details sets the release time component of the ADSR, adjustable through slider <br>
 * @param x input pointer of the @a pd_granular_synth_set_release object <br>
 * @param f argument of type float for handling release slider input <br>
 */
static void pd_granular_synth_set_release(t_pd_granular_synth_tilde *x, t_floatarg f)
{
    int new_release = (int)f;
    if(new_release < 0) new_release = 0;
    x->release = (int)new_release;
}

/**
 * @related t_pd_granular_synth_tilde
 * @brief sets gauss q factor
 * @details sets the gauss q factor for manipulating grain envelopes, adjustable through slider <br>
 * @param x input pointer of the @a pd_granular_synth_set_gauss_q_factor object <br>
 * @param f argument of type float for handling gauss q factor slider input <br>
 */
static void pd_granular_synth_set_gauss_q_factor(t_pd_granular_synth_tilde *x, t_floatarg f)
{
    float new_gauss_q_factor = f;
    if(new_gauss_q_factor < 0) new_gauss_q_factor = 0;
    x->gauss_q_factor = (float)new_gauss_q_factor;
}

/**
 * @brief randomizes the start position of each grain
 * @details randomizes the start position of each grain, value is prior converted into samples, adjustable through slider <b>
 * @param x input pointer of the @a pd_granular_synth_set_spray_input object <br>
 * @param f argument of type float for handling spray slider input <br>
 */
static void pd_granular_synth_set_spray_input(t_pd_granular_synth_tilde *x, t_floatarg f)
{
    int new_spray = (int)f;
    x->spray_input = get_samples_from_ms(new_spray, x->sr);
}

/**
 * @related pd_granular_synth_tilde
 * @brief setup of pd_granular_synth_tilde
 * @details setup of pd_granular_synth_tilde, with alternative constructor for using the name 'purple grain' in puredata <br>
 */

void pd_granular_synth_tilde_setup(void)
{
      pd_granular_synth_tilde_class = class_new(gensym("pd_granular_synth~"),
            (t_newmethod)pd_granular_synth_tilde_new,
            (t_method)pd_granular_synth_tilde_free,
            sizeof(t_pd_granular_synth_tilde),
            CLASS_DEFAULT,
            A_DEFSYM, 0);

      class_addmethod(pd_granular_synth_tilde_class, (t_method)pd_granular_synth_tilde_dsp,
            gensym("dsp"), A_CANT, 0);
      class_addcreator((t_newmethod)pd_granular_synth_tilde_new, gensym("purple_grain"),
            A_DEFSYMBOL, 0);

      class_addmethod(pd_granular_synth_tilde_class, (t_method)pd_granular_synth_set_midi_pitch,
        gensym("midi_pitch"), A_DEFFLOAT, 0);
      class_addmethod(pd_granular_synth_tilde_class, (t_method)pd_granular_synth_set_midi_velo,
        gensym("midi_velo"), A_DEFFLOAT, 0);
      class_addmethod(pd_granular_synth_tilde_class, (t_method)pd_granular_synth_set_start_pos,
        gensym("start_pos"), A_DEFFLOAT, 0);
      class_addmethod(pd_granular_synth_tilde_class, (t_method)pd_granular_synth_set_grain_size,
        gensym("grain_size"), A_DEFFLOAT, 0);
      class_addmethod(pd_granular_synth_tilde_class, (t_method)pd_granular_synth_set_time_stretch_factor,
        gensym("time_stretch_factor"), A_DEFFLOAT, 0);
      class_addmethod(pd_granular_synth_tilde_class, (t_method)pd_granular_synth_set_gauss_q_factor,
        gensym("gauss_q_factor"), A_DEFFLOAT, 0);
      class_addmethod(pd_granular_synth_tilde_class, (t_method)pd_granular_synth_set_spray_input,
        gensym("spray"), A_DEFFLOAT, 0);
      class_addmethod(pd_granular_synth_tilde_class, (t_method)pd_granular_synth_set_attack,
        gensym("attack"), A_DEFFLOAT, 0);
      class_addmethod(pd_granular_synth_tilde_class, (t_method)pd_granular_synth_set_decay,
        gensym("decay"), A_DEFFLOAT, 0);
      class_addmethod(pd_granular_synth_tilde_class, (t_method)pd_granular_synth_set_sustain,
        gensym("sustain"), A_DEFFLOAT, 0);
      class_addmethod(pd_granular_synth_tilde_class, (t_method)pd_granular_synth_set_release,
        gensym("release"), A_DEFFLOAT, 0);

      CLASS_MAINSIGNALIN(pd_granular_synth_tilde_class, t_pd_granular_synth_tilde, f);   
}
