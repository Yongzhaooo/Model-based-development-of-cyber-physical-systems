
#ifndef FILTER_H
#define FILTER_H

#include "semphr.h"

void filterTask(void *pvParameters);

struct FilterParams {
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
