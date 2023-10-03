
#ifndef REFERENCE_GEN_H
#define REFERENCE_GEN_H

#include "semphr.h"

void referenceGenTask(void *pvParameters);

struct ReferenceGenParams {
    // references: roll, pitch, yaw rate, in degrees.
    double *r_rpdy;
    SemaphoreHandle_t references_sem;

    // 4 general purpose doubles for logging
    double *log_data;
};

#endif
