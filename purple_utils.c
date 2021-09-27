//
//  purple_utils.c
//  pd_granular_synth~
//
//  Created by Micha Strobl on 22.09.21.
//  Copyright © 2021 Intrinsic Audio. All rights reserved.
//

#include <stdio.h>
#include <math.h>
#include "m_pd.h"
#include "purple_utils.h"

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

float get_interpolated_sample_value(float sample_left, float sample_right, float frac)
{
    float weighted_a = sample_left * (1 - frac);
    float weighted_b = sample_right * frac;
    return (weighted_a + weighted_b);
}

void switch_float_values(float *a, float *b)
{
    float *temp_ptr = a;
    a = b;
    b = temp_ptr;
    return;
}
