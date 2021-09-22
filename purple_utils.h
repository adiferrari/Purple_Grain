//
//  purple_utils.h
//  pd_granular_synth~
//
//  Created by Micha Strobl on 22.09.21.
//  Copyright © 2021 Intrinsic Audio. All rights reserved.
//

#ifndef purple_utils_h
#define purple_utils_h

int get_samples_from_ms(int ms, float sr);

float get_interpolated_sanple_value(float sample_left, float sample_right, float frac);

#endif /* purple_utils_h */
