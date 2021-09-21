/**
 * @file pd_granular_synth_pd.c
 * @author Nikita Kretschmar, Adrian Philipp, Micha Strobl, Tim Wennemann <br>
 * Audiocommunication Group, Technische Universität Berlin <br>
 * @brief Main file <br>
 * <br>
 * Main file
 */

#include "c_granular_synth.h"


// Absolute playback point [in samples]
// aendert sich nie, nur die Grainstartpunkte werden relativ zu diesem angesetzt
// implementieren mit skaliertem phasor~ object [0:1]->[0:length]

// entweder bekommen Grain Fade In/Out Parameter oder allen ein Hanning Fenster überlagert
// jeweils um Clicks am Start/Ende zu glaetten

static t_class *pd_granular_synth_tilde_class;

typedef struct pd_granular_synth_tilde
{
    t_object  x_obj;
    t_float f;
    c_granular_synth *synth;

    t_word *soundfile;      // Pointer to the soundfile Array
    t_symbol *soundfile_arrayname;  // String used in pd to identify array that holds the soundfile
    int soundfile_length;
    t_word *envelopeTable;

    t_inlet *in;
    t_outlet *out;
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

    x->soundfile = 0;
    x->soundfile_arrayname = soundfile_arrayname;
        post("soundfile arrayname: %s", x->soundfile_arrayname);
    x->soundfile_length = 0;
    x->envelopeTable = 0;
    //x->synth = c_granular_synth_new(30);        // Default value of 30ms
    //The main inlet is created automatically
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

    // Die eigentliche Soundverarbeitung steckt im C-Teil (c_granular_synth.c)

    c_granular_synth_process_alt(x->synth, in, out, n);

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
        outlet_free(x->out);
        c_granular_synth_free(x->synth);
        free(x);
    }
}

/**
 * @brief Reads the array containing the loaded soundfile. Modified version of a method in the course's repository.
 *
 * @param x The granular synth object that uses the soundfile's sample-data.
 */
static void pd_granular_synth_tilde_getArray(t_pd_granular_synth_tilde *x, t_symbol *s)
{
    // To-Do
    // siehe Session 5 rtap_osc6.c "...getArray"-Methode
    post("Array Name: %s",x->soundfile_arrayname->s_name);
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
        x->synth = c_granular_synth_new(x->soundfile, x->soundfile_length, 50);
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

      // this adds the gain message to our object
      // class_addmethod(pd_granular_synth_tilde_class, (t_method)pd_granular_synth_tilde_method, gensym("name"), A_DEFFLOAT,0);

      class_addmethod(pd_granular_synth_tilde_class, (t_method)pd_granular_synth_get_arrayname_message,
                        gensym("soundfile_arrayname"), A_DEFSYMBOL, 0);

      CLASS_MAINSIGNALIN(pd_granular_synth_tilde_class, t_pd_granular_synth_tilde, f);

      // Fetch the current system's samplerate in .h file, check here if value is assigned
      // SAMPLERATE variable is still a "shadowed declaration"... -> needs Fix!
      //t_float SAMPLERATE;
      //SAMPLERATE = sys_getsr();
      //if(SAMPLERATE > 0) post("SAMPLERATE = %f", SAMPLERATE);
      
      /*
      class_sethelpsymbol(pd_granular_synth_tilde_class, gensym("pd_granular_synth~"));

      Das hier oben haben die Grainmaker Leute bei sich am Ende der Setup Methode auch stehen...
      Wenn de aufgerufen wird (bei uns) kommt im pd-Project die Warnung "sample multiply defined"
      "sample" ist im Projekt der Name des Arrays in dem das soundfile ist. DasArray wird dann auch nicht geladen...
      */
      
}

void pd_granular_synth_noteOn(t_pd_granular_synth_tilde *x, float frequency, float velocity)
{
    c_granular_synth_noteOn(x->synth, frequency, velocity);
}
