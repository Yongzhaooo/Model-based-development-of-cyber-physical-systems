#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <memory.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "controller.h"
#include "constants.h"

#define D2R(ang) ((ang) * (M_PI / 180.0))

#define USE_FAST_LQR_GAIN 1

#ifdef USE_FAST_LQR_GAIN
double L[4][5] = {{-0.0701, -0.0571, -0.0164, -0.0133, -0.0584},
                  {-0.0701,  0.0571, -0.0164,  0.0133,  0.0584},
                  { 0.0070,  0.0571,  0.0016,  0.0133, -0.0584},
                  { 0.0070, -0.0571,  0.0016, -0.0133,  0.0584}};
#else 
double L[4][5] = {{-0.0174, -0.0259, -0.0089, -0.0132, -0.0584},
                  {-0.0174,  0.0259, -0.0089,  0.0132,  0.0584},
                  { 0.0174,  0.0259,  0.0089,  0.0132, -0.0584},
                  { 0.0174, -0.0259,  0.0089, -0.0132,  0.0584}};
#endif // USE_FAST_LQR_GAIN

void controlSystemTask(void *pvParameters) {
    struct ControlSystemParams *params =
        (struct ControlSystemParams*)pvParameters;

    TickType_t lastWakeUpTime;
    lastWakeUpTime = xTaskGetTickCount();

    // we keep local copies of the global state + semaphores
    unsigned short motors[4] = {0.0};
    double gyro_data[3] = {0.0};
    double acc_data[3] = {0.0};
    double r_rpdy[3] = {0.0};
    double estimate[3] = {0.0};

    // copy the semaphore handles for convenience
    SemaphoreHandle_t motors_sem = params->motors_sem;
    SemaphoreHandle_t references_sem = params->references_sem;
    SemaphoreHandle_t sensors_sem = params->sensors_sem;
    SemaphoreHandle_t estimate_sem = params->estimate_sem;

    while(1) {
        // read sensor data (gyro)
        xSemaphoreTake(sensors_sem, portMAX_DELAY);
        memcpy(gyro_data, params->gyro_data, sizeof(gyro_data));
        xSemaphoreGive(sensors_sem);

        // read filter data (angle estimates)
        xSemaphoreTake(estimate_sem, portMAX_DELAY);
        memcpy(estimate, params->estimate, sizeof(estimate));
        xSemaphoreGive(estimate_sem);

        // read latest references
        xSemaphoreTake(references_sem, portMAX_DELAY);
        memcpy(r_rpdy, params->r_rpdy, sizeof(r_rpdy));
        xSemaphoreGive(references_sem);

        // compute error
        double error[MAX_STATES] = {0.0};
        error[STATE_X] = D2R(estimate[SEN_X] - r_rpdy[SEN_X]);
        error[STATE_Y] = D2R(estimate[SEN_Y] - r_rpdy[SEN_Y]);
        error[STATE_ANG_VEL_X] = D2R(gyro_data[SEN_X]);
        error[STATE_ANG_VEL_Y] = D2R(gyro_data[SEN_Y]);
        error[STATE_ANG_VEL_Z] = D2R(gyro_data[SEN_Z]);

        // example of how to log some intermediate calculation
        // and use the provided constants
        // params->log_data[0] = crazyflie_constants.m * crazyflie_constants.g;
        // params->log_data[2] = r_rpdy[0];


        // compute motor outputs
        uint8_t r, c;
        for(r = 0; r < MAX_MOTORS; r++)
        {
            motors[r] = 0.0;
            for(c = 0; c < MAX_STATES; c++)
            {
                motors[r] += -L[r][c] * error[c] / crazyflie_constants.pwm_n;
            }
            motors[r] += BASE_THRUST;
        }

        // write motor output
        xSemaphoreTake(motors_sem, portMAX_DELAY);
        memcpy(params->motors, motors, sizeof(motors));
        xSemaphoreGive(motors_sem);

        // sleep 10ms to make this task run at 100Hz
        vTaskDelayUntil(&lastWakeUpTime, 10 / portTICK_PERIOD_MS);
        // vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
