
#ifndef CONSTANTS_H
#define CONSTANTS_H

struct crazyflie_constants_t {
    double m; // Mass
    double g; // Gravitational acceleration
    double d; // Distance from center of mass to rotor axis
    double k; // Drag constant k
    double b; // Lift constant b
    double j_x; // Diagonal
    double j_y; // inertia
    double j_z; // matrix
    double pwm_n; // pwm to newton conversion rate
};

const struct crazyflie_constants_t crazyflie_constants;


#define SEN_X 0
#define SEN_Y 1
#define SEN_Z 2

#define STATE_X 0
#define STATE_Y 1
#define STATE_ANG_VEL_X 2
#define STATE_ANG_VEL_Y 3
#define STATE_ANG_VEL_Z 4

#define MAX_MOTORS 4
#define MAX_STATES 5

#define BASE_THRUST 29491
#endif
