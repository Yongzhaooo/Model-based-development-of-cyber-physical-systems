#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <memory.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "reference_generator.h"

void referenceGenTask(void *pvParameters) {
    struct ReferenceGenParams *params =
        (struct ReferenceGenParams*)pvParameters;

    // copy the semaphore handles for convenience
    SemaphoreHandle_t references_sem = params->references_sem;
    double *r_rpdy = params->r_rpdy;

    int max_ref,i,j;
    int sign = 1;

    // run a few times with increasing "difficulty"
    for(max_ref = 10; max_ref <= 40; max_ref+=10) {
        // roll triangles
        for(i = 0; i < 4; ++i) {
            for(j = 0; j < max_ref; ++j) {
                xSemaphoreTake(references_sem, portMAX_DELAY);
                if (i % 2 == 0) {
                    r_rpdy[0] += 1 * sign;
                } else {
                    r_rpdy[0] -= 1 * sign;
                }
                xSemaphoreGive(references_sem);
                vTaskDelay(100 / portTICK_PERIOD_MS);
            }
        }

        // pitch triangles
        for(i = 0; i < 4; ++i) {
            for(j = 0; j < max_ref; ++j) {
                xSemaphoreTake(references_sem, portMAX_DELAY);
                if (i % 2 == 0) {
                    r_rpdy[1] += 1 * sign;
                } else {
                    r_rpdy[1] -= 1 * sign;
                }
                xSemaphoreGive(references_sem);
                vTaskDelay(100 / portTICK_PERIOD_MS);
            }
        }

        // roll and pitch triangles
        for(i = 0; i < 2; ++i) {
            for(j = 0; j < max_ref; ++j) {
                xSemaphoreTake(references_sem, portMAX_DELAY);
                if (i % 2 == 0) {
                    r_rpdy[0] += 1 * sign;
                    r_rpdy[1] += 1 * sign;
                } else {
                    r_rpdy[0] -= 1 * sign;
                    r_rpdy[1] -= 1 * sign;
                }
                xSemaphoreGive(references_sem);
                vTaskDelay(100 / portTICK_PERIOD_MS);
            }
        }
        sign *= -1;
    }

    // note that this is not the proper way to terminate :)
    // it's just convenient.
    printf("Simulation finished!\n");
    exit(0);
}
