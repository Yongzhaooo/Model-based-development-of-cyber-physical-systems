
#include "constants.h"

const struct crazyflie_constants_t crazyflie_constants = {
    .m = 0.027,
    .g = 9.81,
    .d = 0.046,
    .b = 2.5383e-11,
    .k = 1.9796e-9,
    .j_x = 1.1463e-5,
    .j_y = 1.6993e-5,
    .j_z = 2.9944e-5,
    .pwm_n = 0.06*9.81/65536.0/4.0,
};
