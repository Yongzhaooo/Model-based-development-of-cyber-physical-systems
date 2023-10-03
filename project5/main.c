#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <memory.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "reference_generator.h"
#include "controller.h"
#include "filter.h"
#include "constants.h"

// simulation time in ms
static int simulation_time = 0;

// simulation state space in radians
static double rpy[3] = {0.0};
static double drpy[3] = {0.0};

static unsigned short motors[4] = {0,0,0,0};
static SemaphoreHandle_t motors_sem;

static double r_rpdy[3] = {0.0};
static SemaphoreHandle_t references_sem;

static double gyro_data[3] = {0.0};
static double acc_data[3] = {0.0};
static SemaphoreHandle_t sensors_sem;

static double estimate[3] = {0.0};
static SemaphoreHandle_t estimate_sem;

static double log_data[4] = {0.0};

void dynamicModelTask(void *pvParameters) {
    double h = 1.0/1000.0;
    const double d = crazyflie_constants.d / sqrt(2);
    const double t = crazyflie_constants.pwm_n;
    const double bk = crazyflie_constants.b / crazyflie_constants.k;
    const double j_x = crazyflie_constants.j_x;
    const double j_y = crazyflie_constants.j_y;
    const double j_z = crazyflie_constants.j_z;
    double local_gyro[3] = {0.0};
    double local_acc[3] = {0.0};
    double gyro_roll_drift = 0.1;
    double gyro_pitch_drift = 0.1;
    while(1) {
        // step update
        xSemaphoreTake(motors_sem, portMAX_DELAY);
        double tx = t*d*(-motors[0] - motors[1] + motors[2] + motors[3]);
        double ty = t*d*(-motors[0] + motors[1] + motors[2] - motors[3]);
        double tz = t*bk*(-motors[0] + motors[1] - motors[2] + motors[3]);
        xSemaphoreGive(motors_sem);

        double ddx = 1.0/j_x * (-( (drpy[1] * j_z*drpy[2]) - (drpy[2] * j_y*drpy[1])) + tx);
        double ddy = 1.0/j_y * (-( (drpy[2] * j_x*drpy[0]) - (drpy[0] * j_z*drpy[2])) + ty);
        double ddz = 1.0/j_z * (-( (drpy[0] * j_y*drpy[1]) - (drpy[1] * j_x*drpy[0])) + tz);

        drpy[0] += h * ddx;
        drpy[1] += h * ddy;
        drpy[2] += h * ddz;

        rpy[0] += h * drpy[0] + 0.5*h*h * ddx;
        rpy[1] += h * drpy[1] + 0.5*h*h * ddy;
        rpy[2] += h * drpy[2] + 0.5*h*h * ddz;

        // sensor data readings
        local_gyro[0] = drpy[0] * 180.0 / M_PI;
        local_gyro[1] = drpy[1] * 180.0 / M_PI;
        local_gyro[2] = drpy[2] * 180.0 / M_PI;

        // simulate a little drift in roll and pitch
        local_gyro[0] += gyro_roll_drift;
        local_gyro[1] += gyro_pitch_drift;

        double x1 = rpy[0];
        double x2 = rpy[1];
        double x3 = rpy[2];
        // simulate a little accelerometer noise
        double noise_x = 0.1*((double)rand()/RAND_MAX*2.0-1.0);
        double noise_y = 0.1*((double)rand()/RAND_MAX*2.0-1.0);
        double noise_z = 0.1*((double)rand()/RAND_MAX*2.0-1.0);
        double m = 0.027;
        double T = (motors[0] + motors[1] + motors[2] + motors[3]) / 65536.0 / 4.0 * 0.06;

        double r_x = -T/m*cos(x1)*cos(x2)*(sin(x1)*sin(x3) - cos(x1)*cos(x3)*sin(x2));
        double r_y = -T/m*cos(x1)*cos(x2)*(cos(x3)*sin(x1) + cos(x1)*sin(x3)*sin(x2));
        double r_z =  T/m*(sin(x2)*sin(x2) + cos(x2)*cos(x2)*sin(x1)*sin(x1));

        local_acc[0] = 0.0 - r_x + noise_x;
        local_acc[1] = 0.0 - r_y + noise_y;
        local_acc[2] = T/m - r_z + noise_z;

        xSemaphoreTake(sensors_sem, portMAX_DELAY);
        memcpy(acc_data, local_acc, sizeof(local_acc));
        memcpy(gyro_data, local_gyro, sizeof(local_gyro));
        xSemaphoreGive(sensors_sem);

        simulation_time = simulation_time + 1;
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
}

#define GOOD "\033[0;32m\xE2\x9C\x94\033[0m"
#define BAD "\033[0;31m\xE2\x9C\x97\033[0m"

void logTask(void *pvParameters) {
    int first = 1;
    int num_info_lines = 5;
    int num_rtos_lines = 2;

    FILE *f = fopen("simulation.csv","w");
    if(!f) {
        return;
    }
    fprintf(f, "time,actual_roll,actual_pitch,actual_yaw,estimated_roll,estimated_pitch,estimated_yaw,reference_roll,reference_pitch,reference_yawrate,m1,m2,m3,m4,l1,l2,l3,l4\n");

    while(1) {
        if(first) {
            // free rtos prints two lines, erase them first
            printf("\r\033[%dA\033[0K", num_rtos_lines);
            first = 0;
        } else {
            printf("\r\033[%dA\033[0K", num_info_lines);
        }
        double m1 = 100.0 * (double)motors[0] / (double)USHRT_MAX;
        double m2 = 100.0 * (double)motors[1] / (double)USHRT_MAX;
        double m3 = 100.0 * (double)motors[2] / (double)USHRT_MAX;
        double m4 = 100.0 * (double)motors[3] / (double)USHRT_MAX;

        double roll_error = r_rpdy[0]-rpy[0]*180.0/M_PI;
        double pitch_error = r_rpdy[1]-rpy[1]*180.0/M_PI;
        double yawrate_error = r_rpdy[2]-drpy[2]*180.0/M_PI;
        int roll_ok = sqrt(roll_error*roll_error) < 10.0;
        int pitch_ok = sqrt(pitch_error*pitch_error) < 10.0;
        int yawrate_ok = sqrt(yawrate_error*yawrate_error) < 10.0;
        printf("\r\033[0Kroll:  %7.2f ref: %7.2f %s         m1: %6.2f%%    log1: %f\n",
               rpy[0] * 180.0 / M_PI, r_rpdy[0], roll_ok ? GOOD : BAD, m1, log_data[0]);
        printf("\r\033[0Kpitch: %7.2f ref: %7.2f %s         m2: %6.2f%%    log2: %f\n",
               rpy[1] * 180.0 / M_PI, r_rpdy[1], pitch_ok ? GOOD : BAD, m2, log_data[1]);
        printf("\r\033[0Kyaw:   %7.2f                        m3: %6.2f%%    log3: %f\n",
               rpy[2] * 180.0 / M_PI, m3, log_data[2]);
        printf("\r\033[0Kyaw_r: %7.2f ref: %7.2f %s         m4: %6.2f%%    log4: %f\n",
               drpy[2]*180.0/M_PI, r_rpdy[2], yawrate_ok ? GOOD : BAD, m4, log_data[3]);
        printf("\r\033[0Ktime elapsed: \033[1m%8.1f\033[0ms\n",
               simulation_time / 1000.0);

        fprintf(f, "%f,", simulation_time / 1000.0);
        fprintf(f, "%f,%f,%f,", rpy[0] * 180.0 / M_PI,rpy[1] * 180.0 / M_PI,rpy[2] * 180.0 / M_PI);
        fprintf(f, "%f,%f,%f,", estimate[0],estimate[1],estimate[2]);
        fprintf(f, "%f,%f,%f,", r_rpdy[0],r_rpdy[1],r_rpdy[2]);
        fprintf(f, "%f,%f,%f,%f,", m1, m2, m3, m4);
        fprintf(f, "%f,%f,%f,%f\n", log_data[0],log_data[1],log_data[2],log_data[3]);
        fflush(f);
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
    fclose(f);
}

int main(int argc, char **argv) {
    motors_sem     = xSemaphoreCreateBinary(); xSemaphoreGive(motors_sem);
    references_sem = xSemaphoreCreateBinary(); xSemaphoreGive(references_sem);
    sensors_sem    = xSemaphoreCreateBinary(); xSemaphoreGive(sensors_sem);
    estimate_sem   = xSemaphoreCreateBinary(); xSemaphoreGive(estimate_sem);

    xTaskCreate(dynamicModelTask, "Dynamic Model Task",
                configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    xTaskCreate(logTask, "Logging Task",
                configMINIMAL_STACK_SIZE, NULL, 1, NULL);

    struct ReferenceGenParams r_params = {
        .r_rpdy = r_rpdy,
        .references_sem = references_sem,
        .log_data = log_data
    };
    xTaskCreate(referenceGenTask, "Reference Generator Task",
                configMINIMAL_STACK_SIZE, (void*)&r_params, 1, NULL);

    struct FilterParams f_params = {
        .gyro_data = gyro_data,
        .acc_data = acc_data,
        .sensors_sem = sensors_sem,
        .estimate = estimate,
        .estimate_sem = estimate_sem,
        .log_data = log_data
    };
    xTaskCreate(filterTask, "Filter Task",
                configMINIMAL_STACK_SIZE, (void*)&f_params, 1, NULL);

    struct ControlSystemParams c_params = {
        .motors = motors,
        .motors_sem = motors_sem,
        .r_rpdy = r_rpdy,
        .references_sem = references_sem,
        .gyro_data = gyro_data,
        .acc_data = acc_data,
        .sensors_sem = sensors_sem,
        .estimate = estimate,
        .estimate_sem = estimate_sem,
        .log_data = log_data
    };
    xTaskCreate(controlSystemTask, "Control System Task",
                configMINIMAL_STACK_SIZE, (void*)&c_params, 1, NULL);


    vTaskStartScheduler();

    while(1);
}
