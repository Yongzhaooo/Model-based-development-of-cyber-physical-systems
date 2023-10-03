
#ifndef HELPERS_H
#define HELPERS_H

#include "FreeRTOS.h"

void vPrintString( const portCHAR *pcString );
void vTimerPeriodicISRTests( void );

/*
 * Prototypes for the standard FreeRTOS callback/hook functions implemented
 * within this file.
 */
void vApplicationMallocFailedHook( void );
void vApplicationIdleHook( void );
void vApplicationTickHook( void );

#endif

