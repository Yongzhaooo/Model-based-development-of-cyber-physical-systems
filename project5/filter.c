#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <memory.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "filter.h"
#include "constants.h"

#define R2D(ang) ((ang) * (180.0 / M_PI))
#define SAMPLE_TIME (0.01)
#define GAMMA 0.99

void filterTask(void *pvParameters) {
    struct FilterParams *params =
        (struct FilterParams*)pvParameters;

    TickType_t lastWakeUpTime;
    lastWakeUpTime = xTaskGetTickCount();

    // we keep local copies of the global state + semaphores
    double gyro_data[3];
    double acc_data[3];

    // copy the semaphore handles for convenience
    SemaphoreHandle_t sensors_sem = params->sensors_sem;
    SemaphoreHandle_t estimate_sem = params->estimate_sem;

    // local internal state.
    double estimate[3] = {0.0};

    while(1) {
        // read sensor data
        xSemaphoreTake(sensors_sem, portMAX_DELAY);
        memcpy(gyro_data, params->gyro_data, sizeof(gyro_data));
        memcpy(acc_data, params->acc_data, sizeof(acc_data));
        xSemaphoreGive(sensors_sem);

        // apply filter
        double theta_a[3]  = {0.0};
        double theta_g[3]  = {0.0};

        // accelerometer calculations
        theta_a[SEN_X] = R2D(atan2(acc_data[SEN_Y], acc_data[SEN_Z]));
        theta_a[SEN_Y] = R2D(atan2(-acc_data[SEN_X], sqrt(acc_data[SEN_Y]*acc_data[SEN_Y] + acc_data[SEN_Z]*acc_data[SEN_Z])));
        theta_a[SEN_Z] = 0.0;

        // gyroscope calculations
        theta_g[SEN_X] = gyro_data[SEN_X] * SAMPLE_TIME;
        theta_g[SEN_Y] = gyro_data[SEN_Y] * SAMPLE_TIME;
        theta_g[SEN_Z] = gyro_data[SEN_Z] * SAMPLE_TIME;

        // complementary filter
        estimate[SEN_X] = (1.0 - GAMMA)*theta_a[SEN_X] + (GAMMA)*(estimate[SEN_X] + theta_g[SEN_X]);
        estimate[SEN_Y] = (1.0 - GAMMA)*theta_a[SEN_Y] + (GAMMA)*(estimate[SEN_Y] + theta_g[SEN_Y]);
        estimate[SEN_Z] = (1.0 - GAMMA)*theta_a[SEN_Z] + (GAMMA)*(estimate[SEN_Z] + theta_g[SEN_Z]);
        
        // estimate of the yaw angle provided as an example
        // estimate[2] += 0.01 * gyro_data[2];

        // example of how to log some intermediate calculation
        params->log_data[SEN_X] = estimate[SEN_X];
        params->log_data[SEN_Y] = estimate[SEN_Y];
        
        // write estimates output
        xSemaphoreTake(estimate_sem, portMAX_DELAY);
        memcpy(params->estimate, estimate, sizeof(estimate));
        xSemaphoreGive(estimate_sem);

        // sleep 10ms to make this task run at 100Hz
        vTaskDelayUntil(&lastWakeUpTime, 10 / portTICK_PERIOD_MS);// for better time performance.
        // vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
