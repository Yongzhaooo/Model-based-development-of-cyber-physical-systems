
#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "semphr.h"

void controlSystemTask(void *pvParameters);

struct ControlSystemParams {
    // control system output, PWM 0-2^16-1
    unsigned short *motors;
    SemaphoreHandle_t motors_sem;

    // references: roll, pitch, yaw rate, in degrees.
    double *r_rpdy;
    SemaphoreHandle_t references_sem;

    // sensor data
    // gyro data in degrees/s
    double *gyro_data;

    // acc data in g/s^2
    double *acc_data;

    // sensors semaphore
    SemaphoreHandle_t sensors_sem;

    // output: your estimated angles
    double *estimate;

    // estimation semaphore
    SemaphoreHandle_t estimate_sem;

    // 4 general purpose doubles for logging
    double *log_data;
};

#endif
