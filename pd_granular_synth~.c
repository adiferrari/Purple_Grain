/**
 * @file pd_granular_synth_pd.c
 * @author Nikita Kretschmar
 * @author Adrian Philipp
 * @author Micha Strobl
 * @author Tim Wennemann
 * Audiocommunication Group, Technische Universität Berlin <br>
 * @brief Main file <br>
 * <br>
 * Main file
 */

#include "c_granular_synth.h"
#include "purple_utils.h"


// Absolute playback point [in samples]
// aendert sich nie, nur die Grainstartpunkte werden relativ zu diesem angesetzt
// implementieren mit skaliertem phasor~ object [0:1]->[0:length]

// entweder bekommen Grain Fade In/Out Parameter oder allen ein Hanning Fenster überlagert
// jeweils um Clicks am Start/Ende zu glaetten

static t_class *pd_granular_synth_tilde_class;

/**
 * @brief 
 * @details 
 */
typedef struct pd_granular_synth_tilde
{
    t_object  x_obj;
    t_float f;
    t_float sr;
    c_granular_synth *synth;
    t_int               grain_size,
                        start_pos,
                        midi_pitch,
                        midi_velo,
                        attack,
                        decay,
                        release;
    t_float             sustain,
                        time_stretch_factor,
                        gauss_q_factor;
    t_word *soundfile;      // Pointer to the soundfile Array
    t_symbol *soundfile_arrayname;  // String used in pd to identify array that holds the soundfile
    int soundfile_length;
    float pitch_faktor;
    float soundfile_length_ms;
    t_word *envelopeTable;

    t_inlet             *in_grain_size,
                        *in_start_pos,
                        *in_time_stretch_factor,
                        *in_midi_pitch,
                        *in_midi_velo,
                        *in_attack,
                        *in_decay,
                        *in_sustain,
                        *in_release,
                        *in_gauss_q_factor;
    t_outlet            *out;
} t_pd_granular_synth_tilde;

/** 
 * @related pd_granular_synth_tilde
 * @brief Creates a new pd_granular_synth_tilde object.<br>
 * For more information please refer to the <a href = "https://github.com/pure-data/externals-howto" > Pure Data Docs </a> <br>
 */

void *pd_granular_synth_tilde_new(t_symbol *soundfile_arrayname)
{
    t_pd_granular_synth_tilde *x = (t_pd_granular_synth_tilde *)pd_new(pd_granular_synth_tilde_class);
    x->f = 0;
    x->sr  = sys_getsr();
    x->soundfile = 0;
    x->soundfile_arrayname = soundfile_arrayname;

    x->soundfile_length = 0;
    x->soundfile_length_ms = 0;
    x->grain_size = 50;
    x->start_pos = 0;
    x->time_stretch_factor = 1.0,
    x->midi_pitch = 48;
    x->pitch_faktor = 1;
    x->midi_velo = 0;
    x->attack = 500;
    x->decay = 500;
    x->sustain = 0.7;
    x->release = 1000;
    x->gauss_q_factor = 0.2;
    //The main inlet is created automatically
    
    x->in_grain_size = inlet_new(&x->x_obj,  &x->x_obj.ob_pd, &s_float, gensym("grain_size"));
    x->in_start_pos = inlet_new(&x->x_obj,  &x->x_obj.ob_pd, &s_float, gensym("start_pos"));
    x->in_time_stretch_factor = inlet_new(&x->x_obj,  &x->x_obj.ob_pd, &s_float, gensym("time_stretch_factor"));
    x->in_midi_pitch = inlet_new(&x->x_obj,  &x->x_obj.ob_pd, &s_float, gensym("midi_pitch"));
    x->in_midi_velo = inlet_new(&x->x_obj,  &x->x_obj.ob_pd, &s_float, gensym("midi_velo"));
    x->in_attack = inlet_new(&x->x_obj,  &x->x_obj.ob_pd, &s_float, gensym("attack"));
    x->in_decay = inlet_new(&x->x_obj,  &x->x_obj.ob_pd, &s_float, gensym("decay"));
    x->in_sustain = inlet_new(&x->x_obj,  &x->x_obj.ob_pd, &s_float, gensym("sustain"));
    x->in_release = inlet_new(&x->x_obj,  &x->x_obj.ob_pd, &s_float, gensym("release"));
    x->in_gauss_q_factor = inlet_new(&x->x_obj,  &x->x_obj.ob_pd, &s_float, gensym("gauss_q_factor"));
    
    x->out = outlet_new(&x->x_obj, &s_signal);
    return (void *)x;
}


/**
 * @related pd_granular_synth_tilde
 *
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

    c_granular_synth_properties_update(x->synth, x->grain_size, x->start_pos, x->time_stretch_factor, x->midi_velo, x->midi_pitch, x->attack, x->decay, x->sustain, x->release, x->gauss_q_factor);

    c_granular_synth_process(x->synth, in, out, n);
    /* return a pointer to the dataspace for the next dsp-object */
    /*
        the return argument equals the argument of the perform-routine plus the
        number of pointer variables (as declared as the second argument of dsp_add)
        plus one
    */
    return (w+5);
}

/**
 * @related pd_granular_synth_tilde
 * @brief Frees our object. <br>
 * @param x A pointer the pd_granular_synth_tilde object <br>
 * For more information please refer to the <a href = "https://github.com/pure-data/externals-howto" > Pure Data Docs </a> <br>
 */

void pd_granular_synth_tilde_free(t_pd_granular_synth_tilde *x)
{
    if(x){
        inlet_free(x->in_grain_size);
        inlet_free(x->in_start_pos);
        inlet_free(x->in_time_stretch_factor);
        inlet_free(x->in_midi_velo);
        inlet_free(x->in_midi_pitch);
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
 * @brief Reads the array containing the loaded soundfile.
 * @details Reads the array containing the loaded soundfile. Modified version of a method in the course's repository.
 * @param x The granular synth object that uses the soundfile's sample-data.
 */
static void pd_granular_synth_tilde_getArray(t_pd_granular_synth_tilde *x, t_symbol *s)
{
    t_garray *a;
    x->soundfile_arrayname = s;
    
    if (!(a = (t_garray *)pd_findbyclass(x->soundfile_arrayname, garray_class)))
    {
        
        if (*s->s_name)
        {
        //pd_error(x, "vas_binaural~: %s: no such array", x->soundfile_arrayname->s_name);
        post("Inner if-condition reached");
        x->soundfile = 0;
        }
        post("Get Array method if block reached");
    }

    else if (!garray_getfloatwords(a, &x->soundfile_length, &x->soundfile))
    {
        //pd_error(x, "%s: bad template for pd_granular_synth~", x->soundfile_arrayname->s_name);
        //x->soundfile = 0;
        post("Get Array method else if block reached");
    }
    else {
        garray_usedindsp(a);

        /* int len = garray_npoints(a);
        if(len == 0)
        {
            post("empty array");
        }
        else
        {
            post("Array Length = %d", len);
        } */
        x->soundfile_length = garray_npoints(a);
        x->soundfile_length_ms = get_ms_from_samples(x->soundfile_length, x->sr);
        x->synth = c_granular_synth_new(x->soundfile, x->soundfile_length, x->grain_size, x->start_pos, x->time_stretch_factor, x->attack, x->decay, x->sustain, x->release, x->gauss_q_factor, x->pitch_faktor, x->midi_pitch);
    }

    return;
}

/**
 * @related pd_granular_synth_tilde
 * @brief Adds pd_granular_synth_tilde to the signal chain. <br>
 */
void pd_granular_synth_tilde_dsp(t_pd_granular_synth_tilde *x, t_signal **sp)
{
    pd_granular_synth_tilde_getArray(x, x->soundfile_arrayname);
    dsp_add(pd_granular_synth_tilde_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}
/**
 * @brief 
 * 
 * @param x 
 * @param f 
 */
static void pd_granular_synth_set_grain_size(t_pd_granular_synth_tilde *x, t_floatarg f)
{
    int new_grain_size = (int)f;
    if(new_grain_size < 1) new_grain_size = 1;
    if(x->soundfile_length && new_grain_size > x->soundfile_length) {
        new_grain_size = x->soundfile_length;
        }
    x->grain_size = (int)new_grain_size;
    // Changes get passed to synth in update properties method
}
/**
 * @brief 
 * 
 * @param x 
 * @param f 
 */
static void pd_granular_synth_set_start_pos(t_pd_granular_synth_tilde *x, t_floatarg f)
{
    int new_start_pos = (int)f;
    if(new_start_pos < 0) new_start_pos = 0;
    if(x->soundfile_length && new_start_pos > x->soundfile_length) {
        new_start_pos = x->soundfile_length;
        }
    x->start_pos = (int)new_start_pos;
    // Changes get passed to synth in update properties method
}
/**
 * @brief 
 * 
 * @param x 
 * @param f 
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
            // Was tun wenn der Faktor 0 ist ???
            // war die Richtung zuvor positiv setze jetzt auf -0.1 und umgekehrt
            x->time_stretch_factor = (x->time_stretch_factor > 0) ? -0.1 : 0.1;
            // drehe reverse playback status um
            x->synth->reverse_playback = !x->synth->reverse_playback;
            return;
        }
    }
    x->time_stretch_factor = new_time_stretch_factor;
}
/**
 * @brief 
 * 
 * @param x 
 * @param f 
 */
static void pd_granular_synth_set_midi_pitch(t_pd_granular_synth_tilde *x, t_floatarg f)
{
    int new_midi_pitch = (int)f;
    if(new_midi_pitch < 0) new_midi_pitch = 0;
    x->midi_pitch = (int)new_midi_pitch;
}
/**
 * @brief 
 * 
 * @param x 
 * @param f 
 */
static void pd_granular_synth_set_midi_velo(t_pd_granular_synth_tilde *x, t_floatarg f)
{
    int new_midi_velo = (int)f;
    if(new_midi_velo < 0) new_midi_velo = 0;
    x->midi_velo = (int)new_midi_velo;
}
/**
 * @brief 
 * 
 * @param x 
 * @param f 
 */
static void pd_granular_synth_set_attack(t_pd_granular_synth_tilde *x, t_floatarg f)
{
    int new_attack = (int)f;
    if(new_attack < 0) new_attack = 0;
    x->attack = (int)new_attack;
}
/**
 * @brief 
 * 
 * @param x 
 * @param f 
 */
static void pd_granular_synth_set_decay(t_pd_granular_synth_tilde *x, t_floatarg f)
{
    int new_decay = (int)f;
    if(new_decay < 0) new_decay = 0;
    x->decay = (int)new_decay;
}
/**
 * @brief 
 * 
 * @param x 
 * @param f 
 */
static void pd_granular_synth_set_sustain(t_pd_granular_synth_tilde *x, t_floatarg f)
{
    float new_sustain = (float)f;
    if(new_sustain < 0) new_sustain = 0;
    x->sustain = (float)new_sustain;
}
/**
 * @brief 
 * 
 * @param x 
 * @param f 
 */
static void pd_granular_synth_set_release(t_pd_granular_synth_tilde *x, t_floatarg f)
{
    int new_release = (int)f;
    if(new_release < 0) new_release = 0;
    x->release = (int)new_release;
}

/**
 * @brief 
 * 
 * @param x 
 * @param f 
 */
static void pd_granular_synth_set_gauss_q_factor(t_pd_granular_synth_tilde *x, t_floatarg f)
{
    float new_gauss_q_factor = (float)f;
    if(new_gauss_q_factor < 0) new_gauss_q_factor = 0;
    x->gauss_q_factor = (float)new_gauss_q_factor;
}

/**
 * @brief 
 * 
 * @param x 
 * @param s 
 */
static void pd_granular_synth_get_arrayname_message(t_pd_granular_synth_tilde *x, t_symbol *s)
{
    x->soundfile_arrayname = s;
}

/**
 * @related pd_granular_synth_tilde
 * @brief Setup of pd_granular_synth_tilde <br>
 * For more information please refer to the <a href = "https://github.com/pure-data/externals-howto" > Pure Data Docs </a> <br>
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
      // Alternative Constructor for use of the name"purple grain" in PureData
      class_addcreator((t_newmethod)pd_granular_synth_tilde_new, gensym("purple_grain"),
                        A_DEFSYMBOL, 0);

      class_addmethod(pd_granular_synth_tilde_class, (t_method)pd_granular_synth_set_grain_size,
                    gensym("grain_size"), A_DEFFLOAT, 0);
      class_addmethod(pd_granular_synth_tilde_class, (t_method)pd_granular_synth_set_start_pos,
                    gensym("start_pos"), A_DEFFLOAT, 0);
      class_addmethod(pd_granular_synth_tilde_class, (t_method)pd_granular_synth_set_time_stretch_factor,
                  gensym("time_stretch_factor"), A_DEFFLOAT, 0);
      class_addmethod(pd_granular_synth_tilde_class, (t_method)pd_granular_synth_set_midi_pitch,
                    gensym("midi_pitch"), A_DEFFLOAT, 0);
      class_addmethod(pd_granular_synth_tilde_class, (t_method)pd_granular_synth_set_midi_velo,
                    gensym("midi_velo"), A_DEFFLOAT, 0);
      class_addmethod(pd_granular_synth_tilde_class, (t_method)pd_granular_synth_set_attack,
                    gensym("attack"), A_DEFFLOAT, 0);
      class_addmethod(pd_granular_synth_tilde_class, (t_method)pd_granular_synth_set_decay,
                    gensym("decay"), A_DEFFLOAT, 0);
      class_addmethod(pd_granular_synth_tilde_class, (t_method)pd_granular_synth_set_sustain,
                    gensym("sustain"), A_DEFFLOAT, 0);
      class_addmethod(pd_granular_synth_tilde_class, (t_method)pd_granular_synth_set_release,
                    gensym("release"), A_DEFFLOAT, 0);
      class_addmethod(pd_granular_synth_tilde_class, (t_method)pd_granular_synth_set_gauss_q_factor,
                    gensym("gauss_q_factor"), A_DEFFLOAT, 0);
    
      class_addmethod(pd_granular_synth_tilde_class, (t_method)pd_granular_synth_get_arrayname_message,
                        gensym("soundfile_arrayname"), A_DEFSYMBOL, 0);

      CLASS_MAINSIGNALIN(pd_granular_synth_tilde_class, t_pd_granular_synth_tilde, f);

      /*
      class_sethelpsymbol(pd_granular_synth_tilde_class, gensym("pd_granular_synth~"));

      Das hier oben haben die Grainmaker Leute bei sich am Ende der Setup Methode auch stehen...
      Wenn de aufgerufen wird (bei uns) kommt im pd-Project die Warnung "sample multiply defined"
      "sample" ist im Projekt der Name des Arrays in dem das soundfile ist. DasArray wird dann auch nicht geladen...
      */
      
}

// Obsolete?
void pd_granular_synth_noteOn(t_pd_granular_synth_tilde *x, float frequency, float velocity)
{
    c_granular_synth_noteOn(x->synth, frequency, velocity);
}
