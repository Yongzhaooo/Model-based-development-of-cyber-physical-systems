#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "helpers.h"

#define MIN_CROSSING_DELAY 500
#define CROSSING_DELAY 1000
#define ARRIVE_DELAY 1500

#define MAX_DIRECTIONS 4
#define CHAR_COUNT(str, ch, count) for(count = 0; *str; *str++ == ch ? count++ : 0);
#define GET_DIR_INDEX(dir, idx) switch(dir) {case 'n': idx = 0; break; case 'w': idx = 1; break; case 's': idx = 2; break; case 'e': idx = 3; break;}
#define IS_DEADLOCK_BYPASS(idx) (isDeadLock() && (getDirWithMaxRequest() == idx))


const char valid_directions[] = {'n', 'w', 's', 'e'};

int *batIds;
int numberOfBats;
xTaskHandle *batTaskHandles;    // Create as a pointer since the number of bats are unknown at compile time

SemaphoreHandle_t crossLock;
SemaphoreHandle_t requestCountFromDir[MAX_DIRECTIONS];

void printRequestBuffer(void)
{
    uint16_t i;
    for(i = 0; i < MAX_DIRECTIONS; i++) {
        vPrintf("--- %c = %u ", valid_directions[i], uxSemaphoreGetCount(requestCountFromDir[i]));
    }
    vPrintf("---\n");
}

uint8_t isDeadLock(void)
{
    uint16_t i;
    uint8_t is_deadlock = 0xff;
    for(i = 0; i < MAX_DIRECTIONS; i++) {
        is_deadlock &= (uint8_t)uxSemaphoreGetCount(requestCountFromDir[i]);
    }
    return is_deadlock;
}

int8_t getDirWithMaxRequest(void)
{
    uint16_t i;
    int8_t max_request_dir = -1;
    uint8_t max_request_count = 0;
    for(i = 0; i < MAX_DIRECTIONS; i++) {
        uint8_t request_count = (uint8_t)uxSemaphoreGetCount(requestCountFromDir[i]);
        if(max_request_count > request_count) {
            max_request_count = request_count;
            max_request_dir = i;
        }
    }
    return max_request_dir;
}

void goSouth(int batId)
{
    const TickType_t waitingToArriveDelay = (rand() % ARRIVE_DELAY) / portTICK_PERIOD_MS; // Generate a random delay 
    const TickType_t crossingDelay = (rand() % CROSSING_DELAY + MIN_CROSSING_DELAY) / portTICK_PERIOD_MS; 
    
    vPrintf("BAT %d from North has started to run and is on its way to the crossing\n", batId);

    vTaskDelay(waitingToArriveDelay);   

    vPrintf("BAT %d from North arrives at crossing, waiting for cross permission\n", batId);

    int8_t from_index = -1;
    GET_DIR_INDEX('n', from_index);
    xSemaphoreGive(requestCountFromDir[from_index]);      // expected: incremeant semaphore
    printRequestBuffer();

    int8_t check_index = -1;
    GET_DIR_INDEX('w', check_index);
    
    UBaseType_t requestCount;
    do {
        requestCount = uxSemaphoreGetCount(requestCountFromDir[check_index]);
    } while((requestCount > 0) || IS_DEADLOCK_BYPASS(from_index));  // wait for BAT from west to finish or deadlock bypass
    

    xSemaphoreTake(crossLock, portMAX_DELAY);
    vPrintf("BAT %d from North enters crossing since it received cross permission\n", batId);
    xSemaphoreTake(requestCountFromDir[from_index], portMAX_DELAY);      // expected: decrement semaphore
    printRequestBuffer();

    vTaskDelay(crossingDelay);   

    vPrintf("BAT %d from North leaving crossing\n", batId);    
    xSemaphoreGive(crossLock);
    
    vTaskDelay(waitingToArriveDelay);
}

void goNorth(int batId)
{
    const TickType_t waitingToArriveDelay = (rand() % ARRIVE_DELAY) / portTICK_PERIOD_MS; // Generate a random delay 
    const TickType_t crossingDelay = (rand() % CROSSING_DELAY + MIN_CROSSING_DELAY) / portTICK_PERIOD_MS;
 
    vPrintf("BAT %d from South has started to run and is on its way to the crossing\n", batId);

    vTaskDelay(waitingToArriveDelay);   
  
    vPrintf("BAT %d from South arrives at crossing, waiting for cross permission\n", batId);

    int8_t from_index = -1;
    GET_DIR_INDEX('s', from_index);
    xSemaphoreGive(requestCountFromDir[from_index]);      // expected: incremeant semaphore
    printRequestBuffer();

    int8_t check_index = -1;
    GET_DIR_INDEX('e', check_index);
    
    UBaseType_t requestCount;
    do {
        requestCount = uxSemaphoreGetCount(requestCountFromDir[check_index]);
    } while((requestCount > 0) || IS_DEADLOCK_BYPASS(from_index));  // wait for BAT from west to finish or deadlock bypass
    
    xSemaphoreTake(crossLock, portMAX_DELAY);
    vPrintf("BAT %d from South enters crossing since it received cross permission\n", batId);
    xSemaphoreTake(requestCountFromDir[from_index], portMAX_DELAY);      // expected: decrement semaphore
    printRequestBuffer();

    vTaskDelay(crossingDelay);   

    vPrintf("BAT %d from South leaving crossing\n", batId);    
    xSemaphoreGive(crossLock);

    vTaskDelay(waitingToArriveDelay);
}

void goEast(int batId)
{
    const TickType_t waitingToArriveDelay = (rand() % ARRIVE_DELAY) / portTICK_PERIOD_MS; // Generate a random delay   
    const TickType_t crossingDelay = (rand() % CROSSING_DELAY + MIN_CROSSING_DELAY) / portTICK_PERIOD_MS; 
    
    vPrintf("BAT %d from West has started to run and is on its way to the crossing\n", batId);

    vTaskDelay(waitingToArriveDelay);   

    vPrintf("BAT %d from West arrives at crossing, waiting for cross permission\n", batId);
    
    int8_t from_index = -1;
    GET_DIR_INDEX('w', from_index);
    xSemaphoreGive(requestCountFromDir[from_index]);      // expected: incremeant semaphore
    printRequestBuffer();
    
    int8_t check_index = -1;
    GET_DIR_INDEX('s', check_index);
    
    UBaseType_t requestCount;
    do {
        requestCount = uxSemaphoreGetCount(requestCountFromDir[check_index]);
    } while((requestCount > 0) || IS_DEADLOCK_BYPASS(from_index));  // wait for BAT from west to finish or deadlock bypass
    
    xSemaphoreTake(crossLock, portMAX_DELAY);
    vPrintf("BAT %d from West enters crossing since it received cross permission\n", batId);
    xSemaphoreTake(requestCountFromDir[from_index], portMAX_DELAY);        // expected: decrement semaphore
    printRequestBuffer();

    vTaskDelay(crossingDelay);   

    vPrintf("BAT %d from West leaving crossing\n", batId);    
    xSemaphoreGive(crossLock);
    
    
    vTaskDelay(waitingToArriveDelay);
}

void goWest(int batId)
{
    const TickType_t waitingToArriveDelay = (rand() % ARRIVE_DELAY) / portTICK_PERIOD_MS; // Generate a random delay 
    const TickType_t crossingDelay = (rand() % CROSSING_DELAY + MIN_CROSSING_DELAY) / portTICK_PERIOD_MS; 
     
    vPrintf("BAT %d from East has started to run and is on its way to the crossing\n", batId);

    vTaskDelay(waitingToArriveDelay);   

    vPrintf("BAT %d from East arrives at crossing, waiting for cross permission\n", batId);

    int8_t from_index = -1;
    GET_DIR_INDEX('e', from_index);
    xSemaphoreGive(requestCountFromDir[from_index]);      // expected: incremeant semaphore
    printRequestBuffer();
    
    int8_t check_index = -1;
    GET_DIR_INDEX('n', check_index);
    
    UBaseType_t requestCount;
    do {
        requestCount = uxSemaphoreGetCount(requestCountFromDir[check_index]);
    } while((requestCount > 0) || IS_DEADLOCK_BYPASS(from_index));  // wait for BAT from west to finish or deadlock bypass
    
    xSemaphoreTake(crossLock, portMAX_DELAY);
    vPrintf("BAT %d from East enters crossing since it received cross permission\n", batId);
    xSemaphoreTake(requestCountFromDir[from_index], portMAX_DELAY);        // expected: decrement semaphore
    printRequestBuffer();
    
    vTaskDelay(crossingDelay);   

    vPrintf("BAT %d from East leaving crossing\n", batId);    
    xSemaphoreGive(crossLock); 

    vTaskDelay(waitingToArriveDelay);
}

void batFromNorth(void *pvParameters)
{
    int batId = *(int *)pvParameters;
    
    while (1) // Repeat forever
    {
	goSouth(batId); 
	goNorth(batId);
    }
}

void batFromSouth(void *pvParameters)
{
    int batId = *(int *)pvParameters;
    
    while (1)
    {
	goNorth(batId); 
	goSouth(batId);
    }
}

void batFromEast(void *pvParameters)
{
    int batId = *(int *)pvParameters;
    
    while (1)
    {
	goWest(batId); 
	goEast(batId);
    }
}

void batFromWest(void *pvParameters)
{
    int batId = *(int *)pvParameters;
    
    while (1)
    {
	goEast(batId); 
	goWest(batId);
    }
}


int main(int argc, char **argv)
{
    int batId;
        
    if (argc < 2)
    {
    	vPrintf("Argument missing, specify sequence of arrivals. Example: ./batman nsewwewn\n");
    	exit(-1);
    }
    
    char *commands = argv[1];
    numberOfBats = strlen(argv[1]);

    batTaskHandles = malloc(sizeof(xTaskHandle) * numberOfBats); // Allocate memory for the task handles
    batIds = malloc(sizeof(int) * numberOfBats); // Allocate memory for the task ids
       
    for (batId = 0; batId < numberOfBats; batId++)
    {
    	batIds[batId] = batId;
    	char currentBatDirection = (argv[1])[batId];
    	char batTaskIdentity[50];
    	
        switch(currentBatDirection)
        {
            case 'n':
            {
                sprintf(batTaskIdentity, "Bat %d from North", batId);
                xTaskCreate(batFromNorth, batTaskIdentity, configMINIMAL_STACK_SIZE, (void *)&batIds[batId], 1, batTaskHandles[batId]);  
                vPrintf("Created task \"Bat %d from North\"\n", batId);
                break;
            }
            case 's':
            {
                sprintf(batTaskIdentity, "Bat %d from South", batId);
                xTaskCreate(batFromSouth, batTaskIdentity, configMINIMAL_STACK_SIZE, (void *)&batIds[batId], 1, batTaskHandles[batId]);  
                vPrintf("Created task \"Bat %d from South\"\n", batId);
                break;
            }
            case 'e':
            {
                sprintf(batTaskIdentity, "Bat %d from East", batId);
                xTaskCreate(batFromEast, batTaskIdentity, configMINIMAL_STACK_SIZE, (void *)&batIds[batId], 1, batTaskHandles[batId]);  
                vPrintf("Created task \"Bat %d from East\"\n", batId);
                break;
            }  
            case 'w':
            {
                sprintf(batTaskIdentity, "Bat %d from West", batId);
                xTaskCreate(batFromWest, batTaskIdentity, configMINIMAL_STACK_SIZE, (void *)&batIds[batId], 1, batTaskHandles[batId]);  
                vPrintf("Created task \"Bat %d from West\"\n", batId);
                break;
            }    	
            default:
            {
                vPrintf("Unknown direction: %c\n", currentBatDirection);
                exit(-1);
                break;
            }
        }

    }

    uint8_t dir;
    uint8_t dir_count[MAX_DIRECTIONS] = {0};
    for(dir = 0; dir < MAX_DIRECTIONS; dir++)
    {   
        char* ptr_commands = commands;
        CHAR_COUNT(ptr_commands, valid_directions[dir], dir_count[dir]);
        vPrintf("char: %c, occurence: %u\n", valid_directions[dir], dir_count[dir]);
    }
    for(dir = 0; dir < MAX_DIRECTIONS; dir++)
    {   
        // eg: max_count for 'n' = BATs starting from 'n' + BATs starting from 's' and vice-versa for 's'
        // eg: max_count for 'e' = BATs starting from 'e' + BATs starting from 'w' and vice-versa for 'w'
        uint16_t max_count = dir_count[dir] + dir_count[(dir + 2) % MAX_DIRECTIONS];
        requestCountFromDir[dir] = xSemaphoreCreateCounting(max_count, 0);
    }


    crossLock = xSemaphoreCreateMutex();

    vTaskStartScheduler();
    while (1)
    {
    	// Do nothing
    };
 
    free(batTaskHandles); // Return memory for the task handles
    return 0;
}

